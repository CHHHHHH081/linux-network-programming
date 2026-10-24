#include "Socket.hpp"

using namespace SocketModule;

void Usage(std::string proc)
{
    std::cerr << "Usage: " << proc << " [ip] [port]" << std::endl;
    exit(USAGE_ERR);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        Usage(argv[0]);
    }
    std::string serverip = argv[1];
    uint16_t serverport = std::stoi(argv[2]);
    std::shared_ptr<Socket> client = std::make_shared<TCPSocket>();
    client->BuildTCPClientMethod();
    if (client->Connect(serverip, serverport) < 0)
    {
        LOG(Loglevel::ERROR) << "connect err";
        exit(CONNECT_ERR);
    }
    else
    {
        while (true)
        {
            std::string buf;
            std::cout << "Please Enter# ";
            if (!std::getline(std::cin, buf))
                break; // EOF, 退出
            // 按协议封装: len\r\n{json}\r\n
            std::string frame = std::to_string(buf.size()) + "\r\n" + buf + "\r\n";
            int sendn = client->Send(frame);
            (void)sendn;
            std::string ret;
            int n = client->Recv(&ret);
            if (n <= 0)
                break; // 服务端关闭
            std::cout << ret << std::endl;
        }
    }
    return 0;
}
