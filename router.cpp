#include "routing_table.h"

const int max_rows = 20;
const int round_time = 30;
//Create routing table
routing_table_row routing_table[max_rows];

in_addr ip_str_to_address(char *ipstr)
{

    struct in_addr in;

    if (!inet_aton(ipstr, &in))
    {
        fprintf(stderr, "Invalid address %s!\n", ipstr);
        exit(1);
    }

    return in;
}

void print_routing_table()
{
    for (int i = 0; i < max_rows; i++)
    {
        routing_table_row r = routing_table[i];
        if (r.available == 1)
        {
            print_addr_range(r.netaddr.addr);
            std::cout<<"/"<<r.netaddr.pfx;
            if (r.rechable < 6)
                std::cout<<" distance "<< r.distance;
            else
                std::cout<<" unreachable ";
            if (r.directly == 1)
                std::cout<<" connected directly "<<std::endl;
            else

                std:: cout<<" via "<<r.via_ip_addr<<std::endl;
        }
    }
}

int main(int argc, char **argv)
{

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "socket error: %s\n", strerror(errno));
        return EXIT_FAILURE;
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
        return EXIT_FAILURE;
    }

    // Get inputs
    int numberOfInterfaces;
    scanf("%d", &numberOfInterfaces);

    network_addr_t ip_inet[numberOfInterfaces];
    in_addr own_addresses[numberOfInterfaces];
    int distances[numberOfInterfaces];

    for (int i = 0; i < numberOfInterfaces; i++)
    {
        char input[INET_ADDRSTRLEN + 20];
        scanf(" %[^\n]s", input);

        char *pch;
        pch = strtok(input, " ");
        ip_inet[i] = str_to_netaddr(pch);
        own_addresses[i] = ip_str_to_address(pch);
        pch = strtok(NULL, " ");
        pch = strtok(NULL, " ");
        distances[i] = strtol(pch, (char **)NULL, 10);
        routing_table[i].netaddr = ip_inet[i];
        routing_table[i].distance = distances[i];
        routing_table[i].directly = 1;
        routing_table[i].rechable = 0;
        routing_table[i].available = 1;
    }
    int counter = round_time;
    for (;;)
    {
        counter++;
        if (counter > round_time)
        {
            print_routing_table();
            counter = 0;
            for (int i = 0; i < numberOfInterfaces; i++)
            {
                in_addr_t broad = broadcast(ip_inet[i].addr, ip_inet[i].pfx);
                struct in_addr in;
                in.s_addr = htonl(broad);
                char *add_to_send = inet_ntoa(in);
                struct sockaddr_in sender;
                bzero(&sender, sizeof(sender));
                sender.sin_family = AF_INET;
                sender.sin_port = htons(54321);
                inet_pton(AF_INET, add_to_send, &sender.sin_addr);

                for (int j = 0; j < max_rows; j++)
                {

                    if (routing_table[j].available == 1)
                    {
                        u_int8_t message[9] = {};
                        create_message(&routing_table[j], message);
                        ssize_t message_len = sizeof(message);

                        int broadcastPermission = 1;

                        setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (void *)&broadcastPermission,
                                   sizeof(broadcastPermission));
                        //TODO: Add timeout 
                        if (sendto(sockfd, message, message_len, 0, (struct sockaddr *)&sender, sizeof(sender)) != message_len)
                        {
                            fprintf(stderr, "sendto error: %s\n", strerror(errno));
                            return EXIT_FAILURE;
                        }

                    }
                }
            }
        }

        struct sockaddr_in receiver;
        socklen_t receiver_len = sizeof(receiver);

        u_int8_t buffer[IP_MAXPACKET + 1];

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 500000;
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
        {
            perror("Error");
        }
        ssize_t datagram_len = recvfrom(sockfd, buffer, IP_MAXPACKET, 0, (struct sockaddr *)&receiver, &receiver_len);

        if (datagram_len < 0)
        {
            continue;
        }

        char receiver_ip_str[20];
        inet_ntop(AF_INET, &(receiver.sin_addr), receiver_ip_str, sizeof(receiver_ip_str));
        int igorne = 0;

        for (int i = 0; i < numberOfInterfaces; i++)
        {
            char own_ip_str[20];
            inet_ntop(AF_INET, &(own_addresses[i]), own_ip_str, sizeof(own_ip_str));

            if (!strcmp(own_ip_str, receiver_ip_str))
            {
                igorne = 1;
                break;
            }
        }
        if (igorne == 1)
        {
            continue;
        }

        sleep(3);
    }
    close(sockfd);
    return EXIT_SUCCESS;
}