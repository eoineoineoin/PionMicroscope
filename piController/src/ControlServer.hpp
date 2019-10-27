#pragma once
#include <vector>

struct ControlState
{
	float m_adChannel1;
};

class ControlServer
{
public:
	ControlServer();
	~ControlServer();

	void step(const ControlState& curState);

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

