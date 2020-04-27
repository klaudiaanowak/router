#include "socket_funcs.h"

int create_broadcast_socket()
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "socket error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    int broadcastPermission = 1;
    int set_result = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST,
                                &broadcastPermission, sizeof(broadcastPermission));
    if (set_result < 0)
    {
        fprintf(stderr, "setsockopt error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

int bind()
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "socket error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(54321);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    char server_ip_str[20];
    inet_ntop(AF_INET, &(server_address.sin_addr), server_ip_str, sizeof(server_ip_str));

    if (bind(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        fprintf(stderr, "bind error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

