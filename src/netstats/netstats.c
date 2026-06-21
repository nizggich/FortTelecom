#include "netstats.h"

static unsigned long read_sysfs_ulong(const char *path) {
  FILE *f = fopen(path, "r");
  if (!f)
    return -1;
  unsigned long val = 0;
  if (fscanf(f, "%lu", &val) != 1)
    val = 0;
  fclose(f);
  return val;
}

int netstats_read(const char *ifname, net_stats *out) {
  char path[256];

  snprintf(path, sizeof(path), "/sys/class/net/%s/statistics/rx_bytes", ifname);
  out->rx_bytes = read_sysfs_ulong(path);

  snprintf(path, sizeof(path), "/sys/class/net/%s/statistics/tx_bytes", ifname);
  out->tx_bytes = read_sysfs_ulong(path);

  snprintf(path, sizeof(path), "/sys/class/net/%s/address", ifname);
  FILE *f = fopen(path, "r");

  if (f) {
    if (!fgets(out->mac, sizeof(out->mac), f))
      out->mac[0] = '\0';

    char *nl = strchr(out->mac, '\n');
    if (nl)
      *nl = '\0';
    fclose(f);
  }

  strncpy(out->ifname, ifname, sizeof(out->ifname) - 1);

  out->ip[0] = '\0';
  struct ifaddrs *ifa_list, *ifa;
  if (getifaddrs(&ifa_list) == 0) {
    for (ifa = ifa_list; ifa; ifa = ifa->ifa_next) {
      if (!ifa->ifa_addr)
        continue;
      if (strcmp(ifa->ifa_name, ifname) != 0)
        continue;
      if (ifa->ifa_addr->sa_family != AF_INET)
        continue;

      struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
      inet_ntop(AF_INET, &sa->sin_addr, out->ip, sizeof(out->ip));
      break;
    }
    freeifaddrs(ifa_list);
  }

  return 0;
}
