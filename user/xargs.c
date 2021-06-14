#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[]) {

    int i,j,k;
    char c;
    char *exec_argv[MAXARG];
    char buf[512];

    // 定位buf位置
    j = 0;
    // 定位当前参数在buf的开始位置
    k = 0;
    for(i=0; i<argc-1; i++) {
        exec_argv[i] = argv[i+1];
    }
    
    while(read(0, &c, sizeof(c))) {
        // printf("read char: %c\n", c);
        // printf("j = %d, k = %d \n", j, k);
        if(c == ' ') {
            if( j == k) {
                continue;
            }else {
                buf[j++] = 0;
                exec_argv[i++] = &buf[k];
                k = j;
            }
        } else if(c == '\n') {
            if(j == k) {
                continue;
            }else {
                buf[j] = 0;
                exec_argv[i++] = &buf[k];
                exec_argv[i] = 0;
                if(fork() == 0) {
                    // printf("I'm in child.\n");
                    // printf("%s\n", exec_argv[1]);
                    // printf("argv: (%s) (%s) ()")
                    exec(exec_argv[0],exec_argv);
                }else {
                    wait(0);
                    i = argc-1;
                    j = 0;
                    k = 0;
                }
            }
        } else {
            buf[j++] = c;
        }
    }

    exit(0);
}
