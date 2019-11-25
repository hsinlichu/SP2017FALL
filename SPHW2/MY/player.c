/*b05505004 朱信靂 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  if(argc != 4)
    {
      fprintf(stderr,"Argument WRONG %d\n",argc - 1);
      exit(-1);
    }
  
  char *host_id = argv[1];
  char *player_index = argv[2];
  char *random_key = argv[3];
  int index = player_index[0] - 'A';
  //fprintf(stderr,"index %d\n",index);
  
  char rfifo[20];
  sprintf(rfifo,"./host%s_%s.FIFO",host_id,player_index);  
  //fprintf(stderr,"rfifo name %s\n",rfifo);

  char wfifo[20];
  sprintf(wfifo,"./host%s.FIFO",host_id);
  //fprintf(stderr,"wfifo name %s\n",wfifo);
  
  int fdr,fdw;
  fdr = open(rfifo,O_RDONLY);
  if(fdr == -1)
    fprintf(stderr,"Open fifo wrong\n");
  fdw = open(wfifo,O_WRONLY);
  if(fdw == -1)
    fprintf(stderr,"Open fifo wrong\n");
  
  //printf("fdw %d\n",fdw);
  //printf("fdr %d\n",fdr);
  for(int i = 0;i < 10;++i)
    {
      int money[4];
      char input[100];
      int zero = 0;
      int check = read(fdr,input,sizeof(input));
      sscanf(input,"%d %d %d %d",&money[0],&money[1],&money[2],&money[3]);
      /*
      for(int i = 0;i < 4;++i)
	fprintf(stdout,"%d ",money[i]);
	fprintf(stderr,"\n");
      */      
      int pay = ((i % 4) == index) ? money[index] : 0;
      char output[100];
      sprintf(output,"%c %s %d\n",player_index[0],random_key,pay);
      if(write(fdw,output,strlen(output)) < 0)
	fprintf(stderr,"player write error\n");
      //fprintf(stderr,"player %s len %lu||%s\n",player_index,strlen(output),output);
      fsync(fdw);
    }
  exit(0);
}
