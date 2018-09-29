#include "ClientSocket.h"


ClientSocket::ClientSocket(std::string ipaddr, int port):
m_ipaddr(ipaddr), m_port(port)
{
}

ClientSocket::ClientSocket() {

}

ClientSocket::~ClientSocket()
{
	Cleanup();
}

void ClientSocket::FillIn(std::string ipaddr, int port)
{
	m_ipaddr = ipaddr;
	m_port = port;
}

void ClientSocket::setHwnd(HWND hwnd){
	this->hwnd = hwnd;
}

bool ClientSocket::Init()
{
	WSADATA data;
	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
		//MessageBox(hwnd, "Cant initialize winsock", "Error", MB_OK);
		return false;
	}
	return true;
}

void ClientSocket::Run()
{
	m_sock = CreateSocket();

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(m_port);
	hint.sin_addr.s_addr = inet_addr(m_ipaddr.c_str());
	
	int connResult = connect(m_sock, (sockaddr*)&hint, sizeof(hint));
	if (connResult == SOCKET_ERROR)
	{
		//MessageBox(hwnd, "Cant connect to server", "Error", MB_OK);
		system("pause");
		closesocket(m_sock);
		WSACleanup();
		return;
	}
}

void ClientSocket::Send(const char *frame, int size) {

	send(this->getSocket(), frame, size, 0);
}

SOCKET ClientSocket::CreateSocket()
{
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		Cleanup();
	}
	return sock;
}

void ClientSocket::Cleanup()
{
	WSACleanup();
}

SOCKET ClientSocket::getSocket()
{
	return m_sock;
}
