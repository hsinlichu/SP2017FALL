#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <float.h>
#include <limits.h>
#include "train.h"
#include "test.h"
#include "pool.h"

char directory[40];
char output[40];
int tree_number;
int thread_number;
double data[26000][35];
Node* forest;

void readArgument(int argc,char** argv);
int readTrainingData();
  
int main( int argc, char** argv){
  clock_t t1, t2;
  t1 = clock();
  readArgument(argc,argv);
  printf("%s %s %d %d\n",directory,output,tree_number,thread_number);
  int numData = readTrainingData();
  forest = (Node*)malloc(tree_number * sizeof(Node));
  training(forest,numData);
  testing(forest);
  t2 = clock();
  printf("time: %lf(s)\n", (t2-t1)/(double)(CLOCKS_PER_SEC));
  return 0;
}

int readTrainingData(){
  FILE *stream;
  stream = fopen("./training_data", "r");
  if (stream == NULL) 
    ERR_EXIT("Fopen error")
      int ret;
  char* line = NULL;
  size_t len = 0;
  int numData = 0;  
  while ((ret = getline(&line, &len, stream)) != -1) { // read training_data
    int count = 0;
    line = strtok(line," ");
    while(line != NULL){
      data[numData][count++] = atof(line);
      line = strtok(NULL," ");
    }
    ++numData;
  }
  printf("read training data success\n");
  fclose(stream);
  return numData;
}

void readArgument(int argc,char** argv){
  int c;
  char* const short_options = "a:b:c:d:";  
  struct option long_options[] = {
    { "data"  ,required_argument,NULL,'a'},  
    { "output",required_argument,NULL,'b'},  
    { "tree"  ,required_argument,NULL,'c'},
    { "thread",required_argument,NULL,'d'},
    {        0,                0,   0, 0 }};

  if( argc != 9 ){
    fprintf(stderr,"Argument Format Incorrect\n"); // ./hw4 -data data_dir -output submission.csv -tree tree_number -thread thread_number
    exit(1);
  }
  while((c = getopt_long_only(argc, argv, short_options, long_options, NULL)) != -1){  
    switch (c){  
    case 'a':  
      strcpy(directory,optarg);
      break;
    case 'b':  
      strcpy(output,optarg);
      break;
    case 'c':  
      tree_number = atoi(optarg);  
      break;
    case 'd':  
      thread_number = atoi(optarg);  
      break;
    default:
      printf("Incorrect Option %c\n",c);
      exit(-1);
    }  
  }
  return;
}
