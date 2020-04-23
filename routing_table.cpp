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
    std::cout<<  inet_ntoa(in);

} /* print_addr_range() */

void create_message(char address[], int prefix, int distance,  u_int8_t message[9])
{
    char *pch;
    pch = strtok(address, ".");
    for (int k = 0; k < 4; k++)
    {
        u_int8_t c = atoi(pch);
        message[k] = c;
        pch = strtok(NULL, ".");
    }

    message[4] = prefix;
    int dist = distance;
    for (int i = 8; i > 4; i--)
    {
            message[i] = (dist >> (i * 8));
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

void proceed_message(u_int8_t message[], routing_table_row temp_routing_table[], network_addr_t ip_inet[], int num_of_interfaces,int max_rows)
{
    char addr[20];

    sprintf(addr, "%d.%d.%d.%d/%d", message[0], message[1], message[2], message[3], message[4]);
    std::cout<<addr<<std::endl;
    char received_addr[20];
    strcpy(received_addr, addr);
    network_addr_t netaddr = str_to_netaddr(received_addr);
    int dist = (int)(message[5] << 24 | message[6] << 16 | message[7] << 8 | message[8]);

    int index = 0;
    int in_table = 0;
    // routing_table_row r = temp_routing_table[i];
    // while (r.available == 1)
    for(int i = 0; i<max_rows; i++)
    {

        routing_table_row r = temp_routing_table[i];

        if (r.netaddr.addr == netaddr.addr && r.netaddr.pfx == netaddr.pfx)
        {
            // in the table
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

        if(r.available!=0)
            index++;    
    }
    if (in_table == 0)
    {
        // printf("Not in table. ");

        int neigh = 0;
        // int j = 0;
        // while (ip_inet[j].pfx)
        for (int j = 0; j < num_of_interfaces; j++)
        {
            if (ip_inet[j].addr == netaddr.addr && ip_inet[j].pfx == netaddr.pfx)
            {
                // printf("Already on table ************** %s ************\n", addr);

                temp_routing_table[index].netaddr = netaddr;
                temp_routing_table[index].distance = dist;
                temp_routing_table[index].directly = 1;
                temp_routing_table[index].rechable = 0;
                temp_routing_table[index].available = 1;

                neigh = 1;
                break;
            }
            // j++;
        }
        if (neigh == 0)
        {
            // printf("Not a neighbour ************** %s ************\n", addr);

            temp_routing_table[index].netaddr = netaddr;
            temp_routing_table[index].distance = dist + 1;
            temp_routing_table[index].directly = 0;
            temp_routing_table[index].rechable = 0;
            temp_routing_table[index].via_ip_addr = addr;
            temp_routing_table[index].available = 1;
        }
    }
    
}
