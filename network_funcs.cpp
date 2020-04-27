#include "network_funcs.h"
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


void print_addr_range(in_addr_t lo)
{

    struct in_addr in;

    in.s_addr = htonl(lo);
    std::cout << inet_ntoa(in);
}


network_addr_t to_netaddr(in_addr_t addr, int mask)
{

    network_addr_t netaddr;

    netaddr.pfx = mask;
    netaddr.addr = network(addr, mask);

    return (netaddr);
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