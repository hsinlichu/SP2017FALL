/*b05505004 朱信靂 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void creatfifo(char *rfifoaddress,char *wfifoaddress[4])
{
  //fprintf(stderr,"host create fifo%s\n",rfifoaddress);
  if(mkfifo(rfifoaddress,0600) < 0)
    fprintf(stderr,"host mkfifo WRONG\n");

  for(int i = 0;i < 4;++i)
      if(mkfifo(wfifoaddress[i],0600) < 0)
	fprintf(stderr,"host mkfifo WRONG\n");
  return;
}

void unlinkfifo(char *rfifoaddress,char *wfifoaddress[4])
{
  if(unlink(rfifoaddress) < 0)
    fprintf(stderr,"unlink WRONG\n");

  for(int i = 0;i < 4;++i)
    if(unlink(wfifoaddress[i]) < 0)
      fprintf(stderr,"unlink WRONG\n");
  return;
}

int main(int argc,char *argv[])
{
  if(argc != 2)
    {
      fprintf(stderr,"%d host Argument WRONG\n",argc);
      exit(-1);
    }
  
  const char playerindex[4] = {'A','B','C','D'};
  char rfifoaddress[100];
  char *wfifoaddress[4];
  sprintf(rfifoaddress,"./host%s.FIFO",argv[1]);
  char buffer[4][100];
  for(int i = 0;i < 4;++i)
    {
      sprintf(buffer[i],"./host%s_%c.FIFO",argv[1],playerindex[i]);
      wfifoaddress[i] = buffer[i];
    }
  creatfifo(rfifoaddress,wfifoaddress);

  while(1)
    {
      char input[100];
      int id[4];
      if(read(STDIN_FILENO,input,sizeof(input)) == -1)
	fprintf(stderr,"read WRONG\n");
      //fprintf(stderr,"host read %s\n",input);
      sscanf(input,"%d %d %d %d",&id[0],&id[1],&id[2],&id[3]);
      //fprintf(stderr,"%d %d %d %d\n",id[0],id[1],id[2],id[3]);
      if(id[0] == -1 && id[1] == -1 && id[2] == -1 && id[3] == -1 )
	break;
	      
      int key[4];
      srand(time(NULL));
      for(int i = 0;i < 4;++i)
	  key[i] = (int)(((double)rand() / RAND_MAX) * 65536);
      
      pid_t pid;
      int childnumber = -1;
      for (int i = 0; i < 4; i++)
	{
	  pid = fork();
	  if(pid == 0)childnumber = i;
	  //if(pid > 0)fprintf(stderr,"host child player pid %d\n",pid);
	  if (pid == 0 || pid == -1) break;
	}
      
      if (pid == -1)
	fprintf(stderr,"Fork error\n");
      else if (pid == 0) //child
	{
	  char skey[100];
	  char shost[100];
	  char splayer[100];
	  sprintf(splayer,"%c",playerindex[childnumber]);
	  sprintf(skey,"%d",key[childnumber]);
	  sprintf(shost,"%s",argv[1]);
	  if(execl("./player","./player",shost,splayer,skey,(char *)0) < 0)
	    fprintf(stderr,"execl error\n");
	}
      else //parent
	{
	  int win[4] = {0};
	  int fd[4];
	  int readfd;
	  if((readfd = open(rfifoaddress,O_RDWR)) < 0)
	    fprintf(stderr,"rfifo open wrong\n");
	  for(int i = 0;i < 4;++i)
	    if((fd[i] = open(wfifoaddress[i],O_RDWR)) < 0)
	      fprintf(stderr,"wfifo open wrong\n");
	  
	  int money[4] = {1000,1000,1000,1000};
	  for(int i = 0;i < 10;++i)
	    {	  
	      //fprintf(stderr,"%dth money %d %d %d %d\n",i,money[0],money[1],money[2],money[3]);
	      char output[100];
	      char readbuffer[100];
	      int maxnum = -100000;
	      int index = -1;
	      int bidmoney[4];
	      sprintf(output,"%d %d %d %d\n",money[0],money[1],money[2],money[3]);
	      for(int j = 0;j < 4;++j)// read write
		{
		  if(write(fd[j],output,strlen(output)) < 0)
		    fprintf(stderr,"%d write wrong",j);
		  fsync(fd[j]);
		  int n;
		  if(n = read(readfd,readbuffer,100) < 0)
		    fprintf(stderr,"%d read wrong",j);
		  //fprintf(stderr,"j = %d %d response %s",j,n,readbuffer);---why---
		  char index;
		  int remkey,pay;
		  sscanf(readbuffer,"%c %d %d",&index,&remkey,&pay);
		  if((index - 'A') != j || remkey != key[j])
		    fprintf(stderr,"player write wrong\n");
		  bidmoney[j] = pay;
		}
	      for(int j = 0;j < 4;++j)// find max
		{
		  if(bidmoney[j] > maxnum)
		    maxnum = bidmoney[j],index = j;
		  else if(bidmoney[j] == maxnum)
		    {
		      bidmoney[j] = 0;
		      bidmoney[index] = 0;
		      maxnum = -100000;
		      index = -1;
		      j = 1;
		    }
		}
	      if(index == -1)
		fprintf(stderr,"Can't find max\n");
	      ++win[index];
	      money[index] -= maxnum;
	      
	      for(int j = 0;j < 4;++j)
		money[j] += 1000;
	    }
	  
	  int rank[4];
	  char result[100] = {};
	  for(int i = 0;i < 4;++i)
	    {
	      int bigger = 0;
	      for(int j = 0;j < 4;++j)
		{
		  if(i == j)continue;
		  if(win[j] > win[i])++bigger;
		}
	      char tmp[100];
	      sprintf(tmp,"%d %d\n",id[i],bigger + 1);    
	      strcat(result,tmp);
	    }
	  //fprintf(stderr,"--%s",result);
	  if(write(STDOUT_FILENO,result,strlen(result)) < 0)
	    fprintf(stderr,"write error");
	  //fprintf(stderr,"host return bidding success\n");
	  fsync(STDOUT_FILENO);
	  for(int i = 0;i < 4;++i)
	    wait(NULL);
	  close(readfd);
	  for(int i = 0;i < 4;++i)
	    close(fd[i]);
	}
    }
  unlinkfifo(rfifoaddress,wfifoaddress);
  exit(0);
}
