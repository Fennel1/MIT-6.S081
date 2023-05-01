#include "kernel/types.h"
#include "user/user.h"

void
primes(int left_p[2]){
  close(left_p[1]);
  int left_num;
  if (read(left_p[0], &left_num, sizeof(int)) == sizeof(int)){
    fprintf(1, "prime %d\n", left_num);

    int right_p[2];
    pipe(right_p);
    int num;
    while(read(left_p[0], &num, sizeof(int)) == sizeof(int)){
      if (num % left_num != 0){
        write(right_p[1], &num, sizeof(int));
      }
    }
    close(left_p[0]);
    close(right_p[1]);

    if (fork() == 0){
      primes(right_p);
    }
    else{
      close(right_p[0]);
      wait(0);
    }
  }
  exit(0);
}

int
main(int argc, char *argv[])
{
  int p[2];
  pipe(p);
  for (int i=2; i<=35; i++){
    write(p[1], &i, sizeof(int));
  }

  if (fork() == 0){
    primes(p);
  }
  else{
    close(p[0]);
    close(p[1]);
    wait(0);
  }
  exit(0);
}