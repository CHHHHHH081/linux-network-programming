#include "Socket.hpp"
#include "Protocal.hpp"

using namespace SocketModule;

void Usage(std::string proc)
{
    std::cerr << "Usage: " << proc << " [ip] [port]" << std::endl;
    exit(USAGE_ERR);
}

std::string GetReqFromStdinAndEncode(std::shared_ptr<Protocal> Encodetool)
{
    int x, y;
    char oper;
    std::cout << "Please Enter x: ";
    std::cin >> x;
    std::cout << "Please Enter y: ";
    std::cin >> y;
    std::cout << "Please Enter operator: ";
    std::cin >> oper;
    return Encodetool->BuildReqEncodeString(x, y, oper);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        Usage(argv[0]);
    }
    std::string serverip = argv[1];
    uint16_t serverport = std::stoi(argv[2]);
    InetAddr server(serverip, serverport);
    std::shared_ptr<Socket> client = std::make_shared<TCPSocket>();
    client->BuildTCPClientMethod();
    if (client->Connect(serverip, serverport) < 0)
    {
        LOG(Loglevel::ERROR) << "connect err";
        exit(CONNECT_ERR);
    }
    else
    {
        std::shared_ptr<Protocal> pPro = std::make_shared<Protocal>();
        std::string recv_buf; // recv_buf内存放收到的请求
        while (true)
        {
            std::string EncodeStdin = GetReqFromStdinAndEncode(pPro);
            int sendn = client->Send(EncodeStdin);
            (void)sendn;
            // 请求已发送, 下一步应该接收返回
            Response resp;
            bool recvflag = pPro->GetResponse(client, recv_buf, resp);
            if (!recvflag)
            {
                LOG(Loglevel::ERROR) << "client recv error";
                break;
            }
            resp.ShowResult();
        }
    }
    client->Close();
    return 0;
}
