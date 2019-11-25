#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
int main()
{
  char output[] = "1000 1000 1000 1000";
  int fdw;
  char name[] = "./host1_C.FIFO";
  mkfifo(name,0666);
  mkfifo("./host1.FIFO",0666);
  fdw = open(name,O_RDWR);
  if(fdw == -1)
    fprintf(stderr,"Open fifo wrong\n");
  
  int fdr;
  fdr = open("./host1.FIFO",O_RDWR);
  if(fdr == -1)
    fprintf(stderr,"Open fifo wrong\n");
      
  write(fdw,output,strlen(output));
  fsync(fdw);
  fprintf(stdout,"%s\n",output);

  char buffer[100];
  int check = read(fdr,buffer,sizeof(buffer));
  
  fprintf(stdout,"check %d||%s\n",check,buffer);

  return 0;
}
