#pragma once
#include "Socket.hpp"

using namespace SocketModule;

using service_t = std::function<void(std::shared_ptr<Socket> &, InetAddr &)>;

class TCPServer
{
public:
    TCPServer(uint16_t port, service_t service)
        : _port(port), _service(service), _isrunning(false), _listensock(std::make_unique<TCPSocket>())
    {
        _listensock->BuildTCPSocketMethod(port);
    }

    void Run()
    {
        _isrunning = true;
        while (_isrunning)
        {
            InetAddr addr;
            auto sock = _listensock->AcceptOrDie(&addr);
            if (sock == nullptr)
            {
                continue;
            }
            LOG(Loglevel::INFO) << "accept success, addr: " << addr.AddrString();
            pid_t n = fork();
            if (n < 0)
            {
                LOG(Loglevel::ERROR) << "fork err";
                exit(FORK_ERR);
            }
            else if (n == 0)
            {
                // 子进程
                _listensock->Close();
                if (fork() > 0)
                    exit(OK);
                // 孙子进程
                _service(sock, addr);
                exit(OK);
            }
            else
            {
                // 父进程
                sock->Close();
                pid_t rid = waitpid(n, nullptr, 0);
                (void)rid;
            }
        }
    }

private:
    uint16_t _port;
    std::unique_ptr<Socket> _listensock;
    service_t _service;
    bool _isrunning;
};
