#include <ControlServer.hpp>
#include <CommandHandler.hpp>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <errno.h>
#include <algorithm>

template<typename T>
static void appendToBuffer(std::vector<uint8_t>& buf, const T& data)
{
	const uint8_t* data8 = reinterpret_cast<const uint8_t*>(&data);
	buf.insert(buf.end(), data8, data8 + sizeof(T));
}

static int sendToClient(const ControlServer::Socket& client, std::vector<uint8_t>& buffer)
{
	return send(client.m_socket, &buffer[0], buffer.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
}
namespace
{
struct LargestCommand : public Packets::BasePacket
{
	uint8_t m_padding[7];
};
static_assert(sizeof(LargestCommand) == 8, "Expect largest command to be 8 bytes");
}


ControlServer::ControlServer()
	: m_serverSocket(-1)
{
	int serverFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);

	m_serverSocket = Socket(serverFd);

	sockaddr_in listenAddr;
	listenAddr.sin_family = AF_INET;
	const uint16_t portNum = 3017;
	listenAddr.sin_port = htons(portNum);
	listenAddr.sin_addr.s_addr = inet_addr("0.0.0.0");

	int err = bind(serverFd, (sockaddr*)&listenAddr, sizeof(listenAddr));
	if(err != 0)
	{
		printf("Could not bind to socket (%i)\n", errno);
	}
	else
	{
		err = listen(serverFd, 10);
		if(err != 0)
		{
			printf("Could not listen to socket (%i)\n", errno);
		}
		else
		{
			printf("Server listening on port %i\n", portNum);
		}
	}
}

ControlServer::~ControlServer() = default;

void ControlServer::step(CommandHandler* commandHandler,
		const OnConnectInfo& newClientConnectionInfo,
		const Packets::BeamState& beamState)
{
	sockaddr_in clientAddr;
	socklen_t clientAddrSize = sizeof(clientAddr);
	int newClient = accept(m_serverSocket.m_socket, (sockaddr*)&clientAddr, &clientAddrSize);
	if(newClient > 0)
	{
		printf("Got new connection from %s\n", inet_ntoa(clientAddr.sin_addr));
		m_clients.emplace_back<Socket>(newClient);
		
		std::vector<uint8_t> outputBuffer;

		Packets::ResolutionChanged resolutionInfo;
		resolutionInfo.m_resolutionX = newClientConnectionInfo.m_resolutionX;
		resolutionInfo.m_resolutionY = newClientConnectionInfo.m_resolutionY;
		appendToBuffer(outputBuffer, resolutionInfo);

		sendToClient(m_clients.back(), outputBuffer);
	}

	std::vector<LargestCommand> incomingCommands;
	for(auto it = m_clients.begin(); it != m_clients.end(); it++)
	{
		fd_set readFds;
		timeval waitTime;
		waitTime.tv_sec = 0;
		waitTime.tv_usec = 10;

		FD_ZERO(&readFds);
		FD_SET(it->m_socket, &readFds);

		if(select(it->m_socket + 1, &readFds, nullptr, nullptr, &waitTime))
		{
			LargestCommand cmdIn;
			int nRead = recv(it->m_socket, &cmdIn, sizeof(cmdIn), MSG_DONTWAIT);
			if(nRead == sizeof(cmdIn))
			{
				incomingCommands.push_back(cmdIn);
			}
		}
	}

	std::vector<uint8_t> outputBuffer;
	appendToBuffer(outputBuffer, beamState);

	// Process all incoming commands, pass to handler; append results to output buffer
	for(const Packets::BasePacket& c : incomingCommands)
	{
		if(c.m_type == Packets::Type::SET_RESOLUTION)
		{
			const Packets::SetResolution* setRes = static_cast<const Packets::SetResolution*>(&c);
			commandHandler->setResolution(setRes->m_resolutionX, setRes->m_resolutionY);
			
			Packets::ResolutionChanged resolutionInfo;
			resolutionInfo.m_resolutionX = setRes->m_resolutionX;
			resolutionInfo.m_resolutionY = setRes->m_resolutionY;
			appendToBuffer(outputBuffer, resolutionInfo);
		}
		else if(c.m_type == Packets::Type::SET_TARGET_MODE)
		{
			const Packets::SetTargetMode* setTarget = static_cast<const Packets::SetTargetMode*>(&c);

			if(setTarget->m_modeX == Packets::SetTargetMode::AxisMode::LOCK)
			{
				float frac = (float)setTarget->m_fracX / (float)UINT16_MAX;
				commandHandler->lock(CommandHandler::Axis::X, frac);
			}
			else
			{
				commandHandler->free(CommandHandler::Axis::X);
			}

			if(setTarget->m_modeY == Packets::SetTargetMode::AxisMode::LOCK)
			{
				float frac = (float)setTarget->m_fracY / (float)UINT16_MAX;
				commandHandler->lock(CommandHandler::Axis::Y, frac);
			}
			else
			{
				commandHandler->free(CommandHandler::Axis::Y);
			}

			// Write the packet back for any other clients:
			//TODO: This needs to be sent to new clients, too
			appendToBuffer(outputBuffer, *setTarget);
		}
		else
		{
			printf("Got unknown command!\n");
		}
	}
	

	for(auto it = m_clients.begin(); it != m_clients.end();)
	{
		int sentBytes = sendToClient(*it, outputBuffer);
		if(sentBytes != (decltype(sentBytes))outputBuffer.size())
		{
			printf("Lost connection\n");
			m_clients.erase(it);
		}
		else
		{
			it++;
		}
	}
}

ControlServer::Socket::Socket(int sockfd) : m_socket(sockfd) {}

ControlServer::Socket::Socket(Socket&& other)
	: m_socket(other.m_socket)
{
	other.m_socket = -1;
}

ControlServer::Socket& ControlServer::Socket::operator=(Socket&& other)
{
	m_socket = other.m_socket;
	other.m_socket = -1;
	return *this;
}

ControlServer::Socket::~Socket()
{
	if(m_socket >= 0)
	{
		close(m_socket);
	}
}

