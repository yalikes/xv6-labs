#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// a naive string match algorithm to find first match
int rb_match(char *s, int s_len, char *t, int t_len)
{
    for (int i = 0; i <= s_len - t_len; i++)
    {
        int flag = 1;
        for (int j = 0; j < t_len; j++)
        {
            if (s[i + j] != t[j])
            {
                flag = 0;
                break;
            }
        }
        if (flag)
        {
            return i;
        }
    }
    return -1;
}

void find(char *path, char *name)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;
    char *file_name;
    char dir_name_buf[DIRSIZ + 1];

    if ((fd = open(path, 0)) < 0)
    {
        fprintf(2, "ls: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type)
    {
    case T_FILE:
        file_name = fmtname(path);
        if (rb_match(file_name, strlen(file_name), name, strlen(name)) >= 0)
        {
            printf("%s\n", path);
        }
        break;
    case T_DEVICE:
        file_name = fmtname(path);
        if (rb_match(file_name, strlen(file_name), name, strlen(name)) >= 0)
        {
            printf("%s\n", path);
        }
        break;
    case T_DIR:
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
        {
            printf("find: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        while (read(fd, &de, sizeof(de)) == sizeof(de))
        {
            if (de.inum == 0)
            {
                continue;
            }
            memmove(dir_name_buf, de.name, DIRSIZ);
            dir_name_buf[DIRSIZ] = 0;
            if (strcmp(dir_name_buf, ".") == 0 || strcmp(dir_name_buf, "..") == 0)
            {
                continue;
            }
            memmove(p, dir_name_buf, DIRSIZ + 1);
            if (stat(buf, &st) < 0)
            {
                printf("find: cannot stat %s\n", buf);
                continue;
            }
            find(buf, name);
        }
        break;
    }
    close(fd);//this is important, because xv6 only has limited file descriptor aviliable
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        find(".", "");
    }
    else if (argc == 2)
    {
        find(argv[1], "");
    }
    else
    {
        for (int i = 1; i < argc - 1; i++)
        {
            find(argv[i], argv[argc - 1]);
        }
    }
    exit(0);
}
