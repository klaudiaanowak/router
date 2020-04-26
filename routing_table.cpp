#include "routing_table.h"

int set_reachable(int dist)
{
    if (dist == UNREACHABLE)
        return 0;
    else
        return MAX_REACHABLE;
}

int set_distance(int dist_to_ip, int dist_to_network)
{
    if(dist_to_ip==UNREACHABLE || dist_to_network==UNREACHABLE)
        return UNREACHABLE;
    else
        return dist_to_ip+dist_to_network;
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

in_addr_t netmask(int prefix)
{

    if (prefix == 0)
        return (~((in_addr_t)-1));
    else
        return (~((1 << (32 - prefix)) - 1));

} /* netmask() */

in_addr_t broadcast(in_addr_t addr, int prefix)
{

    return (addr | ~netmask(prefix));

} /* broadcast() */

in_addr_t network(in_addr_t addr, int prefix)
{

    return (addr & netmask(prefix));

} /* network() */

//TOD0: Lepiej zmien nazwę
in_addr_t a_to_hl(char *ipstr)
{

    struct in_addr in;

    if (!inet_aton(ipstr, &in))
    {
        fprintf(stderr, "Invalid address %s!\n", ipstr);
        exit(1);
    }

    return (ntohl(in.s_addr));

} /* a_to_hl() */

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
    netaddr.addr = network(a_to_hl(ipstr), prefix);

    return (netaddr);

} /* str_to_netaddr() */
network_addr_t to_netaddr(in_addr_t addr, char *mask)
{

    long int prefix = 32;
    network_addr_t netaddr;

    prefix = strtol(mask, (char **)NULL, 10);
    netaddr.pfx = (int)prefix;
    netaddr.addr = network(addr, prefix);

    return (netaddr);

} /* str_to_netaddr() */
void print_addr_range(in_addr_t lo)
{

    struct in_addr in;

    in.s_addr = htonl(lo);
    std::cout << inet_ntoa(in);

} /* print_addr_range() */

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
    int dist = r.distance;
    for (int i = 8; i > 4; i--)
    {
        message[i] = (dist >> (i * 8));
    }
}

void read_message(u_int8_t message[], table_row_t *row)
{
    char addr[20] = {};

    sprintf(addr, "%d.%d.%d.%d/%d", message[0], message[1], message[2], message[3], message[4]);

    (*row).netaddr = str_to_netaddr(addr);
    int a = (int)(message[5] | message[6] << 8 | message[7] << 16 | message[9] << 24);

    (*row).distance = a;
    (*row).directly = DIRECT;
    (*row).reachable = 1;
}

void proceed_message(char receiver_ip_str[], u_int8_t message[], routing_table_t *temp_routing_table)
{
    char addr[20];
    sprintf(addr, "%d.%d.%d.%d/%d", message[0], message[1], message[2], message[3], message[4]);

    network_addr_t netaddr = str_to_netaddr(addr);
    int dist = (int)(message[5] << 24 | message[6] << 16 | message[7] << 8 | message[8]);
    network_addr_t receiver_network = str_to_netaddr(receiver_ip_str);
    in_addr_t receiver_ip = a_to_hl(receiver_ip_str);
    int new_addr_index = find_ip_network_index(receiver_network, temp_routing_table);
    if (new_addr_index < 0)
    {
        printf("No matching ip address error");
        exit(EXIT_FAILURE);
    }
    table_row_t new_addr_row = (*temp_routing_table).table_rows[new_addr_index];
    int sum_distance = set_distance(new_addr_row.distance,dist);

    int in_table = 0;
    for (int i = 0; i < (*temp_routing_table).rows_count; i++)
    {

        table_row_t r = (*temp_routing_table).table_rows[i];

        // Case: Network already in table
        if (r.netaddr.addr == netaddr.addr && r.netaddr.pfx == netaddr.pfx)
        {
            r.reachable = set_reachable(dist);

            // Case neighbour
            if (r.directly == DIRECT)
                break;

            // Update distance
            if (r.distance > sum_distance)
            {
                r.distance = sum_distance;
                r.directly = INDIRECT;
                r.router_addr = receiver_ip;
                r.available = 1;
            }
            in_table = 1;
            break;
        }
    }
    // Case: Address not in table
    if (in_table == 0)
    {
        int index = (*temp_routing_table).rows_count;
        (*temp_routing_table).table_rows[index].netaddr = netaddr;
        (*temp_routing_table).table_rows[index].distance = sum_distance;
        (*temp_routing_table).table_rows[index].directly = INDIRECT;
        (*temp_routing_table).table_rows[index].reachable = set_reachable(dist);
        (*temp_routing_table).table_rows[index].router_addr = receiver_ip;
        (*temp_routing_table).table_rows[index].available = 1;
        (*temp_routing_table).rows_count++;
    }
}
