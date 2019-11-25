#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <float.h>
#include <limits.h>
#include "pool.h"
#include "train.h"
#define numSampling 300


char directory[40];
char output[40];
int tree_number;
int thread_number;
double data[26000][35];
Node* forest;

void training(Node* forest,int numtotalData){
  printf("Parallel forest training with %d trees and %d threads\n", tree_number, thread_number);
  printf("Sampling number: %d\n",numSampling);
  void* pool = pool_start(&train_gini,thread_number); //Create thread pool
  srand( (unsigned)time(NULL));
  for(int i = 0; i < tree_number;++i) {
    forest[i].dataID = randomSampling(numtotalData);
    forest[i].numdata = numSampling;
    forest[i].number = i;
    pool_enqueue(pool,&forest[i]);
  }
  pool_wait(pool); //JOIN on all
  pool_end(pool);  //Free resources
  return;
}

void train_gini(Node* root,int depth){
  //printf("Start train tree %d depth %d\n",root->number,depth);
  root->left = NULL;
  root->right = NULL;
  if(ishomogeneous(root->dataID,root->numdata)){ // leaf node
    root->isLeaf = 1;
    root->label = data[root->dataID[0]][34];
    //printf("Is homogeneous\n");
    return;
  }
  else{
    //printf("Not homogeneous\n");
    root->isLeaf = 0;
    double min_gini= DBL_MAX;
    double cut_value;
    int cut_index;
    int cut_dimension;
    int tmp_numlowerthreshold;
    int numlowerthreshold;
    for(int i = 1; i < 34; ++i) { // dimension
      qsort_r(root->dataID,root->numdata,sizeof(int),compare,(void*)&i);
      double ret;
      for(int j = 0;j < root->numdata - 1;++j){ // there are (root->numdata - 1) cutpoint
	if(data[root->dataID[j]][i] == data[root->dataID[j + 1]][i])
	  continue;
	double cutvalue = (data[root->dataID[j]][i] + data[root->dataID[j + 1]][i]) / 2;
	//printf("cutvalue %lf\n",cutvalue);
	ret = calculate_gini(root->dataID,root->numdata,i,cutvalue,&tmp_numlowerthreshold);
	//printf("ID %d dimension %d -> gini %lf\n",root->dataID[j],i,ret);
	if(ret <= min_gini){
	  if(ret == min_gini)
	    if((j - (root->numdata / 2.0)) > (cut_index - (root->numdata / 2.0))) // cut as near as the midpoint
	      continue;
	  min_gini = ret;
	  cut_dimension = i;
	  cut_index = j;
	  cut_value = cutvalue;
	  numlowerthreshold = tmp_numlowerthreshold;
	}
      }
    }
    root->dimension = cut_dimension;
    root->label = cut_value;
    
    root->left = (Node*)malloc(sizeof(Node));
    root->left->numdata = numlowerthreshold;
    root->left->dataID = (int*)malloc(root->left->numdata * sizeof(int));
    root->left->number = root->number;
    root->right = (Node*)malloc(sizeof(Node));
    root->right->numdata = (root->numdata - root->left->numdata);
    root->right->dataID = (int*)malloc(root->right->numdata * sizeof(int));
    root->right->number = root->number;
    int leftcount = 0;
    int rightcount = 0;
    for(int i = 0;i < root->numdata;++i){
      if(data[root->dataID[i]][root->dimension] <= root->label)
	root->left->dataID[leftcount++] = root->dataID[i];
      else
	root->right->dataID[rightcount++] = root->dataID[i];
    }
    //printf("%d cutdimension %d ID %d threshold %lf\n",root->number,root->dimension,root->dataID[cut_index],root->label);
    //printf("%d leftcount %d rightcount %d\n",root->number,leftcount,rightcount);
    //printf("%d %d\n",leftcount,numlowerthreshold[root->dimension]);
#ifdef DEBUG
    printf("\n-----------------------check-%d-----------------------\n",root->numdata);
    compareDimension = root->dimension;       // tell qsort compare which dimension
    qsort(root->dataID,root->numdata,sizeof(int),compare);
    for(int j = 0;j < root->numdata;++j)
      printf("%5d %lf %lf\n",root->dataID[j],data[root->dataID[j]][root->dimension],data[root->dataID[j]][34]);
    printf("\n--------------------------Check--------------------------\n");
#endif

    if(leftcount != numlowerthreshold)
      ERR_EXIT("count subdataID error");
    train_gini(root->left,depth + 1);
    train_gini(root->right,depth + 1);
  }
  return;
}

double calculate_gini(const int* dataID,const int num,const int dimension,const double cutvalue,int* numlowerthreshold){
  int numyes = 0;
  int count = 0;
  //printf("dimension %d threshold %lf\n",dimension,cutvalue);
  while((data[dataID[count]][dimension] <= cutvalue)){
    if(data[dataID[count]][34])
      ++numyes;
    ++count;
    if(count == num)
      break;
  }
  *numlowerthreshold = count;
  double yespercent = (double)numyes / count;
  double uppergini = 2 * yespercent * (1 - yespercent);
  //printf("numyes/count %d/%d percent %lf uppergini %lf\n",numyes,count,yespercent,uppergini);
  numyes = 0;
  while(count < num){
    if(data[dataID[count]][34])
      ++numyes;
    ++count;
  }

  double lowergini; 
  if(*numlowerthreshold == num)
    lowergini = 0;
  else{
    yespercent = (double)numyes / (num - *numlowerthreshold);
    lowergini = 2 * yespercent * (1 - yespercent);
  //printf("numyes/count %d/%d percent %lf uppergini %lf\n",numyes,(numData - *numlowerthreshold),yespercent,lowergini);
  return uppergini + lowergini;
  }
}

int ishomogeneous(int* dataID,int num){
  double check = data[dataID[0]][34];
  for(int i = 1;i < num;++i){
    if(data[dataID[i]][34] != check)
      return 0;
  }
  return 1;
}

int* randomSampling(int numData){
  int* tmp = (int*)malloc(numSampling * sizeof(int));
  for(int i = 0;i < numSampling;++i)
    tmp[i] = (rand() % numData);
  return tmp;
}

int compare(const void *data1,const void *data2,void* arg){
  int* ID1 = (int*)data1;
  int* ID2 = (int*)data2;
  int* compareDimension = (int*)arg;
  if(data[*ID1][*compareDimension] < data[*ID2][*compareDimension])
    return -1;
  else if(data[*ID1][*compareDimension] > data[*ID2][*compareDimension])
    return 1;
  else{
    if(data[*ID1][34] < data[*ID2][34])
      return -1;
    if(data[*ID1][34] > data[*ID2][34])
      return 1;
    else
      return 0;
  }
}
