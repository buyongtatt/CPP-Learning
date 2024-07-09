#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <tchar.h>
#include <thread>
#include <vector>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

bool Initialize() {
	WSADATA data;
	return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

void InteractWithClient(SOCKET clientSocket, vector<SOCKET> &clients) {
	char buffer[4096];

	while (1) {
		int bytesrecv = recv(clientSocket, buffer, sizeof(buffer), 0);

		if (bytesrecv <= 0) {
			cout << "client disconnected" << endl;
			break;
		}

		string message(buffer, bytesrecv);
		cout << "message from client: " << message << endl;

		for (auto client : clients) {
			if (client != clientSocket) {
				send(client, message.c_str(), message.length(), 0);
			}
		}
	}

	closesocket(clientSocket);
}

int main() {
	if (!Initialize()) {
		cout << "winsock initialization failed" << endl;		
		WSACleanup();
		return 1;
	}


	cout << "Server" << endl;

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (listenSocket == INVALID_SOCKET) {
		cout << "Socket creation failed" << endl;
		WSACleanup();
		return 1;
	}

	sockaddr_in serveraddr;
	int port = 12345;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);

	if (InetPton(AF_INET, _T("0.0.0.0"), &serveraddr.sin_addr) != 1) {
		cout << "setting address structure failed" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
		cout << "bind failed" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		cout << "listen failed" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	cout << "server has started listening on port: " << port << endl;
	vector<SOCKET> clients;
	SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);

	while (1) {

		if (clientSocket == INVALID_SOCKET) {
			cout << "invalid client socket" << endl;
		}

		clients.push_back(clientSocket);

		thread t1(InteractWithClient, clientSocket, std::ref(clients));
		t1.detach();
	}

	
	auto it = find(clients.begin(), clients.end(), clientSocket);

	if (it != clients.end()) {
		clients.erase(it);
	}
	

	closesocket(listenSocket);

	WSACleanup();

	return 0;
}