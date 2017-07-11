// **** FSM event handlers ****

void enterStartupState();
void updateStartupState();
void leaveStartupState();

void enterIdleState();
void updateIdleState();
void leaveIdleState();

void enterForwardState();
void updateForwardState();
void leaveForwardState();

void enterReverseState();
void updateReverseState();
void leaveReverseState();

// **** aREST listener ****

int setControlState(String command);

// **** Motor driver helpers ****

void allStop();
void allStart();
void allSpeedAhead();
void allSpeedReverse();