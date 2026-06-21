#include "../config/config.h"
#include "../netstats/netstats.h"
#include "../uci_loader/uci_loader.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include <unistd.h>

#define BACKLOG 5

typedef enum {
  REQUEST_GET,
  REQUEST_SET_OK,
  REQUEST_SET_ERR,
  REQUEST_ERROR
} request_type_t;

int server_init(const char *path);
request_type_t handle_client(int server_fd, app_config *cfg, net_stats *stats);
