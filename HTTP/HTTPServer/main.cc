#include "Http.hpp"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage err!" << std::endl;
        return USAGE_ERR;
    }
    Enable_Console_Stretage();
    uint16_t port = std::stoi(argv[1]);
    std::shared_ptr<Http> Httpptr = std::make_shared<Http>(port);
    Httpptr->Start();
    return 0;
}