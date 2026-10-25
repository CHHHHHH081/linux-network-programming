#include "TCPServer.hpp"
#include "Protocal.hpp"
#include "NetCal.hpp"

using namespace SocketModule;
void Usage(std::string proc)
{
    std::cerr << "Usage: " << proc << " [port]" << std::endl;
    exit(USAGE_ERR);
}
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        Usage(argv[0]);
    }
    Enable_Console_Stretage();
    uint16_t port = std::stoi(argv[1]);
    std::unique_ptr<Cal> cal = std::make_unique<Cal>();

    std::shared_ptr<Protocal> pro = std::make_shared<Protocal>([&cal](Request &req) -> Response
                                                               { return cal->Execute(req); });

    std::shared_ptr<TCPServer> tsvr = std::make_shared<TCPServer>(port,
                                                                  [&pro](std::shared_ptr<Socket> &sock, InetAddr &addr)
                                                                  {
                                                                      pro->GetRequest(sock, addr);
                                                                  });
    tsvr->Run();
    return 0;
}
