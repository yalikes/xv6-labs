#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int pipe_fd[2];
    pipe(pipe_fd);
    int pid = fork();
    
    if (pid == 0)
    {
        int this_pid = getpid();
        char buf[1];
        read(pipe_fd[1], buf, 1);
        printf("%d: received ping\n", this_pid);
        write(pipe_fd[1], buf, 1);
    }
    else if (pid > 0)
    {
        int this_pid = getpid();
        char buf[1];
        buf[0]=42;
        write(pipe_fd[0], buf, 1);
        read(pipe_fd[0], buf, 1);
        printf("%d: received pong\n", this_pid);
    }
    else
    {
        fprintf(2, "fork error\n");
    }
    exit(0);
}