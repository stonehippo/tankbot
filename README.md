# Tankbot

Tankbot is an simple mobile robotics platform for experimentation and hacking. It's meant to be a small, lightweight and inexpensive setup.

## Hardware

The initial prototype hardware for Tankbot is based on cheap, off-the-shelf components. The basic chassis is assembled around Tamiya parts, with twin-motor gearbox mated to a universal plate with a plastic track and wheel set.

Tankbot gets its brain from the venerable Arduino. In this case, I'm starting off with an Arduino Pro Mini running at 3.3v and 8MHz. I decided to use the Arduino, rather than a dedicated robotics motion controller, as I've got several sitting around waiting to be integrated into something.

The gearbox motors are driven by a Sparkfun TB6612FNG breakout board. I've also dropped in an Adafruit 16-channel 12-bit PWM board; this lets me control the motor driver via I2C and gives me additional options for mounting servos or other devices to Tankbot as things develop. And it also reduces the pin usage on the Arduino.

# Firmware

The firmware for Tankbot is a work in progress, and there's still significant work to do. At the core of the design is one of my favorite patterns for Arduino coding, a finite state machine. Having a FSM makes it much easier to writing minimally blocking code and lets me organize thing more neatly. When paired with a simpler timer implementation, having the FSM around is great.
