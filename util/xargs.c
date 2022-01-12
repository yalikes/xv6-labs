#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/stat.h"
#include "user/user.h"
void run_cmd_helper(char *argv_exec[], char *argv_buf, char cmd, int initial_argv_num, int n_arg)
{
    for (int i = 0; i < n_arg; i++)
    {
        argv_exec[i + initial_argv_num] = argv_buf[i];
    }
    argv_exec[initial_argv_num + n_arg] = 0;
    int pid = fork();
    if (pid == 0)
    {
        exec(cmd, argv_exec);
    }
    else if (pid > 0)
    {
        wait(0);
    }
}

int main(int argc, char *argv[])
{
    char cmd[128];
    int initial_argv_num = argc - 1;

    char argv_buf[128][MAXARG];
    char *argv_exec[MAXARG];
    char buf[1];
    int n_arg = 0, n_char = 0;

    strcpy(cmd, argv[1]);

    for (int i = 1; i < argc; i++)
    {
        argv_exec[i - 1] = argv[i];
    }

    while (read(0, buf, sizeof(buf)))
    {
        if (buf[0] == '\n')
        {
            if (n_char)
            {
                n_arg += 1;
            }
            run_cmd_helper(argv_exec, argv_buf, cmd, initial_argv_num, n_arg);
            n_arg = 0;
            n_char = 0;
        }
        else
        {
            if (buf[0] != ' ' && buf[0] != '\t')
            {
                argv_buf[n_arg][n_char] = buf[0];
                n_char += 1;
            }
            else if (n_char != 0) // start reading next argument
            {
                argv_buf[n_arg][n_char] = 0;
                n_arg += 1;
                n_char = 0;
            }
        }
    }
    if (n_arg || n_char)
    {
        if (n_char)
        {
            n_arg += 1;
        }
        run_cmd_helper(argv_exec, argv_buf, cmd, initial_argv_num, n_arg);
    }
    exit(0);
}
