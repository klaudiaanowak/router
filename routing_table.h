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
#include "network_funcs.h"

void create_message(table_row_t *row, u_int8_t message[9]);

void read_message(u_int8_t message[], table_row_t *row);

void proceed_message(char receiver_ip_str[], u_int8_t message[], routing_table_t *routing_table);
