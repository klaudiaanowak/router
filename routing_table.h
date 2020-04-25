#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <string>

const int max_rows = 20;
const int max_reachable = 3;
const int min_reachable = -3;
const int unreachable = 0x7fffffff;
typedef struct network_addr
{
    in_addr_t addr;
    int pfx;
} network_addr_t;

typedef struct
{
    network_addr_t netaddr;
    std::string via_ip_addr;
    in_addr_t router_addr;
    int directly;
    int reachable;
    int distance;
    int available;
} table_row_t;

typedef struct
{
    table_row_t table_rows[max_rows];
    int rows_count;
} routing_table_t;

in_addr_t netmask(int prefix);

in_addr_t broadcast(in_addr_t addr, int prefix);

in_addr_t network(in_addr_t addr, int prefix);

in_addr_t a_to_hl(char *ipstr);

// in_addr a_to_in(char *ipstr);

network_addr_t str_to_netaddr(char *ipstr);
network_addr_t to_netaddr(in_addr_t addr, char *mask);

void print_addr_range(in_addr_t lo);

void create_message(table_row_t *row, u_int8_t message[9]);

void read_message(u_int8_t message[], table_row_t *row);

void proceed_message(char receiver_ip_str[], u_int8_t message[], routing_table_t *temp_routing_table);
