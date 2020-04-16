#include "routing_table.h"

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

void print_addr_range(in_addr_t lo)
{

    struct in_addr in;

    in.s_addr = htonl(lo);
    printf("%s", inet_ntoa(in));

} /* print_addr_range() */

void create_message(routing_table_row *row, u_int8_t message[9])
{
    struct in_addr in;
    routing_table_row r = *row;
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
    for (int i = 0; i < 4; i++)
    {
        message[i + 5] = (dist >> (i * 8));
    }
}

void read_message(u_int8_t message[], routing_table_row *row)
{
    char addr[20] = {};

    sprintf(addr, "%d.%d.%d.%d/%d", message[0], message[1], message[2], message[3], message[4]);

    (*row).netaddr = str_to_netaddr(addr);
    int a = (int)(message[5] | message[6] << 8 | message[7] << 16 | message[9] << 24);

    (*row).distance = a;
    (*row).directly = 1;
    (*row).rechable = 1;
}

void proceed_message(u_int8_t message[], routing_table_row temp_routing_table[], network_addr_t ip_inet[])
{
    char addr[20];

    sprintf(addr, "%d.%d.%d.%d/%d", message[0], message[1], message[2], message[3], message[4]);
    printf("Address: %s\n", addr);

    network_addr_t netaddr = str_to_netaddr(addr);
    int dist = (int)(message[5] | message[6] << 8 | message[7] << 16 | message[9] << 24);

    int i = 0;
    int in_table = 0;
    routing_table_row r = temp_routing_table[i];
    while (r.available == 1)
    {
        routing_table_row r = temp_routing_table[i];
        if (r.netaddr.addr == netaddr.addr && r.netaddr.pfx == netaddr.pfx)
        {
            // jest w tablicy - sprawdz dystans, czy jest w interfejsach
            if (r.distance > dist + 1)
            {
                r.distance = dist + 1;
                r.directly = 0;
                r.rechable = 0;
                r.via_ip_addr = addr;
                r.available = 1;
            }
            in_table = 1;
            break;
        }
        i++;
    }
    if (in_table == 0)
    {
        int neigh = 0;
        int j = 0;
        while (ip_inet[j].addr)
        {
            if (ip_inet[j].addr == netaddr.addr && ip_inet[j].pfx == netaddr.pfx)
            {
                temp_routing_table[i].netaddr = netaddr;
                temp_routing_table[i].distance = dist;
                temp_routing_table[i].directly = 1;
                temp_routing_table[i].rechable = 0;
                temp_routing_table[i].available = 1;
                neigh = 1;
                break;
            }
            j++;
        }
        if (neigh == 0)
        {
            temp_routing_table[i].netaddr = netaddr;
            temp_routing_table[i].distance = dist + 1;
            temp_routing_table[i].directly = 0;
            temp_routing_table[i].rechable = 0;
            temp_routing_table[i].via_ip_addr = addr;
            temp_routing_table[i].available = 1;
        }
    }
}
