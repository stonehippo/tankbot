/*
Copyright (C) 2015 George White <stonehippo@gmail.com> All rights reserved.

See https://raw.githubusercontent.com/stonehippo/sploder/master/LICENSE.txt for license details. All rights reserved.
*/

// ******************* Logging and debug helpers *******************
#define DEBUG true

void logMessage(String message) {
  if (DEBUG) {
    Serial.println(message);
  }
}

void info(String message) {
  String fullMessage = "INFO: "+ message;
  logMessage(fullMessage);
}

void warn(String message) {
  String fullMessage = "WARN: "+ message;
  logMessage(fullMessage);
}

void startLog() {
  Serial.begin(57600); // the max rate for a 3.3/8MHz Arduino is 57600, not 115200
  if (DEBUG) {
    info("Now logging to serial");
  }
}
