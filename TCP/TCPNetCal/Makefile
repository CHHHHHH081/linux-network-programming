.PHONY:all
all:tcpserver tcpclient

tcpserver:TCPServer.cc
	g++ -o $@ $^ -std=c++17 -ljsoncpp
tcpclient:TCPClient.cc
	g++ -o $@ $^ -std=c++17 -ljsoncpp

.PHONY:clean
clean:
	rm -f tcpserver