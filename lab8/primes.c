#include "kernel/types.h"
#include "user/user.h"

void sieve(int in) __attribute__((noreturn));

// one stage of the sieve: reads numbers from fd "in"
void
sieve(int in)
{
  int p, n;

  if(read(in, &p, sizeof(p)) != sizeof(p)){
    close(in);
    exit(0);
  }

  printf("prime %d\n", p);

  int fd[2];
  pipe(fd);

  if(fork() == 0){
    close(fd[1]);
    close(in);
    sieve(fd[0]);
  }

  close(fd[0]);

  while(read(in, &n, sizeof(n)) == sizeof(n)){
    if(n % p != 0)
      write(fd[1], &n, sizeof(n));
  }

  close(in);
  close(fd[1]);
  wait(0);
  exit(0);
}

int
main(int argc, char *argv[])
{
  int fd[2];
  pipe(fd);

  if(fork() == 0){
    close(fd[1]);
    sieve(fd[0]);
  }

  close(fd[0]);

  for(int i = 2; i <= 35; i++)
    write(fd[1], &i, sizeof(i));

  close(fd[1]);
  wait(0);
  exit(0);
}
