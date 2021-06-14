#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {

    int ping[2];
    int pong[2];
    int pid;
    char buf;
    int n;

    if(argc != 1) {
        fprintf(2, "Usage: pingpong...\n");
        exit(1);
    }
    // 管道对父子进程都可见，所以应该在fork之前声明
    pipe(ping);
    pipe(pong);

    pid = fork();

    if(pid < 0) {
        fprintf(2, "Failed to fork child process.\n");
        exit(1);
    }else if(pid == 0) { // 子进程
        close(ping[1]);
        close(pong[0]);
        n = read(ping[0], &buf, sizeof(buf));
        if(n == 1) {
            fprintf(1, "%d: received ping\n", getpid());
            n = write(pong[1], &buf, sizeof(buf));
            if(n != 1){
                fprintf(2, "Failed to write to pipe in child process.\n");
                exit(1);
            }
            close(ping[0]);
            close(pong[1]);
            // close(ping[0]);
            // close(ping[1]);
            // close(pong[0]);
            // close(pong[1]);
            exit(0);
        }else {
            fprintf(2, "Failed to read from pipe in child process.\n");
            exit(1);
        }
    }else{
        close(ping[0]);
        close(pong[1]);
        n = write(ping[1], "a", 1);
        if(n != 1) {
            fprintf(2, "Failed to write to pipe in parent process.\n");
            wait(0);
            exit(1);
        } 
        n = read(pong[0], &buf, 1);
        if(n != 1) {
            fprintf(2, "Failed to read from pipe in parent process.\n");
            wait(0);
            exit(1);
        }

        fprintf(1, "%d: received pong\n", getpid());
        close(ping[1]);
        close(pong[0]);
        exit(0);
    }


}