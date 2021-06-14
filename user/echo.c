#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int i;
  // argv[1] 是程序名
  for(i = 1; i < argc; i++){
    write(1, argv[i], strlen(argv[i])); // 将argv[i]写入文件描述符1所指向的文件中，strlen返回字符串的长度，不包括结尾空字符
    if(i + 1 < argc){ //当前不是最后一个argv
      write(1, " ", 1);
    } else {
      write(1, "\n", 1);
    }
  }
  exit(0); // exit系统调用，正常退出时参数为0，异常退出时参数为1
}
