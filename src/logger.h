#ifndef LOGGER
#define LOGGER

#include <Arduino.h>

class Logger {
  public:
    enum Level {
      INFO,
      WARN,
      ERROR
    };
    void printlog(Level level, String message);
};

#endif
