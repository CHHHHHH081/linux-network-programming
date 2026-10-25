.PHONY:all
all:server_netcal client_netcal

server_netcal:TCPServer.cc
	g++ -o $@ $^ -std=c++17 -ljsoncpp
client_netcal:TCPClient.cc
	g++ -o $@ $^ -std=c++17 -ljsoncpp

.PHONY:clean
clean:
	rm -f server_netcal client_netcal