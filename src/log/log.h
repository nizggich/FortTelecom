#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

typedef enum {
  LOG_INF,
  LOG_WAR,
  LOG_ERR,
} log_level_t;

static log_level_t g_log_level = LOG_INF;
static FILE *g_log_file = NULL;

static inline int open_log(const char *path, log_level_t log_level) {
  g_log_level = log_level;
  g_log_file = fopen(path, "a");
  if (!g_log_file) {
    fprintf(stderr, "Can't open log file: %s", path);
    return -1;
  }
  return 0;
}

static inline void close_log(void) {
  if (g_log_file) {
    fclose(g_log_file);
    g_log_file = NULL;
  }
}

static inline void log_write(log_level_t level, const char *file, int line,
                             const char *fmt, ...) {
  if (!g_log_file) {
    return;
  }

  const char *level_str[] = {"INFO", "WARNING", "ERROR"};

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  char time_buf[20];
  strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", t);

  fprintf(g_log_file, "[%s] [%s] %s:%d: ", time_buf, level_str[level], file,
          line);

  va_list args;
  va_start(args, fmt);
  vfprintf(g_log_file, fmt, args);
  va_end(args);

  fprintf(g_log_file, "\n");
  fflush(g_log_file);
}

#define LOG_W(fmt, ...)                                                        \
  log_write(LOG_WAR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...)                                                        \
  log_write(LOG_ERR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_I(fmt, ...)                                                        \
  log_write(LOG_INF, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif
