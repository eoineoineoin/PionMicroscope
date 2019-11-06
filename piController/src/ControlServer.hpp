#pragma once
#include <vector>
#include <deque>
#include <Protocol.h>

class ControlServer
{
public:
	ControlServer();
	~ControlServer();

	Packets::ControlCommand step(const Packets::CurrentState& curState);

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
	std::deque<Packets::ControlCommand> m_incomingCommands;
};

