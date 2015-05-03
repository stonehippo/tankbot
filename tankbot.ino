// put aREST in lightweight mode to reduce program size
#define LIGHTWEIGHT 1
#include <FiniteStateMachine.h>
#include <SPI.h>
#include "Adafruit_BLE_UART.h"
#include <aREST.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "LogHelpers.h"
#include "TimingHelpers.h"

// disable debug output
#define DEBUG true

// FSM states
State startupState = State(enterStartupState,updateStartupState,leaveStartupState);
State idleState = State(enterIdleState,updateIdleState,leaveIdleState);
State forwardState = State(enterForwardState, updateForwardState, leaveForwardState);
State backwardState = State(enterBackwardState, updateBackwardState, leaveBackwardState);

FSM stateMachine = FSM(startupState);  

aREST rest = aREST();

// Pin configs for the Adafruit nrf8001 breakout board
const byte BLE_REQUEST = 10;
const byte BLE_READY = 2;
const byte BLE_RESET = 9;

// Create a Bluetooth LE UART instance to set up the basic connection
Adafruit_BLE_UART BluetoothLESerial = Adafruit_BLE_UART(BLE_REQUEST, BLE_READY, BLE_RESET);

// Global to monitor the state of the nrf8001 Bluetooth board
aci_evt_opcode_t bleLastStatus = ACI_EVT_DISCONNECTED;

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
  BluetoothLESerial.begin();
}

void loop() {
  // What's up, Bluetooth?
  BluetoothLESerial.pollACI();
  //check the status of the Bluetooth LE connection
  aci_evt_opcode_t bleStatus = BluetoothLESerial.getState();
  if (bleStatus != bleLastStatus) {
    if (bleStatus == ACI_EVT_DEVICE_STARTED) {
      info("BLE advertising started");
    }
    if (bleStatus == ACI_EVT_CONNECTED) {
      info("BLE connected");
    }
    if (bleStatus == ACI_EVT_DISCONNECTED) {
      info("BLE disconnected");
    }
    bleLastStatus = bleStatus;
  }
  
  // Time to handle the state machine
  stateMachine.update();
  
  // deal wih any REST calls
  if (bleStatus == ACI_EVT_CONNECTED) {
    rest.handle(BluetoothLESerial);
  }
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

