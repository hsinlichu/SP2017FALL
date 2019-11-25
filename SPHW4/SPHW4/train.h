#include "pool.h"

void training(Node* forest,int numData);
void train_gini(Node* root,int depth);
double calculate_gini(const int* dataID,const int numData,const int dimension,const double cutvalue,int* numlowerthreshold);
int ishomogeneous(int* dataID,int numData);
int compare(const void *data1,const void *data2,void* arg);
int* randomSampling(int numData);
