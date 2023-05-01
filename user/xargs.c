#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

#define MAXARGSIZE 512

enum state {
  S_WAIT,         // 等待参数输入，此状态为初始状态或当前字符为空格
  S_ARG,          // 参数内
  S_ARG_END,      // 参数结束
  S_ARG_LINE_END, // 左侧有参数的换行，例如"arg\n"
  S_LINE_END,     // 左侧为空格的换行，例如"arg  \n""
  S_END           // 结束，EOF
};

enum char_type {
  C_SPACE,
  C_CHAR,
  C_LINE_END
};

enum char_type get_char_type(char c)
{
  switch (c) {
  case ' ':
    return C_SPACE;
  case '\n':
    return C_LINE_END;
  default:
    return C_CHAR;
  }
}

enum state transform_state(enum state cur, enum char_type ct)
{
  switch (cur) {
  case S_WAIT:
    if (ct == C_SPACE)    return S_WAIT;
    if (ct == C_LINE_END) return S_LINE_END;
    if (ct == C_CHAR)     return S_ARG;
    break;
  case S_ARG:
    if (ct == C_SPACE)    return S_ARG_END;
    if (ct == C_LINE_END) return S_ARG_LINE_END;
    if (ct == C_CHAR)     return S_ARG;
    break;
  case S_ARG_END:
  case S_ARG_LINE_END:
  case S_LINE_END:
    if (ct == C_SPACE)    return S_WAIT;
    if (ct == C_LINE_END) return S_LINE_END;
    if (ct == C_CHAR)     return S_ARG;
    break;
  default:
    break;
  }
  return S_END;
}

int main(int argc, char *argv[])
{
  if (argc - 1 >= MAXARG){
    fprintf(2, "xargs: too many arguments\n");
    exit(1);
  }

  char lines[MAXARGSIZE];
  char *p = lines;
  char *args[MAXARG] = {0};
  for (int i=1; i<argc; i++){
    args[i-1] = argv[i];
  }

  int arg_begin = 0;
  int arg_end = 0;
  int arg_cnt = argc - 1;
  enum state cur_state = S_WAIT;

  while (cur_state != S_END){
    if (read(0, p, sizeof(char)) != sizeof(char)){
      cur_state = S_END;
    }
    else{
      cur_state = transform_state(cur_state, get_char_type(*p));
    }

    if (++arg_end >= MAXARGSIZE){
      fprintf(2, "xargs: too many arguments\n");
      exit(1);
    }

    if (cur_state == S_WAIT){
      arg_begin++;
    }
    else if (cur_state == S_ARG_END){
      args[arg_cnt++] = &lines[arg_begin];
      arg_begin = arg_end;
      *p = '\0';
    }
    else if (cur_state == S_ARG_LINE_END){
      args[arg_cnt++] = &lines[arg_begin];
      arg_begin = arg_end;
      *p = '\0';
      if (fork() == 0){
        exec(argv[1], args);
      }
      arg_cnt = argc - 1;
      for (int i=arg_cnt; i<MAXARG; i++){
        args[i] = 0;
      }
      wait(0);
    }
    else if (cur_state == S_LINE_END){
      arg_begin = arg_end;
      *p = '\0';
      if (fork() == 0){
        exec(argv[1], args);
      }
      arg_cnt = argc - 1;
      for (int i=arg_cnt; i<MAXARG; i++){
        args[i] = 0;
      }
      wait(0);
    }
    p++;
  }
  exit(0);
}