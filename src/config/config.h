#ifndef CONFIG_H
#define CONFIG_H

#define SOCK_PATH "/tmp/myapp.sock"
#define CONF_FILE "uci_cfg"
#define BUF_SIZE 1024

#define CMD_GET "GET"
#define CMD_SET "SET"

typedef struct {
  char ifname[128];
  long interval;
} app_config;

#endif
