#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  char buf = '1';
  int pf_c[2], pc_f[2];
  pipe(pf_c);
  pipe(pc_f);

  int pid = fork();
  int exit_code = 0;

  if (pid < 0){
    fprintf(2, "fork failed\n");
    close(pf_c[0]);
    close(pf_c[1]);
    close(pc_f[0]);
    close(pc_f[1]);
    exit(1);
  }
  else if (pid == 0) {
    // child
    close(pf_c[1]);
    close(pc_f[0]);

    if (read(pf_c[0], &buf, 1) != 1) {
      fprintf(2, "child read failed\n");
      exit_code = 1;
    }
    else{
      fprintf(1, "%d: received ping\n", getpid());
    }

    if (write(pc_f[1], "1", sizeof(char)) != sizeof(char)) {
      fprintf(2, "child write failed\n");
      exit_code = 1;
    }

    close(pf_c[0]);
    close(pc_f[1]);
    exit(exit_code);
  } else {
    // parent
    close(pf_c[0]);
    close(pc_f[1]);
    if (write(pf_c[1], "1", sizeof(char)) != sizeof(char)) {
      fprintf(2, "parent write failed\n");
      exit_code = 1;
    }

    if (read(pc_f[0], &buf, sizeof(char)) != sizeof(char)) {
      fprintf(2, "parent read failed\n");
      exit_code = 1;
    }
    else{
      fprintf(1, "%d: received pong\n", getpid());
    }

    close(pf_c[1]);
    close(pc_f[0]);
    exit(exit_code);
  }
}