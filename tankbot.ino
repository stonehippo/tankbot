// put aREST in lightweight mode to reduce program size
#define LIGHTWEIGHT 1
#include <FiniteStateMachine.h>
#include <Adafruit_CC3000.h>
#include <SPI.h>
#include <aREST.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "LogHelpers.h"
#include "TimingHelpers.h"
#include "wlan.h" // put your 

// disable debug output
#define DEBUG true

// FSM states
State startupState = State(enterStartupState,updateStartupState,leaveStartupState);
State idleState = State(enterIdleState,updateIdleState,leaveIdleState);
State forwardState = State(enterForwardState, updateForwardState, leaveForwardState);
State backwardState = State(enterBackwardState, updateBackwardState, leaveBackwardState);

FSM stateMachine = FSM(startupState);  

// These are the pins for the CC3000 chip if you are using a breakout board
#define ADAFRUIT_CC3000_IRQ   3
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10

// Create CC3000 instance
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIV2);

/*
  Your WiFi SSID and password
  don't actually do this here. Create a local wlan.h and put these #defines in it. That file will be excluded
  from commits to github, for privacy and security reasons. Remember, never commit security secrets to a public
  repo!
*/
// #define WLAN_SSID       "yourSSID"
// #define WLAN_PASS       "yourPassword"
// #define WLAN_SECURITY   WLAN_SEC_WPA2

#define LISTEN_PORT  80
Adafruit_CC3000_Server restServer(LISTEN_PORT);

aREST rest = aREST();

// PWM driver board
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// define PWM channels that will tie to the motor driver board
const byte MOTOR_A_PWM_CHANNEL = 0;
const byte MOTOR_A_FORWARD_CHANNEL = 1;
const byte MOTOR_A_REVERSE_CHANNEL = 2;

const byte MOTOR_B_PWM_CHANNEL = 4;
const byte MOTOR_B_FORWARD_CHANNEL = 5;
const byte MOTOR_B_REVERSE_CHANNEL = 6;

const byte MOTOR_STANDBY_CHANNEL = 7;

void setup() {
  pwm.begin();
  pwm.setPWMFreq(1600);
  
  startLog(); // starts Serial, and prints a message if DEBUG is enabled
  
  if (!cc3000.begin())
  {
    while(1);
  }
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    while(1);
  }
  while (!cc3000.checkDHCP())
  {
    delay(100);
  }
  
  while (! displayConnectionDetails()) {
    delay(1000);
  }
  
  restServer.begin();
}

void loop() {
  stateMachine.update();
  
  // Handle REST calls
  Adafruit_CC3000_ClientRef client = restServer.available();
  rest.handle(client);
}

// ******************* FSM state callback methods *******************
// -------------- Startup State ---------------
void enterStartupState() {
  info(F("Booting up..."));
}
void updateStartupState() {
  // set up the REST listener
  rest.function("tb", setControlState);
  
  // get the REST service registered
  rest.set_id("001");
  rest.set_name("tankbot");

  stateMachine.transitionTo(idleState);
}
void leaveStartupState() {
  info(F("Boot Complete"));
}

// -------------- Idle State ---------------
void enterIdleState() {
  allStop();
  info(F("Idling, ready for commands..."));
}
void updateIdleState() {}
void leaveIdleState() {}

// -------------- Forward State ---------------

// Move the bot forward at full speed
void enterForwardState() {
  allStart();
  allSpeedAhead();
}
void updateForwardState() {}
void leaveForwardState() {}

// -------------- Backward State ---------------

// Move the bot backward at full speed
void enterBackwardState() {
  allStart();
  allSpeedReverse();
}
void updateBackwardState() {}
void leaveBackwardState() {}


// ******************* HELPERS *******************

// aREST API listener
int setControlState(String command) {
  info(F("Handling a REST request"));
  info(command);
  if (command.startsWith("stop")) {
     stateMachine.transitionTo(idleState);
     return 2;
  }
  if (command.startsWith("forward")) {
     stateMachine.transitionTo(forwardState);
     return 3;
  }
  if (command.startsWith("backward")) {
     stateMachine.transitionTo(backwardState);
     return 4;
  }
  return 1; 
}

// ******************* HELPERS *******************

bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}

// -------------- Motor Driver Helpers ---------------
// disable the motors via the motor driver STANDBY
void allStop() {
  warn(F("enabling STANDBY"));
  pwm.setPWM(MOTOR_STANDBY_CHANNEL, 0, 0);
}
// disable the motors via the motor driver STANDBY
void allStart() {
  warn(F("disabling STANDBY"));
  pwm.setPWM(MOTOR_STANDBY_CHANNEL, 0, 4095);
}
void allSpeedAhead() {
  warn(F("moving forward"));
  pwm.setPWM(MOTOR_A_PWM_CHANNEL, 0, 4095); // appromixmately 50% speed
  pwm.setPWM(MOTOR_B_PWM_CHANNEL, 0, 4095); // appromixmately 50% speed
  pwm.setPWM(MOTOR_A_FORWARD_CHANNEL, 0, 4095);
  pwm.setPWM(MOTOR_A_REVERSE_CHANNEL, 0, 0);
  pwm.setPWM(MOTOR_B_FORWARD_CHANNEL, 0, 4095);
  pwm.setPWM(MOTOR_B_REVERSE_CHANNEL, 0, 0);
}
void allSpeedReverse() {
  warn(F("moving backward"));
  pwm.setPWM(MOTOR_A_PWM_CHANNEL, 0, 4095); // appromixmately 50% speed
  pwm.setPWM(MOTOR_B_PWM_CHANNEL, 0, 4095); // appromixmately 50% speed
  pwm.setPWM(MOTOR_A_FORWARD_CHANNEL, 0, 0);
  pwm.setPWM(MOTOR_A_REVERSE_CHANNEL, 0, 4095);
  pwm.setPWM(MOTOR_B_FORWARD_CHANNEL, 0, 0);
  pwm.setPWM(MOTOR_B_REVERSE_CHANNEL, 0, 4095);
}

