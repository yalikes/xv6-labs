#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int pid = fork();
    if (pid == 0)
    {
        
    }
    else if (pid > 0)
    {

        for (int i = 2; i <= 35; i++)
        {
        }
    }
    else
    {
        fprintf(2, "fork error\n");
    }
}