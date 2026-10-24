#pragma once
#include "Common.hpp"
#include "InetAddr.hpp"
#include "log.hpp"

const int defaultbacklog = 5;

using namespace LogModule;
namespace SocketModule
{
    class Socket
    {
    public:
        virtual ~Socket() = default;
        virtual void SocketOrDie() = 0;
        virtual void BindOrDie(uint16_t port) = 0;
        virtual void ListenOrDie(int backlog) = 0;
        virtual std::shared_ptr<Socket> AcceptOrDie(InetAddr *addr) = 0;
        virtual int Close() = 0;
        virtual int Send(std::string &msg) = 0;
        virtual int Recv(std::string *out) = 0;
        virtual int Connect(std::string serverip, uint16_t serverport) = 0;

    public:
        void BuildTCPSocketMethod(uint16_t port, int backlog = defaultbacklog)
        {
            SocketOrDie();
            BindOrDie(port);
            ListenOrDie(backlog);
        }
        void BuildTCPClientMethod()
        {
            SocketOrDie();
        }

    private:
    };

    const static int defaultfd = -1;
    class TCPSocket : public Socket
    {
    public:
        TCPSocket()
            : _sockfd(defaultfd)
        {
        }
        TCPSocket(int fd)
            : _sockfd(fd)
        {
        }

        void SocketOrDie() override
        {
            _sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (_sockfd < 0)
            {
                LOG(Loglevel::FATAL) << "socket err";
                exit(SOCKET_ERR);
            }
            LOG(Loglevel::INFO) << "socket success";
        }

        void BindOrDie(uint16_t port) override
        {
            InetAddr addr(port);
            int n = bind(_sockfd, addr.NetAddrPtr(), addr.NetAddrlen());
            if (n < 0)
            {
                LOG(Loglevel::ERROR) << "bind err";
                exit(BIND_ERR);
            }
            LOG(Loglevel::INFO) << "bind success";
        }

        void ListenOrDie(int backlog = defaultbacklog) override
        {
            int n = listen(_sockfd, backlog);
            if (n < 0)
            {
                LOG(Loglevel::ERROR) << "listen err";
                exit(LISTEN_ERR);
            }
            LOG(Loglevel::INFO) << "listen success";
        }

        std::shared_ptr<Socket> AcceptOrDie(InetAddr *addr) override
        {
            struct sockaddr_in peer;
            socklen_t len = sizeof(peer);
            int n = accept(_sockfd, CONV(peer), &len);
            if (n < 0)
            {
                LOG(Loglevel::WARNING) << "accept err";
                return nullptr;
            }
            addr->SetAddr(peer);
            return std::make_shared<TCPSocket>(n);
        }

        int Send(std::string &msg) override
        {
            return send(_sockfd, msg.c_str(), msg.size(), 0);
        }

        int Recv(std::string *out) override
        {
            char buf[1024];
            ssize_t n = recv(_sockfd, buf, sizeof(buf) - 1, 0);
            if (n > 0)
            {
                buf[n] = 0;
                *out += buf;
            }
            return n;
        }

        int Close() override
        {
            int n = close(_sockfd);
            if (n < 0)
            {
                LOG(Loglevel::ERROR) << "close err";
                return 1;
            }
            return n;
        }

        int Connect(std::string serverip, uint16_t serverport) override
        {
            InetAddr addr(serverip, serverport);
            return connect(_sockfd, addr.NetAddrPtr(), addr.NetAddrlen());
        }

        ~TCPSocket()
        {
        }

    private:
        int _sockfd;
    };
}
