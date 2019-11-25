#include "pool.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

static void * thread(void *arg);

Pool* pool_start(void (*thread_func)(Node*,int), unsigned int threads) {
  Pool *workingPool = (Pool*)malloc(sizeof(Pool));
  pthread_mutex_init(&workingPool->queue_mtx, NULL);
  pthread_cond_init(&workingPool->queue_cnd, NULL);
  workingPool->nthreads = threads;
  workingPool->fn = thread_func;
  workingPool->cancelled = 0;
  workingPool->remaining = 0;
  workingPool->end = NULL;
  workingPool->queue = NULL;
  workingPool->threads = (pthread_t*)malloc(threads * sizeof(pthread_t));
  
  for(int i = 0;i < threads;++i)
    pthread_create(&workingPool->threads[i],NULL,&thread,(void*)workingPool);
  printf("thread pool open success\n");
  return workingPool;
}

void pool_enqueue(void *pool, void *arg) {
  Pool *workingPool = (Pool *) pool;
  Queue *q = (Queue *) malloc(sizeof(Queue));
  q->root = (Node*)arg;
  q->next = NULL;

  pthread_mutex_lock(&workingPool->queue_mtx);
  if (workingPool->end != NULL) workingPool->end->next = q;
  if (workingPool->queue == NULL) workingPool->queue = q; // first in queue
  workingPool->end = q;
  workingPool->remaining++;
  pthread_cond_signal(&workingPool->queue_cnd);
  pthread_mutex_unlock(&workingPool->queue_mtx);
  //printf("pool enqueue success\n");
  return;
}

void pool_wait(void *pool) {
  Pool *workingPool = (Pool *) pool;

  pthread_mutex_lock(&workingPool->queue_mtx);
  while (!workingPool->cancelled && workingPool->remaining) 
    pthread_cond_wait(&workingPool->queue_cnd, &workingPool->queue_mtx);
  pthread_mutex_unlock(&workingPool->queue_mtx);
}

void pool_end(void *pool) {
  Pool *workingPool = (Pool *) pool;
  Queue *q;
  workingPool->cancelled = 1;

  pthread_mutex_lock(&workingPool->queue_mtx);
  pthread_cond_broadcast(&workingPool->queue_cnd);
  pthread_mutex_unlock(&workingPool->queue_mtx);

  for (int i = 0; i < workingPool->nthreads; i++)
    pthread_join(workingPool->threads[i], NULL);

  while (workingPool->queue != NULL) {
    q = (workingPool->queue);
    workingPool->queue = q->next;
    free(q);
  }
  free(workingPool);
}

static void* thread(void *arg) { // wait for assignment
  Queue *q;
  Pool *workingPool = (Pool *) arg;

  while (!workingPool->cancelled) {
    pthread_mutex_lock(&workingPool->queue_mtx);
    while (!workingPool->cancelled && workingPool->queue == NULL) 
      pthread_cond_wait(&workingPool->queue_cnd, &workingPool->queue_mtx);
    
    if(workingPool->cancelled) {
      pthread_mutex_unlock(&workingPool->queue_mtx);
      return NULL;
    }
    q = workingPool->queue; // wakeup and getjob
    workingPool->queue = q->next;
    workingPool->end = (q == workingPool->end ? NULL : workingPool->end);
    pthread_mutex_unlock(&workingPool->queue_mtx);
    //printf("got work\n");
    workingPool->fn(q->root,0); // execute
    //printf("work done\n");
    free(q);
    q = NULL;
    pthread_mutex_lock(&workingPool->queue_mtx);
    workingPool->remaining--;
    pthread_cond_broadcast(&workingPool->queue_cnd);
    pthread_mutex_unlock(&workingPool->queue_mtx);
  }
  return NULL;
}
