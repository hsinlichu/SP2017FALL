#pragma once

#include <pthread.h>
#define ERR_EXIT(a) { perror(a); exit(1); }

typedef struct node Node;
struct node{
  int number;
  Node* left;
  Node* right;
  int* dataID;
  int numdata;
  int isLeaf;			    	// boolean flag for leaf nodes
  int dimension;	        	// Stores which dimension cut the data
  double label;     		        // Stores the class label for leaf nodes.
                                        // For nodes that are not leaf nodes, it stores the value of the cutpoint. 
};

typedef struct pool_queue Queue;
struct pool_queue {
  Node *root;
  Queue *next;
};

typedef struct pool {
  char cancelled;
  void (*fn)(Node*,int);
  unsigned int remaining;
  unsigned int nthreads;
  Queue *queue;
  Queue *end;
  pthread_mutex_t queue_mtx;
  pthread_cond_t queue_cnd;
  pthread_t* threads;
}Pool;

extern char directory[40];
extern char output[40];
extern int tree_number;
extern int thread_number;
extern double data[26000][35];
extern Node* forest;

Pool* pool_start(void (*thread_func)(Node*,int), unsigned int threads);
void pool_enqueue(void *pool, void *arg);
void pool_wait(void *pool);
void pool_end(void *pool);
