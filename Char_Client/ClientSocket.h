#pragma once
#include <string>
#include <Windows.h>
#include <iostream>
#include <mutex>
#pragma comment(lib, "ws2_32.lib")
#include <opencv2/opencv.hpp>


class ClientSocket
{
private:
	std::string m_ipaddr;
	int m_port;
	SOCKET m_sock;
	HWND hwnd;
public:
	ClientSocket( std::string ipaddr, int port);
	ClientSocket();
	~ClientSocket();
	void FillIn(std::string ipaddr, int port);
	void setHwnd(HWND hwnd);
	bool Init();
	void Run();
	void Send(const char*, int);
	SOCKET CreateSocket();
	void Cleanup();
	SOCKET getSocket();
};

