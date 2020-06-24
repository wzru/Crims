#include <stdio.h>
#include <string.h>

#include "exec.h"
#include "json.h"
#include "log.h"
#include "server.h"

#if defined(_WIN16) || defined(_WIN32) || defined(_WIN64)
    #include <winsock2.h>
    #include <windows.h>
#else
    #include <arpa/inet.h>
    #include <errno.h>
    #include <netinet/in.h>
    #include <sys/select.h>
    #include <sys/socket.h>
    #include <sys/time.h>
    #include <sys/types.h>
    #include <unistd.h>
#endif

u16 listen_port = DEFAULT_LISTEN_PORT;
int address_length = sizeof (struct sockaddr_in);
char send_buffer[BUFFER_LENGTH];

#if defined(_WIN16) || defined(_WIN32) || defined(_WIN64)
/*
    运行服务器, 绑定端口, 轮询监听请求
*/
inline int run_server()
{
    //初始化Windows Sockets Asynchronous
    WORD socket_version = MAKEWORD (2, 2);
    WSADATA wsa_data;
    if (WSAStartup (socket_version, &wsa_data) != 0)
    {
        plog ("[ERROR]: Initialize WSA failed!\n");
        return -1;
    }
    //创建套接字
    SOCKET server_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET)
    {
        plog ("[ERROR]: Create socket failed!\n");
        return -1;
    }
    //绑定IP和端口
    struct sockaddr_in server_address =
    {
        .sin_family = AF_INET, // TCP/IPv4
        .sin_port = htons (listen_port),
        .sin_addr.S_un.S_addr = INADDR_ANY //任意地址
    };
    if (bind (server_socket, (LPSOCKADDR) &server_address, address_length) ==
            SOCKET_ERROR)
    {
        plog ("[ERROR]: Bind address failed!\n");
        return -1;
    }
    //开始监听
    if (listen (server_socket, 10) == SOCKET_ERROR)
    {
        plog ("[ERROR]: Listen failed!\n");
        return -1;
    }
    plog ("[INFO]: Server has started...\n");
    // select非阻塞式
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
                    SOCKET client_socket = accept (
                                               server_socket, (struct sockaddr *) &client_address,
                                               &address_length);
                    FD_SET (client_socket, &client_sockets);
                    plog ("[INFO]: Client %s connected\n",
                          inet_ntoa (client_address.sin_addr));
                }
                else
                {
                    char recv_buffer[BUFFER_LENGTH];
                    memset (recv_buffer, 0, sizeof (recv_buffer));
                    memset (send_buffer, 0, sizeof (send_buffer));
                    if (recv (client_sockets.fd_array[i], recv_buffer,
                              BUFFER_LENGTH, 0) > 0)
                    {
                        plog ("[INFO]: Received client message: %s\n",
                              recv_buffer);
                        exec (recv_buffer);
                        #ifdef DEBUG
                        printf ("%s\n", json_buffer);
                        #endif
                        Sleep (50); //解决粘包的问题
                        send (client_sockets.fd_array[i], json_buffer,
                              strlen (json_buffer), 0);
                        // send (client_sockets.fd_array[i], recv_buffer, strlen
                        // (recv_buffer), 0);
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

#else

#define backlog 7
#define MAX_CLIENT 10

int client_fds[MAX_CLIENT];
char input_message[BUFFER_LENGTH];
char recv_buffer[BUFFER_LENGTH], send_buffer[BUFFER_LENGTH];

inline int run_server()
{
    int ser_souck_fd;
    int i;
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; // IPV4
    server_address.sin_port = htons (listen_port);
    server_address.sin_addr.s_addr = INADDR_ANY; //指定的是所有地址
    // creat socket
    if ( (ser_souck_fd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
        plog ("[ERROR]: Create socket failed!\n");
        return -1;
    }
    // bind soucket
    if (bind (ser_souck_fd, (const struct sockaddr *) &server_address,
              sizeof (server_address)) < 0)
    {
        plog ("[ERROR]: Bind address failed!\n");
        return -1;
    }
    // listen
    if (listen (ser_souck_fd, backlog) < 0)
    {
        plog ("[ERROR]: Listen failed!\n");
        return -1;
    }
    // fd_set
    fd_set ser_fdset;
    int max_fd = 1;
    struct timeval mytime;
    plog ("[INFO]: Server has started...\n");
    while (1)
    {
        mytime.tv_sec = 27;
        mytime.tv_usec = 0;
        FD_ZERO (&ser_fdset);
        // add standard input
        FD_SET (0, &ser_fdset);
        if (max_fd < 0)
        {
            max_fd = 0;
        }
        // add serverce
        FD_SET (ser_souck_fd, &ser_fdset);
        if (max_fd < ser_souck_fd)
        {
            max_fd = ser_souck_fd;
        }
        // add client
        for (i = 0; i < MAX_CLIENT; i++) //用数组定义多个客户端fd
        {
            if (client_fds[i] != 0)
            {
                FD_SET (client_fds[i], &ser_fdset);
                if (max_fd < client_fds[i])
                {
                    max_fd = client_fds[i];
                }
            }
        }
        // select多路复用
        int ret = select (max_fd + 1, &ser_fdset, NULL, NULL, &mytime);
        if (ret < 0)
        {
            plog ("[ERROR]: Select failed!\n");
            continue;
        }
        else if (ret == 0)
        {
            plog ("[ERROR]: Time out!\n");
            continue;
        }
        else
        {
            if (FD_ISSET (
                        0,
                        &ser_fdset)) //标准输入是否存在于ser_fdset集合中（也就是说，检测到输入时，做如下事情）
            {
                printf ("send message to");
                bzero (input_message, BUFFER_LENGTH);
                fgets (input_message, BUFFER_LENGTH, stdin);
                for (i = 0; i < MAX_CLIENT; i++)
                {
                    if (client_fds[i] != 0)
                    {
                        printf ("client_fds[%d]=%d\n", i, client_fds[i]);
                        // send(client_fds[i], input_message, BUFFER_LENGTH, 0);
                    }
                }
            }
            if (FD_ISSET (ser_souck_fd, &ser_fdset))
            {
                struct sockaddr_in client_address;
                socklen_t address_len;
                int client_sock_fd =
                    accept (ser_souck_fd, (struct sockaddr *) &client_address,
                            &address_len);
                if (client_sock_fd > 0)
                {
                    int flags = -1;
                    //一个客户端到来分配一个fd，MAX_CLIENT=3，则最多只能有三个客户端，超过4以后跳出for循环，flags重新被赋值为-1
                    for (i = 0; i < MAX_CLIENT; i++)
                    {
                        if (client_fds[i] == 0)
                        {
                            flags = i;
                            client_fds[i] = client_sock_fd;
                            break;
                        }
                    }
                    if (flags >= 0)
                    {
                        printf ("New user client[%d] add sucessfully!\n", flags);
                    }
                    else // flags=-1
                    {
                        char full_message[] =
                            "the client is full! can't join!\n";
                        bzero (input_message, BUFFER_LENGTH);
                        strncpy (input_message, full_message, 100);
                        // send(client_sock_fd, input_message, BUFFER_LENGTH,
                        // 0);
                    }
                }
            }
        }
        // deal with the message
        for (i = 0; i < MAX_CLIENT; i++)
        {
            if (client_fds[i] != 0)
            {
                if (FD_ISSET (client_fds[i], &ser_fdset))
                {
                    bzero (recv_buffer, BUFFER_LENGTH);
                    int byte_num =
                        read (client_fds[i], recv_buffer, BUFFER_LENGTH);
                    if (byte_num > 0)
                    {
                        printf ("message form client[%d]:%s\n", i, recv_buffer);
                        exec (recv_buffer);
                        #ifdef DEBUG
                        printf ("%s\n", json_buffer);
                        #endif
                        Sleep (100); //解决粘包的问题
                        send (client_fds[i], json_buffer, strlen (json_buffer),
                              0);
                    }
                    else if (byte_num < 0)
                    {
                        printf ("rescessed error!");
                    }
                    //某个客户端退出
                    else // cancel fdset and set fd=0
                    {
                        printf ("clien[%d] exit!\n", i);
                        FD_CLR (client_fds[i], &ser_fdset);
                        client_fds[i] = 0;
                        // printf("clien[%d] exit!\n",i);
                        continue; //这里如果用break的话一个客户端退出会造成服务器也退出。
                    }
                }
            }
        }
    }
    return 0;
}
#endif