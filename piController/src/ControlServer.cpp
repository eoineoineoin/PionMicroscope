#include <ControlServer.hpp>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <errno.h>
#include <algorithm>

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
		printf("Could not bind to socket (%i)", errno);
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

void ControlServer::step(const Packets::CurrentState& curState)
{
	sockaddr_in clientAddr;
	socklen_t clientAddrSize = sizeof(clientAddr);
	int newClient = accept(m_serverSocket.m_socket, (sockaddr*)&clientAddr, &clientAddrSize);
	if(newClient > 0)
	{
		printf("Got new connection from %s\n", inet_ntoa(clientAddr.sin_addr));
		m_clients.emplace_back<Socket>(newClient);
	}

	for(auto it = m_clients.begin(); it != m_clients.end();)
	{
		int sentBytes = send(it->m_socket, &curState, sizeof(curState), MSG_DONTWAIT | MSG_NOSIGNAL);
		if(sentBytes != sizeof(curState))
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

