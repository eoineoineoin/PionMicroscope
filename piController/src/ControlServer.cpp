#include <ControlServer.hpp>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <errno.h>

ControlServer::ControlServer()
	: m_serverSocket(-1)
{
	int serverFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
	printf("Server FD: %i\n", serverFd);

	m_serverSocket = Socket(serverFd);

	sockaddr_in listenAddr;
	listenAddr.sin_family = AF_INET;
	listenAddr.sin_port = htons(3017);
	listenAddr.sin_addr.s_addr = inet_addr("0.0.0.0");

	int err = bind(serverFd, (sockaddr*)&listenAddr, sizeof(listenAddr));
	printf("Bind: %i errno=%i\n", err, errno);
	err = listen(serverFd, 10);
	printf("Listen: %i errno=%i\n", err, errno);
}

ControlServer::~ControlServer() = default;

void ControlServer::step(const ControlState& curState)
{
	sockaddr_in clientAddr;
	socklen_t clientAddrSize = sizeof(clientAddr);
	int newClient = accept(m_serverSocket.m_socket, (sockaddr*)&clientAddr, &clientAddrSize);
	if(newClient > 0)
	{
		printf("Got new connection from %s\n", inet_ntoa(clientAddr.sin_addr));
		m_clients.emplace_back<Socket>(newClient);
	}

	for(Socket& s : m_clients)
	{
		send(s.m_socket, &curState, sizeof(curState), MSG_DONTWAIT);
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
