#include "server.h"

int server_init(const char *path) {
  struct sockaddr_un addr;

  int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sock_fd < 0) {
    return -1;
  }

  unlink(path);

  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

  if (bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(sock_fd);
    return -1;
  }

  if (listen(sock_fd, BACKLOG) < 0) {
    close(sock_fd);
    return -1;
  }

  return sock_fd;
}

static request_type_t handle_get(int client_fd, const app_config *cfg,
                                 const net_stats *stats) {
  char buf[BUF_SIZE];
  int n = snprintf(buf, sizeof(buf),
                   "ifname=%s\n"
                   "interval=%lu\n"
                   "rx_bytes=%lu\n"
                   "tx_bytes=%lu\n"
                   "ip=%s\n"
                   "mac=%s\n"
                   "status=OK\n",
                   cfg->ifname, cfg->interval, stats->rx_bytes, stats->tx_bytes,
                   stats->ip, stats->mac);
  send(client_fd, buf, n, 0);
  return REQUEST_GET;
}

static request_type_t handle_set(int client_fd, app_config *cfg,
                                 char *message) {
  char *new_str;
  bool changed = false;
  char *tmp = strdup(message);
  char *tok = strtok_r(tmp, " \t\n", &new_str);

  while (tok) {
    char key[64], val[64];
    if (sscanf(tok, "%63[^=]=%63s", key, val) == 2) {
      if (strcmp(key, "ifname") == 0) {
        strncpy(cfg->ifname, val, sizeof(cfg->ifname) - 1);
        changed = true;
      } else if (strcmp(key, "time") == 0) {
        cfg->interval = strtoll(val, NULL, 10);
        changed = true;
      }
    }
    tok = strtok_r(NULL, " \t\n", &new_str);
  }
  free(tmp);

  if (changed) {
    if (config_save(cfg) < 0) {
      char *msg = "status=ERR\n";
      send(client_fd, msg, strlen(msg), 0);
      return REQUEST_ERROR;
    }
  }

  char *msg = "status=OK\n";
  send(client_fd, msg, strlen(msg), 0);
  return REQUEST_SET_OK;
}

request_type_t handle_client(int server_fd, app_config *cfg, net_stats *stats) {
  int client_fd = accept(server_fd, NULL, 0);
  if (client_fd < 0) {
    return REQUEST_ERROR;
  }

  request_type_t res = REQUEST_ERROR;

  char buf[BUF_SIZE];
  int cmd_offset = 4;
  ssize_t n = recv(client_fd, buf, sizeof(buf) - 1, 0);
  if (n > 0) {
    buf[n] = '\0';

    char *nl = strchr(buf, '\n');
    if (nl)
      *nl = '\0';

    if (strncmp(buf, CMD_GET, 3) == 0) {
      res = handle_get(client_fd, cfg, stats);
    } else if (strncmp(buf, CMD_SET " ", cmd_offset) == 0) {
      res = handle_set(client_fd, cfg, buf + cmd_offset);
    } else {
      char *err_msg = "status=ERROR; unknown type of message\n";
      send(client_fd, err_msg, strlen(err_msg), 0);
      res = REQUEST_ERROR;
    }
  }
  close(client_fd);
  return res;
}
