#include <FiniteStateMachine.h>
#include "LogHelpers.h"
#include "TimingHelpers.h"

// disable debug output
#define DEBUG true

// FSM states
State startupState = State(enterStartupState,updateStartupState,leaveStartupState);
State idleState = State(enterIdleState,updateIdleState,leaveIdleState);

FSM stateMachine = FSM(startupState);

void setup() {
  startLog();
}

void loop() {
  stateMachine.update();
}

// ******************* FSM state callback methods *******************
// -------------- Startup State ---------------
void enterStartupState() {
  info(F("Booting up..."));
}
void updateStartupState() {
  stateMachine.transitionTo(idleState);
}
void leaveStartupState() {
  info(F("Boot Complete"));
}

// -------------- Idle State ---------------
void enterIdleState() {
  info(F("Idling, ready for commands..."));
  digitalWrite(13, HIGH);
}
void updateIdleState() {

}
void leaveIdleState() {

}

// ******************* HELPERS *******************
