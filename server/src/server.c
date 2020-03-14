#include <winsock2.h>
#include <windows.h>
#include <string.h>
#include <stdio.h>

#include "server.h"
#include "exec.h"

//#pragma comment(lib,"ws2_32.lib")

int listen_port = DEFAULT_LISTEN_PORT;
int address_length = sizeof (struct sockaddr_in);
char send_buffer[BUFFER_LENGTH];

inline int run_server()
{
    //初始化Windows Sockets Asynchronous
    WORD socket_version = MAKEWORD (2, 2);
    WSADATA wsa_data;
    if (WSAStartup (socket_version, &wsa_data) != 0)
    {
        printf ("Initialize WSA ERROR!\n");
        return -1;
    }
    //创建套接字
    SOCKET server_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET)
    {
        printf ("Create socket ERROR!\n");
        return -1;
    }
    //绑定IP和端口
    struct sockaddr_in server_address =
    {
        .sin_family = AF_INET,//TCP/IPv4
        .sin_port = htons (listen_port),
        .sin_addr.S_un.S_addr = INADDR_ANY//任意地址
    };
    if (bind (server_socket, (LPSOCKADDR) &server_address,
              address_length) == SOCKET_ERROR)
    {
        printf ("Bind ERROR!\n");
        return -1;
    }
    //开始监听
    if (listen (server_socket, 10) == SOCKET_ERROR)
    {
        printf ("Listen ERROR!\n");
        return -1;
    }
    printf ("Server has been started...\n");
    //select非阻塞式
    fd_set client_sockets;
    FD_ZERO (&client_sockets);
    FD_SET (server_socket, &client_sockets); //将server_socket添加进该集合
    while (1)
    {
        fd_set fdRead = client_sockets;
        if (select (0, &fdRead, NULL, NULL, NULL) <= 0)
        {
            break;
        }
        for (int i = 0; i < (int) client_sockets.fd_count; ++i)
        {
            if (FD_ISSET (client_sockets.fd_array[i], &fdRead))
            {
                if (client_sockets.fd_array[i] == server_socket)
                {
                    struct sockaddr_in client_address;
                    SOCKET client_socket = accept (server_socket,
                                                   (struct sockaddr *) &client_address,
                                                   &address_length);
                    FD_SET (client_socket, &client_sockets);
                    printf ("Client %s connected\n", inet_ntoa (client_address.sin_addr));
                }
                else
                {
                    char recv_buffer[BUFFER_LENGTH];
                    memset (recv_buffer, 0, sizeof (recv_buffer));
                    memset (send_buffer, 0, sizeof (send_buffer));
                    if (recv (client_sockets.fd_array[i], recv_buffer, BUFFER_LENGTH, 0) > 0)
                    {
                        printf ("Received client message:%s\n", recv_buffer);
                        exec (recv_buffer);
                        send (client_sockets.fd_array[i], recv_buffer, strlen (recv_buffer), 0);
                    }
                    else
                    {
                        closesocket (client_sockets.fd_array[i]);
                        FD_CLR (client_sockets.fd_array[i], &client_sockets);
                    }
                }
            }
        }
    }
    closesocket (server_socket);
    WSACleanup();
    return 0;
}