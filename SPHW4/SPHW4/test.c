#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "pool.h"
#include "test.h"

char directory[40];
char output[40];
int tree_number;
int thread_number;
double data[26000][35];
Node* forest;


void correct_rate(){  
  FILE *submissionstream;
  submissionstream = fopen(output,"r");
  if (submissionstream == NULL) 
    ERR_EXIT("Fopen submissionstream error")
  FILE *answerstream;
  answerstream = fopen("ans.csv","r");
  if (answerstream == NULL) 
    ERR_EXIT("Fopen answerstream error")

  char* line1 = NULL;
  char* line2 = NULL;
  size_t len1 = 0,len2 = 0;
  int total = -1;
  int unequel = 0;
  int ret1 = 0,ret2 = 0;
  while(ret1 != -1 || ret2 != -1){
    ret1 = getline(&line1,&len1,answerstream);
    ret2 = getline(&line2,&len2,submissionstream);
    if((strcmp(line1,line2) != 0) || (ret1 != ret2))
      ++unequel;
    ++total;
  }
  //printf("unequel/total %d / %d\n",unequel,total);
  double rate = 1 - (double)unequel / total;
  printf("correct_rate: %lf%%\n",rate * 100);
  fclose(submissionstream);
  fclose(answerstream);
  return;
}

int answering(const Node* forest,const double* input){
  int count = 0;
  for(int i = 0;i < tree_number;++i)
    count += traverse(&forest[i],input);
  return ((count >= (tree_number / 2.0)) ? 1 : 0);
}

int traverse(const Node* root,const double* input){
  if(root->isLeaf)
    return root->label;
  if(input[root->dimension] <= root->label)
    return traverse(root->left,input);
  else
    return traverse(root->right,input);
}

void testing(const Node* forest){
  int submissionfd;
  if((submissionfd = open(output,O_WRONLY | O_CREAT,(mode_t)0777)) < 0)
    ERR_EXIT("Submission file open error")
      char buffer[50];
  sprintf(buffer,"id,label\n");
  write(submissionfd,buffer,strlen(buffer));
  
  FILE *stream;
  stream = fopen("./testing_data", "r");
  if (stream == NULL) 
    ERR_EXIT("Fopen error")

  int ret;
  char* line = NULL;
  size_t len = 0;
  int numData = 0;
  while ((ret = getline(&line, &len, stream)) != -1) { // read training_data
    int count = 0;
    double input[34];
    line = strtok(line," ");
    while(line != NULL){
      input[count++] = atof(line);
      line = strtok(NULL," ");
    }
    int answer = answering(forest,input);
    sprintf(buffer,"%d,%d\n",numData,answer);
    write(submissionfd,buffer,strlen(buffer));
    ++numData;
  }
  printf("test success\n");
  correct_rate();
  fclose(stream);
  close(submissionfd);
  return;
}
