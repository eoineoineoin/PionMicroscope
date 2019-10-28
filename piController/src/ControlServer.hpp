#pragma once
#include <vector>
#include <Protocol.h>

class ControlServer
{
public:
	ControlServer();
	~ControlServer();

	void step(const Packets::CurrentState& curState);

	struct Socket
	{
		Socket(int sockfd);
		Socket(Socket&& other);
		Socket& operator=(Socket&& other);
		~Socket();

		int m_socket;
	};

	Socket m_serverSocket;
	std::vector<Socket> m_clients;
};

