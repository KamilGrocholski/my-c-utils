#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include "logger.h"

void logger_log(LogLevel level, const char *fmt, ...) {
  switch (level) {
  case LOG_INFO:
    fprintf(stderr, "[INFO] ");
    break;
  case LOG_WARNING:
    fprintf(stderr, "[WARNING] ");
    break;
  case LOG_ERROR:
    fprintf(stderr, "[ERROR] ");
    break;
  case LOG_FATAL:
    fprintf(stderr, "[FATAL] ");
    break;
  default:
    assert(0 && "logger_log unreachable level");
    break;
  }

  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
}
