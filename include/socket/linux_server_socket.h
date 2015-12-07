


#pragma  once
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include<string>

class linux_server_socket{
public:
	linux_server_socket(const char * address = "localhost", int port = 8787) :_address(address), _port(port){
	}

	bool connect(){
		struct sockaddr_in destination;

		destination.sin_family = AF_INET;
		socket_id = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (socket_id < 0)
		{
			printf("\nSocket Creation FAILED!");
			return false;
		}

		destination.sin_port = htons(_port);
		destination.sin_addr.s_addr = inet_addr(_address.c_str());
		if (::connect(socket_id, (struct sockaddr *)&destination, sizeof(destination)) != 0){
			printf("\nSocket Connection FAILED!\n");
			if (socket_id)
				clean_up();
			return false;
		}
		//printf("\nConnected!");
		return true;
	}

	bool listen() {
		struct sockaddr_in server, client;

		printf("Initialised.\n");

		//Create a socket
		if ((socket_id = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
			printf("Could not create socket");
			return false;
		}

		printf("Socket created.\n");

		//Prepare the sockaddr_in structure
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = INADDR_ANY;
		server.sin_port = htons(_port);

		//Bind
		if (bind(socket_id, (struct sockaddr *)&server, sizeof(server)) == -1)
		{
			printf("Bind failed with error code %d\n",errno);
			return false;
		}
		puts("Bind done");

		//Listen to incoming connections
		::listen(socket_id, 3);

		//Accept and incoming connection
		puts("Waiting for incoming connections...");

		socklen_t c = sizeof(struct sockaddr_in);
		socket_client_id = accept(socket_id, (struct sockaddr *)&client, &c);
		if (socket_client_id == -1)
		{
			printf("accept failed with error code");
			return false;
		}

		puts("Connection accepted");
		return true;
	}

	void close(){
		::close(socket_client_id);
		::close(socket_id);
	}

	std::string receive() {
		int recv_size;
		char server_reply[1024];
		if ((recv_size = read(socket_client_id, server_reply, 1024)) < -1)
		{
			printf("recv failed");
			return std::string();
		}
		return std::string(server_reply, server_reply + recv_size);
	}

	int send(const std::string & buffer){
		return ::send(socket_client_id, buffer.c_str(), buffer.size(), 0);
	}

	~linux_server_socket(){
		clean_up();
	}

private:
	void clean_up(){
		::close(socket_client_id);
		::close(socket_id);
	}

	int socket_id;
	int socket_client_id;
	std::string _address;
	int _port;
};