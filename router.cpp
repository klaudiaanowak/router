// Klaudia Nowak
// 297936

#include "routing_table.h"
#include "socket_funcs.h"

const int round_time = 15;

//Create routing table
routing_table_t routing_table;
void send_routing_table(int broadcastsocket);
void recive_and_update_routing_table(int socket);

void update_reachability();

void print_routing_table();

int main(int argc, char **argv)
{

    int broadcastsocket = create_broadcast_socket();
    int sockfd = bind();

    // Get inputs
    routing_table.rows_count = 0;
    int numberOfInterfaces;
    scanf("%d", &numberOfInterfaces);

    network_addr_t ip_inet[numberOfInterfaces];
    in_addr own_addresses[numberOfInterfaces];
    int distances[numberOfInterfaces];

    for (int i = 0; i < numberOfInterfaces; i++)
    {

        int dist;
        char addr[INET_ADDRSTRLEN + 20];

        scanf("%s %*s %d", addr, &dist);

        char *subnet_mask_str = strchr(addr, '/') + 1;
        char *router_addr = strtok(addr, "/");

        in_addr_t ip_addr = address_from_str(addr);
        long int prefix = 32;

        prefix = (int)strtol(subnet_mask_str, (char **)NULL, 10);
        routing_table.table_rows[i].netaddr = to_netaddr(ip_addr, prefix);
        routing_table.table_rows[i].router_addr = ip_addr;
        routing_table.table_rows[i].distance = dist;
        routing_table.table_rows[i].directly = DIRECT;
        routing_table.table_rows[i].reachable = MAX_REACHABLE;
        routing_table.rows_count++;
    }
    int counter = round_time;
    for (;;)
    {
        print_routing_table();
        update_reachability();

        send_routing_table(broadcastsocket);
        recive_and_update_routing_table(sockfd);

        sleep(round_time);
    }
    close(sockfd);
    close(broadcastsocket);

    return EXIT_SUCCESS;
}

void print_routing_table()
{

    if (routing_table.rows_count < 1)
    {
        std::cout << "No valid connections " << std::endl;
    }

    for (int i = 0; i < routing_table.rows_count; i++)
    {
        table_row_t r = routing_table.table_rows[i];

        print_addr_range(r.netaddr.addr);
        std::cout << "/" << r.netaddr.pfx;
        if (r.reachable > 0 && r.distance < UNREACHABLE)
            std::cout << " distance " << r.distance;
        else
            std::cout << " unreachable";
        if (r.directly == DIRECT)
            std::cout << " connected directly " << std::endl;
        else
        {
            std::cout << " via ";
            print_addr_range(r.router_addr);
        }
    }
    std::cout << std::endl;
}

void update_reachability()
{
    int i;
    for (int i = 0; i < routing_table.rows_count; i++)
    {
        routing_table.table_rows[i].reachable--;
        if (routing_table.table_rows[i].distance == UNREACHABLE &&
            routing_table.table_rows[i].reachable < MIN_REACHABLE) 
        {
                // neighbour
            routing_table.table_rows[i].reachable = MIN_REACHABLE;
            continue;
        }
        if (routing_table.table_rows[i].reachable >= MIN_REACHABLE &&
            routing_table.table_rows[i].reachable <= 0) 
        {
            //no reachable
            routing_table.table_rows[i].distance = UNREACHABLE;
            if (routing_table.table_rows[i].directly == INDIRECT)
            {
                if (routing_table.table_rows[i].reachable <= MIN_REACHABLE)
                {

                    //remove from table
                    for (int j = i; j < routing_table.rows_count; j++)
                    {
                        if (j < routing_table.rows_count - 1)
                        {
                            routing_table.table_rows[j] = routing_table.table_rows[j + 1];
                        }
                        else
                        {
                            routing_table.table_rows[j] = {0};
                            routing_table.rows_count--;
                        }
                    }
                }
            }
        }
    }
}
void send_routing_table(int broadcastsocket)
{
    for (int i = 0; i < routing_table.rows_count; i++)
    {
        if (routing_table.table_rows[i].directly == DIRECT)
        {
            table_row_t row = routing_table.table_rows[i];
            in_addr_t broad = broadcast(row.router_addr, row.netaddr.pfx);
            struct in_addr in;
            in.s_addr = htonl(broad);
            char *add_to_send = inet_ntoa(in);
            struct sockaddr_in sender;
            bzero(&sender, sizeof(sender));
            sender.sin_family = AF_INET;
            sender.sin_port = htons(54321);
            inet_pton(AF_INET, add_to_send, &sender.sin_addr);

            for (int j = 0; j < routing_table.rows_count; j++)
            {
                if (routing_table.table_rows[j].reachable <= MIN_REACHABLE)
                {
                    continue;
                }
                u_int8_t message[9] = {};
                create_message(&routing_table.table_rows[j], message);
                ssize_t message_len = sizeof(message);

                if (sendto(broadcastsocket, message, message_len, 0, (struct sockaddr *)&sender, sizeof(sender)) < 0)
                {
                    routing_table.table_rows[j].reachable = 0;
                }
            }
        }
    }
}

void recive_and_update_routing_table(int socket)
{

    table_row_t new_row;
    int is_socket_ready;

    while (true)
    {
        fd_set descriptors;
        FD_ZERO(&descriptors);
        FD_SET(socket, &descriptors);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 10000;

        int ready = select(socket + 1, &descriptors, NULL, NULL, &tv);
        if (ready < 0)
        {
            fprintf(stderr, "select socket error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (ready == 0)
        {
            break;
        }

        struct sockaddr_in receiver;
        socklen_t receiver_len = sizeof(receiver);

        u_int8_t buffer[IP_MAXPACKET + 1];

        ssize_t datagram_len = recvfrom(socket, buffer, IP_MAXPACKET, 100, (struct sockaddr *)&receiver, &receiver_len);
        if (datagram_len < 0)
        {
            fprintf(stderr, "Receive error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        char receiver_ip_str[20];
        inet_ntop(AF_INET, &(receiver.sin_addr), receiver_ip_str, sizeof(receiver_ip_str));
        proceed_message(receiver_ip_str, buffer, &routing_table);
    }
}