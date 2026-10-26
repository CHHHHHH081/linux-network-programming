.PHONY:all
all:server_netcald client_netcald

server_netcald:TCPServer.cc
	g++ -o $@ $^ -std=c++17 -ljsoncpp
client_netcald:TCPClient.cc
	g++ -o $@ $^ -std=c++17 -ljsoncpp

.PHONY:output
output:
	@mkdir output
	@mkdir -p output/bin
	@mkdir -p output/conf
	@mkdir -p output/log
	@cp server_netcald output/bin
	@cp client_netcald output/bin
	@cp test.conf output/conf
	@cp install.sh output/
	@cp uninstall.sh output/
	@tar czf output.tgz output

.PHONY:clean
clean:
	rm -rf server_netcald client_netcald output output.tgz