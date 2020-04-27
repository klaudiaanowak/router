// Klaudia Nowak
// 297936

#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <string>

const int MAX_ROWS = 20;
const int MAX_REACHABLE = 4;
const int MIN_REACHABLE = -3;
const unsigned int UNREACHABLE = 0xffffffff;
const int INFINITE_DISTANCE = 32;
const int DIRECT = 1;
const int INDIRECT = 0;
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
    unsigned int distance;
} table_row_t;

typedef struct
{
    table_row_t table_rows[MAX_ROWS];
    int rows_count;
} routing_table_t;



network_addr_t to_netaddr(in_addr_t addr, int mask);

network_addr_t str_to_netaddr(char *ipstr);
in_addr_t netmask(int prefix);

in_addr_t broadcast(in_addr_t addr, int prefix);

in_addr_t network(in_addr_t addr, int prefix);

in_addr_t address_from_str(char *ipstr);
void print_addr_range(in_addr_t lo);