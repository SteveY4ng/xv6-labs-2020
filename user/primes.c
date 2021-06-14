#include "kernel/types.h"
#include "user/user.h"

#define R 0
#define W 1

void redirect(int k, int *pd)
{
    close(k);
    dup(pd[k]);
    close(pd[R]);
    close(pd[W]);
}

void filter(int p)
{
    int n;
    while (read(R, &n, sizeof(n)))
    {
        if (n % p != 0)
            write(W, &n, sizeof(n));
    }

    close(W);
    close(R);
    wait(0);
    exit(0);
}

void waitForNumber()
{
    int pd[2];
    int n;
    if (read(R, &n, sizeof(n)))
    {
        printf("prime %d\n", n);
        pipe(pd);
        if (fork() == 0)
        { // child
            redirect(R, pd);
            waitForNumber();
        }
        else
        { // parent
            redirect(W, pd);
            filter(n);
        }
    }

    exit(0);
}

int main(int argc, char *argv)
{

    int pd[2];

    if (argc != 1)
    {
        fprintf(2, "Usage: primes...\n");
        exit(1);
    }

    pipe(pd);
    if (fork() == 0)
    { // child
        redirect(R, pd);
        waitForNumber();
    }
    else
    { // parent
        redirect(W, pd);
        for (int i = 2; i < 36; i++)
        {
            write(W, &i, sizeof(i));
        }
        close(W);
        wait(0);
        exit(0);
    }
    return 0;
}