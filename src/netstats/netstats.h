#ifndef NETSATS_H
#define NETSATS_H

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  unsigned long rx_bytes;
  unsigned long tx_bytes;
  char ifname[64];
  char ip[INET_ADDRSTRLEN];
  char mac[64];
} net_stats;

int netstats_read(const char *ifname, net_stats *out);

#endif
