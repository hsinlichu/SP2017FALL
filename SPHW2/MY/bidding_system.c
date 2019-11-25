/*b05505004 朱信靂 */
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int C(int N,int K)
{
  double answer = 1;
  for(int i = N,j = K;j > 0;--i,--j)
    {
      answer *= i;
      answer /=j;
    }
  return answer;
}

void schedule(int *table,int number,int start,int end,int index,int choose)
{
  static int count = 0;
  if(index == choose)
    {
      table[count] = number;
      /*
      printf("count %d\n",count);
      for(int k = end;k >= 0;--k)
	printf("%d ",(table[count] >> k) & 1);
      printf("\n");
      */
      ++count;
      return;
    }
  for(int i = start;i <=end && (end - i + 1 >= choose - index);++i)
      schedule(table,number | (1 << i),i + 1,end,index + 1,choose);
  return;
}

void assign(char *buffer,int input,int player)
{
  int tmp[4];
  int count = 0;
  for(int i = 0;i < player;++i)
    if(input & (1 << i))
      tmp[count++] = i;
  sprintf(buffer,"%d %d %d %d\n",tmp[0],tmp[1],tmp[2],tmp[3]);
  //printf("assign %s",buffer);
  return;
}


int main(int argc,char *argv[])
{
  if(argc != 3)
    {
      printf("Argument wrong\n");
      exit(-1);
    }
  int player;
  int host;
  sscanf(argv[1],"%d",&host);
  sscanf(argv[2],"%d",&player);
  //printf("host %d player %d\n",host,player);
  unsigned long long int comnumber = C(player,4);
  //printf("com %llu\n",comnumber);
  int *table = (int*)malloc(comnumber * sizeof(int));
  
  memset(table,0,comnumber);
  schedule(table,0,0,player - 1,0,4);
  /*
  for(int j = 0;j < comnumber;++j)
    {
      for(int k = 0;k < player;++k)
	printf("%d ",(table[j] >> k) & 1);
      printf("\n");
    }
  */
  pid_t pid;
  int hostnumber;
  int hostfd[12][2];// 0 for bidding listen, 1 for host write
  int biddingfd[12][2];// 0 for host listen, 1 for bidding write
  for(int i = 0;i < host;++i)
    {
      if(pipe(hostfd[i]))
	printf("pipe1 error\n");
      if(pipe(biddingfd[i]))
	printf("pipe2 error\n");
      pid = fork();
      //if(pid > 0)printf("child pid %d\n",pid);
      if(pid == 0)hostnumber = i;
      if(pid == 0 || pid == -1)break;
    }
  if(pid == -1)
    printf("fork wrong\n");
  else if(pid == 0) // host
    {
      if(dup2(biddingfd[hostnumber][0],STDIN_FILENO) != STDIN_FILENO)
	printf("dup STDIN_FILENO error\n");
      if(dup2(hostfd[hostnumber][1],STDOUT_FILENO) != STDOUT_FILENO)
	printf("dup STDOUT_FILENO error\n");
      char tmp[100];
      sprintf(tmp,"%d",hostnumber);
      if(execl("./host","./host",tmp,(char *)0) < 0)
	printf("execl error\n");
    }
  else // bidding system
    {
      for(int i = 0;i < host;++i)
	{
	  char input[100];
	  assign(input,table[i],player);
	  if(write(biddingfd[i][1],input,strlen(input)) < 0)
	    printf("first write error\n");
	  fsync(biddingfd[i][1]);
	}

      int rank[20] = {0};
      fd_set master_set,check_set;
      int hasassign = host;
      FD_ZERO(&master_set);
      for(int i = 0;i < host;++i)
	  FD_SET(hostfd[i][0],&master_set);
      for(int i = 0;i < comnumber;)// read 
	{
	  memcpy(&check_set,&master_set,sizeof(master_set));
	  select(hostfd[host - 1][0] + 1,&check_set,NULL,NULL,NULL);
	  for(int j = 0;j < host;++j)
	    if(FD_ISSET(hostfd[j][0],&check_set))// read ready
	      {
		//printf("fd %d can read\n",hostfd[j][0]);
		char buffer[100];
		if(read(hostfd[j][0],buffer,100) < 0)
		  printf("read error\n");
		//printf("read from host %d\n%s",j,buffer);
		int playerid[4],playerrank[4];
		sscanf(buffer,"%d %d\n%d %d\n%d %d\n%d %d\n",
		       &playerid[0],&playerrank[0],&playerid[1],&playerrank[1],
		       &playerid[2],&playerrank[2],&playerid[3],&playerrank[3]);
		
		for(int k = 0;k < 4;++k)
		  {
		    rank[playerid[k]] += 4 - playerrank[k];
		    //printf("id %d %d\n",playerid[k],rank[playerid[k]]);
		  }
		++i;
		if(hasassign < comnumber)
		  {
		    char input[100];
		    assign(input,table[hasassign],player);
		    if(write(biddingfd[j][1],input,strlen(input)) < 0)
		      printf("host %d ||write error\n",j);
		    fsync(biddingfd[j][1]);
		    ++hasassign;
		  }
	      }
	}
      
      for(int i = 0;i < host;++i)
	{
	  char input[] ="-1 -1 -1 -1\n";
	  if(write(biddingfd[i][1],input,strlen(input)) < 0)
	    printf("close host error\n");
	  wait(NULL);
	}
      for(int i = 0;i < player;++i)
	{
	  int bigger = 0;
	  for(int j = 0;j < player;++j)
	    {
	      if(i == j)continue;
	      if(rank[j] > rank[i])++bigger;
	    }
	  printf("%d %d\n",i + 1,bigger + 1);
	}
    }
  free(table);
  return 0;
}
