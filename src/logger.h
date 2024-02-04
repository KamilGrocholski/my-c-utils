#ifndef _LOGGER_H
#define _LOGGER_H

typedef enum {
  LOG_INFO,
  LOG_WARNING,
  LOG_ERROR,
} LogLevel;

void logger_log(LogLevel level, const char *fmt, ...);

#endif // _LOGGER_H
