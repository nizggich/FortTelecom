#include "client.h"

static int connect_to_daemon() {
  struct sockaddr_un addr;

  int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sock_fd < 0) {
    perror("Can't create client socket");
    return -1;
  }

  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

  if (connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("Failed to connect! (is daemon running?)");
    close(sock_fd);
    return -1;
  }

  return sock_fd;
}

static int send_to_server(char *message) {
  int sock_fd = connect_to_daemon();
  if (sock_fd < 0) {
    return -1;
  }

  send(sock_fd, message, strlen(message), 0);

  char buf[BUF_SIZE];
  ssize_t n = 0;
  while ((n = recv(sock_fd, buf, sizeof(buf) - 1, 0)) > 0) {
    buf[n] = '\0';
    fprintf(stdout, "%s", buf);
  }

  close(sock_fd);
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s get | set <key=value> ... \n", argv[0]);
    return 1;
  }

  open_log("/var/log/client_log.txt", LOG_INF);

  LOG_I("Client start");

  char req[BUF_SIZE];

  strcpy(req, CMD_GET "\n");
  if (strcmp(argv[1], "get") == 0) {
    strcpy(req, CMD_GET "\n");
    LOG_I("Client send GET request");
  } else if (strcmp(argv[1], "set") == 0) {
    strcpy(req, CMD_SET " ");
    LOG_I("Client send SET request");
    for (int i = 2; i < argc; i++) {
      strcat(req, argv[i]);
      if (i < argc - 1)
        strcat(req, " ");
    }
    strcat(req, "\n");
  }

  send_to_server(req);

  LOG_I("Client stopped");

  close_log();
}
