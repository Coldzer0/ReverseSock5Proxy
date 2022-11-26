//
// Created by Coldzer0 on 2022/11/25.
//
#pragma once
#include "Application.h"

#define UNUSED(x) (void)(x)

#define SERVER_OK 1
#define SERVER_C2_ERROR 2
#define SERVER_SOCK5_ERROR 3
#define SERVER_TUNNEL_ERROR 4

#define C2_CMD_SOCK 1
#define C2_CMD_PING 2
#define C2_CMD_PONG 3
#define C2_CMD_EXIT 4

#define MAX_BUFFER_SIZE 32768

/*-------------------------------------------------------------------*/

#define SOCKS5_VERSION 5

#define SOCKS5_REPLY_SUCCEEDED 0
#define SOCKS5_CMD_TCP_CONNECT 1

#define SOCKS5_ADDR_TYPE_IPV4         1
#define SOCKS5_ADDR_TYPE_DOMAIN_NAME  3
#define SOCKS5_ADDR_TYPE_IPV6         4 // TODO

/*-------------------------------------------------------------------*/

std::thread InitNewServer();
std::thread StartC2Server();