#pragma once
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Common.hpp"
#include "log.hpp"

using namespace LogModule;

const std::string nullfd = "/dev/null";

void Daemon(int nochdir, int noclose)
{
    // 忽略IO, 子进程相关信号
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

    // 父进程退出
    if (fork() > 0)
        exit(0);

    setsid();

    if (nochdir == 0)
        chdir("/");
    if (noclose == 0)
    {
        int fd = open(nullfd.c_str(), O_RDWR);
        if (fd == -1)
        {
            LOG(Loglevel::FATAL) << "open file err";
            exit(OPEN_ERR);
        }
        else
        {
            dup2(fd, 0);
            dup2(fd, 1);
            dup2(fd, 2);
            close(fd);
        }
    }
}
