#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <string>

typedef struct network_addr
{
    in_addr_t addr;
    int pfx;
} network_addr_t;

typedef struct routing_table_row
{
    network_addr_t netaddr;
    std::string via_ip_addr;
    int directly;
    int rechable;
    int distance;
    int available;
} routing_table_row;


in_addr_t netmask(int prefix);

in_addr_t broadcast(in_addr_t addr, int prefix);

in_addr_t network(in_addr_t addr, int prefix);

in_addr_t a_to_hl(char *ipstr);

// in_addr a_to_in(char *ipstr);

network_addr_t str_to_netaddr(char *ipstr);

void print_addr_range(in_addr_t lo);

void create_message(routing_table_row *row, u_int8_t message[9]);

void read_message(u_int8_t message[], routing_table_row *row);


void proceed_message(u_int8_t message[], routing_table_row temp_routing_table[], network_addr_t ip_inet[], int num_of_interfaces, int max_rows);

