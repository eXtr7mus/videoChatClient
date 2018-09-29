#include <Windows.h>
#include "resource.h"
#include <string>
#include "ClientSocket.h"
#include <sstream>
#include <algorithm>
#include <thread>
#include <opencv2/opencv.hpp>
#pragma comment(lib, "ws2_32.lib")

const int MAX_BUF_SIZE = 2073600;
const char g_czClassName[] = "myClass";
std::string ipaddr, port;
std::string GetTextEditMsg(HWND hwnd, int idcHandle);
void consoleWrite(HWND hwnd, std::string msg);
void receiveFrame(HWND hwnd, ClientSocket sock);
void videoSending(HWND hwnd, ClientSocket sock);

LRESULT CALLBACK DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM) {
	switch (Message) {
		case WM_INITDIALOG:
			SetDlgItemText(hwnd, IDC_TEXT, "192.168.0.1");
			SetDlgItemText(hwnd, IDC_TEXT2, "53500");
		break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK: {
					ipaddr = GetTextEditMsg(hwnd, IDC_TEXT);
					port = GetTextEditMsg(hwnd, IDC_TEXT2);
					//check port for correct numbers
					if (!(port.end() == std::find_if_not(port.begin(), port.end(), ::isdigit))) {
						MessageBox(0, "Incorrect port", "Err", MB_ICONEXCLAMATION | MB_OK);
					}
					else {
						EndDialog(hwnd, IDOK);
					}
				}
				break;
				case IDCANCEL:
					EndDialog(hwnd, IDCANCEL);
				break;
			}
		break;
		default:
			return FALSE;
	}
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam){
	std::string msg = "";


	switch (Message) {
		case WM_CREATE: {
			HFONT hfDefault;
			HWND hEditMsgShow;

			hEditMsgShow = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
				WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY,
				0, 0, 400, 400,
				hwnd, (HMENU)IDC_MAIN_MESSAGES, GetModuleHandle(NULL), NULL);

			if (hEditMsgShow == NULL) {
				MessageBox(hwnd, "Cant create edit box", "Err", MB_OK);
			}
			hfDefault = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			SendMessage(hEditMsgShow, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
		}
		break;
		case WM_SIZE: {
			HWND hEdit;
			RECT rcClient;

			GetClientRect(hwnd, &rcClient);
			hEdit = GetDlgItem(hwnd, IDC_MAIN_MESSAGES);
			SetWindowPos(hEdit, NULL, 0, 0, rcClient.right, 300, SWP_NOZORDER);
		}
		break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case ID_FILE_EXIT: {
					SendMessage(hwnd, WM_CLOSE, 0, 0);
				}
			break;
			}
		break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
		break;
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	std::cout << "Hello" << std::endl;
	WNDCLASSEX wc;
	HWND hWnd;
	MSG msg;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = g_czClassName;
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc)) {
		MessageBox(0, "Window Reg Failed", "Err", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	ClientSocket client;

	hWnd = CreateWindowEx(WS_EX_CLIENTEDGE, g_czClassName,
		"Chat client", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		600, 400, NULL, NULL, hInstance, (LPVOID)&client);
	if (hWnd == NULL) {
		MessageBox(0, "Window Creation Failed", "Err", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	int ret = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG1), hWnd, DlgProc);
	if (ret == -1) {
		MessageBox(hWnd, "DIALOG FAILED", "DIALOG FAILED", MB_OK);
	}
	
	client.FillIn(ipaddr, atoi(port.c_str()));
	client.setHwnd(hWnd);
	consoleWrite(hWnd, "Trying to connect the server... \r\n");

	if (client.Init()) {
		client.Run();
	}

	std::thread sendingThread(videoSending, std::ref(hWnd), std::ref(client));
	sendingThread.detach();
	std::thread receiveThread(receiveFrame, std::ref(hWnd), std::ref(client));
	receiveThread.detach();

	
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		if (msg.message == WM_KEYDOWN) SendMessage(hWnd, WM_KEYDOWN, msg.wParam, msg.lParam);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

std::string GetTextEditMsg(HWND hwnd, int idcHandle) {
	int len = GetWindowTextLength(GetDlgItem(hwnd, idcHandle));
	if (len > 0) {
		char *buf;
		buf = (char*)GlobalAlloc(GPTR, len + 1);
		GetDlgItemText(hwnd, idcHandle, buf, len + 1);
		std::string result(buf);
		GlobalFree((HANDLE)buf);
		return result;
	}
	return nullptr;
}

void consoleWrite(HWND hwnd, std::string msg)
{
	HWND hEdit = GetDlgItem(hwnd, IDC_MAIN_MESSAGES);
	int ndx = GetWindowTextLength(hEdit);
	SetFocus(hEdit);
	SendMessage(hEdit, EM_SETSEL, (WPARAM)ndx, (LPARAM)ndx);
	SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)((LPSTR)msg.c_str()));
}

void videoSending (HWND hwnd, ClientSocket sock) {
	cv::VideoCapture cap;
	consoleWrite(hwnd, "Opening webcam... \r\n");
	if (!cap.open(0)) {
		MessageBox(0, "Cant find default webcam", "Err", MB_ICONEXCLAMATION | MB_OK);
		return;
	}
	
	while(true)
	{
		cv::Mat frame;
		cap >> frame;
		if (frame.empty()) break; // end of video stream
		std::vector<uchar> imgBuf;
		std::vector<int> quality_params = std::vector<int>(2);              
		quality_params[0] = CV_IMWRITE_JPEG_QUALITY; 
		quality_params[1] = 10;
		cv::imencode(".jpg", frame, imgBuf, quality_params);
		std::string data(imgBuf.begin(), imgBuf.end());
		int IResult = send(sock.getSocket(), data.c_str(), data.size(), 0);
	}
}

void receiveFrame(HWND hwnd, ClientSocket sock) {
	char buf[200000];
	consoleWrite(hwnd, "Receiving frames... \r\n");
	while (true) {
		std::vector<uchar> videoBuffer;
		
		IplImage img;
		int iResult = recv(sock.getSocket(), &buf[0], MAX_BUF_SIZE, 0);
		if (iResult == 0) {
			consoleWrite(hwnd, "Client disconnected. \r\n");
			break;
		}
		if (iResult > 0) {
			cv::Mat jpegimage;
			videoBuffer.resize(iResult); 
			memcpy((char*)(&videoBuffer[0]), buf, iResult);
			jpegimage = cv::imdecode(videoBuffer, cv::IMREAD_COLOR);
			std::cout << "result: " << iResult << std::endl;
			if (jpegimage.rows != 0 && jpegimage.cols != 0) {
				cv::imshow("Hello!", jpegimage);
			}
			cvWaitKey(80);
		}
	}
}