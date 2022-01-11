#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void mid_process(int parent_pipe_fd)
{
    int buf[1];
    read(parent_pipe_fd, buf, sizeof(buf));
    int p = buf[0];               // this is a prime
    printf("prime %d\n", buf[0]); //这里使用的1文件描述符1仍然是标准输出
    int n = read(parent_pipe_fd, buf, sizeof(buf));
    if (n && buf[0] % p != 0) // there is new possible prime from parent process
    {
        int pipe_mid_fd[2];
        pipe(pipe_mid_fd);
        int pid = fork();
        if (pid == 0)
        {
            close(parent_pipe_fd);
            close(pipe_mid_fd[1]);
            mid_process(pipe_mid_fd[0]);
        }
        else if (pid > 0)
        {
            close(1);
            close(pipe_mid_fd[0]);
            while (n)
            {
                if (buf[0] % p != 0)
                {
                    write(pipe_mid_fd[1], buf, sizeof(buf));
                }
                n = read(parent_pipe_fd, buf, sizeof(buf));
            }
            close(pipe_mid_fd[1]);
            close(parent_pipe_fd);
        }
    }
}

int main(int argc, char *argv[])
{
    close(0);
    close(2);
    int pipe_fd[2];
    pipe(pipe_fd);
    int pid = fork();
    if (pid == 0) // first fork is special, because it should produce numbers
    {
        close(pipe_fd[1]);
        mid_process(pipe_fd[0]);
    }
    else if (pid > 0)
    {
        close(1);
        close(pipe_fd[0]);
        int buf[1];
        for (int i = 2; i <= 35; i++)
        {
            buf[0] = i;
            write(pipe_fd[1], buf, sizeof(buf));
        }
        close(pipe_fd[1]);
    }
    exit(0);
}