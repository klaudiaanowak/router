#include "routing_table.h"

int set_reachable(int dist)
{
    if (dist == UNREACHABLE && dist <0)
        return 0;
    else
        return MAX_REACHABLE;
}

int set_distance(int dist_to_ip, int dist_to_network)
{
    if (dist_to_ip == UNREACHABLE || dist_to_network == UNREACHABLE)
        return UNREACHABLE;
    else
        return dist_to_ip + dist_to_network;
}
int match_ip_to_network(in_addr_t address, routing_table_t *routing_table)
{
    for (int i = 0; i < (*routing_table).rows_count; i++)
    {
        table_row_t r = (*routing_table).table_rows[i];
        in_addr_t first = network(r.netaddr.addr, r.netaddr.pfx);
        in_addr_t last = broadcast(r.netaddr.addr, r.netaddr.pfx);
        if (first < address && last > address)
        {
            return i;
        }
    }
    return -1;
}
int find_ip_network_index(network_addr_t network, routing_table_t *routing_table)
{

    for (int i = 0; i < (*routing_table).rows_count; i++)
    {
        table_row_t r = (*routing_table).table_rows[i];
        if (r.netaddr.addr == network.addr && r.netaddr.pfx == network.pfx)
        {
            return i;
        }
    }
    return -1;
}
int find_ip_address_index(in_addr_t address, routing_table_t *routing_table)
{

    for (int i = 0; i < (*routing_table).rows_count; i++)
    {
        table_row_t r = (*routing_table).table_rows[i];

        if (r.router_addr == address)
        {
            return i;
        }
    }
    return -1;
}
in_addr_t netmask(int prefix)
{

    if (prefix == 0)
        return (~((in_addr_t)-1));
    else
        return (~((1 << (32 - prefix)) - 1));

} 

in_addr_t broadcast(in_addr_t addr, int prefix)
{

    return (addr | ~netmask(prefix));

} 

in_addr_t network(in_addr_t addr, int prefix)
{

    return (addr & netmask(prefix));

} 

in_addr_t address_from_str(char *ipstr)
{

    struct in_addr in;

    if (!inet_aton(ipstr, &in))
    {
        fprintf(stderr, "Invalid address %s!\n", ipstr);
        exit(1);
    }

    return (ntohl(in.s_addr));

} 

network_addr_t str_to_netaddr(char *ipstr)
{

    long int prefix = 32;
    char *prefixstr;
    network_addr_t netaddr;

    if ((prefixstr = strchr(ipstr, '/')))
    {
        *prefixstr = '\0';
        prefixstr++;
        prefix = strtol(prefixstr, (char **)NULL, 10);
        if ((*prefixstr == '\0') || (prefix < 0) || (prefix > 32))
        {
            fprintf(stderr, "Invalid prefix /%s...!\n", prefixstr);
            exit(1);
        }
    }

    netaddr.pfx = (int)prefix;
    netaddr.addr = network(address_from_str(ipstr), prefix);

    return (netaddr);

} 
network_addr_t to_netaddr(in_addr_t addr, int mask)
{

    network_addr_t netaddr;

    netaddr.pfx = mask;
    netaddr.addr = network(addr, mask);

    return (netaddr);

} 
void print_addr_range(in_addr_t lo)
{

    struct in_addr in;

    in.s_addr = htonl(lo);
    std::cout << inet_ntoa(in);

} 

void create_message(table_row_t *row, u_int8_t message[9])
{
    struct in_addr in;
    table_row_t r = *row;
    in.s_addr = htonl(r.netaddr.addr);
    char *address = inet_ntoa(in);

    char *pch;
    pch = strtok(address, ".");

    for (int k = 0; k < 4; k++)
    {
        u_int8_t c = atoi(pch);
        message[k] = c;
        pch = strtok(NULL, ".");
    }

    message[4] = r.netaddr.pfx;
    unsigned int dist = htonl(r.distance);
    for (int i = 8; i > 4; i--)
    {
        message[i] = (dist >> (i * 8));
    }
}

void proceed_message(char sender_ip_str[], u_int8_t message[], routing_table_t *routing_table)
{
    char received_network_addr[20];
    sprintf(received_network_addr, "%d.%d.%d.%d/%d", message[0], message[1], message[2], message[3], message[4]);

    network_addr_t netaddr = str_to_netaddr(received_network_addr);
    unsigned int dist = (unsigned int)(message[5] << 24 | message[6] << 16 | message[7] << 8 | message[8]);

    dist = ntohl(dist);
    in_addr_t sender_ip = address_from_str(sender_ip_str);
// index of received network
    int sender_network_index = find_ip_network_index(netaddr, routing_table); 
    // index to sender's network
      int sender_address_index = match_ip_to_network(sender_ip, routing_table);


    if (sender_network_index < 0)
    {
        //not in table
        if (sender_address_index < 0)
        {
            printf("No matching ip address error\n");
            exit(EXIT_FAILURE);
        }
        // Add new row to the table
        int index = (*routing_table).rows_count;
        int sum_distance = set_distance((*routing_table).table_rows[sender_address_index].distance, dist);

        (*routing_table).table_rows[index].netaddr = netaddr;
        (*routing_table).table_rows[index].distance = sum_distance;
        (*routing_table).table_rows[index].directly = INDIRECT;
        (*routing_table).table_rows[index].reachable = set_reachable(dist);
        (*routing_table).table_rows[index].router_addr = sender_ip;
        (*routing_table).table_rows[index].available = 1;
        (*routing_table).rows_count++;
    }
    else
    {
        // network is in table
        if ((*routing_table).table_rows[sender_network_index].directly == DIRECT && (*routing_table).table_rows[sender_network_index].router_addr == sender_ip)
        {
            // received message from yourself
            return;
        }


        // Neighbour sends info of my neighbours 
        if ((*routing_table).table_rows[sender_network_index].directly == DIRECT && sender_network_index != sender_address_index)
        {
            return;
        }

        // Check if network is reachable
        (*routing_table).table_rows[sender_network_index].reachable = set_reachable(dist);


        // Neighbour sends about himself - info of direct networks from source
        if ((*routing_table).table_rows[sender_network_index].directly == DIRECT && sender_network_index == sender_address_index)
        {
            (*routing_table).table_rows[sender_network_index].distance = dist;
            return;
        }
 
        // Update distance of indirect networks
        int sum_distance = set_distance((*routing_table).table_rows[sender_address_index].distance, dist);
        if ((*routing_table).table_rows[sender_network_index].distance > sum_distance)
        {

            (*routing_table).table_rows[sender_network_index].distance = sum_distance;
            (*routing_table).table_rows[sender_network_index].directly = INDIRECT;
            (*routing_table).table_rows[sender_network_index].router_addr = sender_ip;
            (*routing_table).table_rows[sender_network_index].available = 1;
        }
    }
}
