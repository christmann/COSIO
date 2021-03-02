#include "logger.h"

void Logger::printlog(Level level, String message){

  String loglevel;

  switch(level){
    case ERROR:
      loglevel = "ERROR";
      break;
    case WARN:
      loglevel = "WARN";
      break;
    case INFO:
      loglevel = "INFO";
      break;
  }

  Serial.println("[" + (String) millis() + "] [" + loglevel + "] " + message);
}
