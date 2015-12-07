


#pragma  once
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include<string>

class linux_socket{
public:
	linux_socket(const char * address, int port) :_address(address), _port(port){
	}

	bool connect(){
		struct sockaddr_in destination;

		destination.sin_family = AF_INET;
		socket_id = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (socket_id < 0)
		{
			//printf("\nSocket Creation FAILED!");
			return false;
		}

		destination.sin_port = htons(_port);
		destination.sin_addr.s_addr = inet_addr(_address.c_str());
		if (::connect(socket_id, (struct sockaddr *)&destination, sizeof(destination)) != 0){
			//printf("\nSocket Connection FAILED!\n");
			if (socket_id)
				clean_up();
			return false;
		}
		//printf("\nConnected!");
		return true;
	}

	void send(const std::string & buffer){
		::send(socket_id, buffer.c_str(), buffer.size(), 0);
	}

	std::string receive() {
		int recv_size = 0;
		char server_reply[1024];
		if ((recv_size = read(socket_id, server_reply, 1024)) < 1)
		{
			return std::string();
			//printf("recv failed");
		}
		return std::string(server_reply, server_reply + recv_size);
	}

	~linux_socket(){
		clean_up();
	}

private:
	void clean_up(){
		close(socket_id);
	}

	int socket_id;
	std::string _address;
	int _port;
};