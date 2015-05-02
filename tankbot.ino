#include <FiniteStateMachine.h>
#include <SPI.h>
#include <aREST.h>
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

void setup() {
  startLog(); // starts Serial, and prints a message if DEBUG is enabled
}

void loop() {
  stateMachine.update();
  rest.handle(Serial);
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
  info(F("Idling, ready for commands..."));
  allStop();
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
void leaveForwardState() {
  allStop();
}

// -------------- Backward State ---------------

// Move the bot backward at full speed
void enterBackwardState() {
  allStart();
  allSpeedReverse();
}
void updateBackwardState() {}
void leaveBackwardState() {
  allStop();
}


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
}
// disable the motors via the motor driver STANDBY
void allStart() {
  warn(F("disabling STANDBY"));
}
void allSpeedAhead() {
  warn(F("moving forward"));
}
void allSpeedReverse() {
  warn(F("moving backward"));
}

