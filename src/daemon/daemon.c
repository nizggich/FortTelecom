#include "../log/log.h"
#include "../netstats/netstats.h"
#include "../server/server.h"
#include "../uci_loader/uci_loader.h"
#include "fcntl.h"
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <syslog.h>
#include <uci.h>
#include <unistd.h>

static volatile sig_atomic_t d_running = 1;
static app_config g_cfg;
static net_stats g_stats;

static void signal_handler(int sig) {
  (void)sig;
  d_running = 0;
}

static void set_time(int timer_fd, unsigned long interval_sec) {
  struct itimerspec its = {
      .it_interval = {.tv_sec = interval_sec, .tv_nsec = 0},
      .it_value = {.tv_sec = interval_sec, .tv_nsec = 0}};

  timerfd_settime(timer_fd, 0, &its, NULL);
}

static int setup_timer(unsigned long interval_sec) {
  int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
  if (fd < 0)
    return -1;
  set_time(fd, interval_sec);

  return fd;
}

static void daemonize(void) {
  switch (fork()) {
  case -1:
    exit(EXIT_FAILURE);
  case 0:
    break;
  case 1:
    exit(EXIT_SUCCESS);
  }

  if (setsid() < 0)
    exit(EXIT_FAILURE);

  switch (fork()) {
  case -1:
    exit(EXIT_FAILURE);
  case 0:
    break;
  case 1:
    exit(EXIT_SUCCESS);
  }

  umask(0);

  if (chdir("/") < 0)
    exit(EXIT_FAILURE);

  int nfd = open("/dev/null", O_RDWR);
  if (nfd < 0) {
    exit(EXIT_FAILURE);
  }

  dup2(nfd, STDIN_FILENO);
  dup2(nfd, STDOUT_FILENO);
  dup2(nfd, STDERR_FILENO);

  close(nfd);
}

int main(int argc, char *argv[]) {
  open_log("/var/log/server_log.txt", LOG_INF);

  int foreground = 0;
  if (argc > 1 && strcmp(argv[1], "-f") == 0) {
    foreground = 1;
    LOG_I("Standart process start");
  }
  if (!foreground) {
    daemonize();
  }

  if (config_load(&g_cfg) < 0) {
    fprintf(stderr, "ERROR LOAD CONFIG");
    LOG_E("Failed to load config, using defaults");
    strcpy(g_cfg.ifname, "eth0");
    g_cfg.interval = 10;
  }

  netstats_read(g_cfg.ifname, &g_stats);

  LOG_I("Daemon started: ifname = %s, interval = %lu", g_cfg.ifname,
        g_cfg.interval);

  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);
  signal(SIGPIPE, SIG_IGN);

  int sock_fd = server_init(SOCK_PATH);
  int timer_fd = setup_timer(g_cfg.interval);

  if (sock_fd < 0 || timer_fd < 0) {
    perror("ERROR socket, timer_fd");
    LOG_E("Failed to init fd: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }

  struct pollfd fds[2] = {{.fd = sock_fd, .events = POLLIN},
                          {.fd = timer_fd, .events = POLLIN}};

  while (d_running) {
    int ret = poll(fds, 2, -1);
    if (ret < 0) {
      if (errno == EINTR)
        continue;
      LOG_E("poll error: %s", strerror(errno));
      break;
    }

    if (fds[1].revents & POLLIN) {
      netstats_read(g_cfg.ifname, &g_stats);
      LOG_I("Netstats updated: rx = %lu, tx = %lu", g_stats.rx_bytes,
            g_stats.tx_bytes, g_cfg.interval);
    }

    if (fds[0].revents & POLLIN) {
      switch (handle_client(sock_fd, &g_cfg, &g_stats)) {
      case REQUEST_SET_OK:
        set_time(timer_fd, g_cfg.interval);
        LOG_I("App config changed successfully: ifname: = %s, interval = %lu",
              g_cfg.ifname, g_cfg.interval);
        break;
      case REQUEST_SET_ERR:
        LOG_E("Failed to change app config");
        break;
      case REQUEST_GET:
        LOG_I("Netstats send successfully");
        break;
      case REQUEST_ERROR:
        LOG_E("Failed to process client request");
        break;
      }
    }
  }

  close(sock_fd);
  close(timer_fd);
  unlink(SOCK_PATH);
  LOG_I("Daemon stopped");
  close_log();
  return 0;
}
