#include "kernel/types.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  struct sysinfo info;

  if (sysinfo(&info) < 0) {
    fprintf(2, "sysinfo: failed\n");
    exit(1);
  }

  printf("free memory: %lu bytes\n", info.freemem);
  printf("processes:   %lu\n", info.nproc);

  exit(0);
}
