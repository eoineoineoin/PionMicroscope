#pragma once
#include <vector>
#include <deque>
#include <Protocol.h>

class CommandHandler;

class ControlServer
{
public:
	ControlServer();
	~ControlServer();

	struct OnConnectInfo
	{
		uint16_t m_resolutionX;
		uint16_t m_resolutionY;
	};

	void step(CommandHandler* commandHandler,
			const OnConnectInfo& newClientConnectionInfo,
			const Packets::BeamState& beamState);

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

