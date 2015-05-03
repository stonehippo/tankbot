# Design notes

When I first conceived of Tankbot, I was thinking of a a simple little robot that could be controlled via a remote connection from a computer, tablet or smartphone. I thought it would be cool if the bot used a track and wheel system for locomotion, because these systems can handle all sorts of terrain and because it's just cool to have a little robotic tank.

I also wanted to add some autonomous features to tankbot, or at least sensors that I could read to get information about the current state of both the bot and the world around it.

One issue that I've run up against has been the limitation of the microcontroller that I selected to be the brains of Tankbot. For various reasons, I selected the Arduino platform as the basis of the bot. Arduinos are fairly cheap, there's a ton of hardware and software support for them, and I happen to have several versions of the boards around in various form factors.

The core of the original Arduino is an Atmel ATmega microcontroller, specifically the ATmega328 (at least in most modern versions). The ATmega328 is a versatile and pretty capable microcontroller. It is simple enough to be the foundation of a platform for hobby-level work. It is powerful enough to handle lots of tasks. But it does have some constraints.


## Pins, pins, who's got some pins?

The first constraint I ran into is the number of GPIO pins available. Early on I selected the TB6612FNG dual H-bridge as my motor driver. Sparkfun and Adafruit both carry breakout boards for this handy IC – Adafruit even uses it in the heart of version 2 of their nice motor driver shield. So the TB6612FNG seemed like a good choice for driving the two 6V DC hobby motors tied to the gearbox. The downside is that this IC requires a lot of GPIO pins from the Arduino: 1 PWM and two digital pins for each motor control, plus another digital pin for the standby feature.

Most standard Arduino models, such as the Uno, can provide that many pins with no problem; this certainly true of the Arduino Pro is planned to use. This problem is I want to hook other stuff up to the board as well: a communications module for control and some sensors, at least. So every pin I use for the motor driver is one less available for something else.

Fortunately, the answer to the problem occurred to me pretty quickly. There are several different ways to expand the available pins from a microcontroller, such as shift registers or a dedicated PWM IC. If found that Adafruit had a few of these PWM breakous available, including one with 16 channels of PWM, all controlled by I2C. This is great, because I2C is often implemented as a two-wire solution, meaning that if I could integrate the PWM board, it would save me five digital pins. Better yet, I'd have the start of a I2C bus and many sensors are set up to work with this protocol. Not only am I freeing up pins, I'm setting down a path for more options in the future. Yay!

It then occurred to me that Adafruit must have done just that with the motor shield. That shield uses the same chip I as a motor driver, and it's controlled by I2C. So I took a look at the circuit diagrams for the shild and yep, they integrated the same PWM chip on that board.

At this point, you might wonder why I didn't just switch over to the motor shield, or its breakboard equivalent. The answer is simple: I wanted to hack something together. Part of the point of building Tankbot is to get my hands dirty and to exercise my design skills. Yes, I'm pulling pre-existing stuff of the shelf, but I'd like to do as much of the integration as possible. At some point, I hope to my custom boards for Tankbot. But for now, it's all about hacking out solutions.

I now have a solution that frees up pins, gives me the basis of an I2C network for connecting lots of stuff the Arduino, and it's a known-to-work solution. Yay!

## A tiny memory issue

The next constraint that I ran afoul of is memory. The ATmega328 is fairly capable, but one thing it doesn't have is a lot of room to work in. One place where the ATmega can get a bit snug is dynamic memory (where the work happens) and program space. The ATMeage328 only has ~30K or so of program space. A bit more can be made available, depending on which bootloader you use or by forgoing a bootloader entirely, but the hardware roof for storing stuff in 32K.

In addition, the ATmega328 on has 2K of dynamic, working memory.

For many things, this is fine. But for Tankbot, we start to run into some issues quickly. The pressure comes largely from the libraries that I'm using to support the functions that makes up the bot. There's a library for the PWM board, a library for the finite state machine that manages the robots functions, a library the communicatoins module.

It's that last one the really seems to be the biggest issue. I've implemented to comm modules, one using a nRF8001 Bluetooth LE board and the other using the CC3000 Wifi module. And both of these come with large libraries, relative to the capacities in the ATmega. I found that the nRF8001 library left decent space in program storage, but it eats up a lot of dyanmic memory. The CC3000 library is lightly better on dynamic memeory use but take a big chunk of the storage space.

I don't have a solution for that, but there are several options I could try (this isn't an exhaustive list):

* Try a different module with smaller library footprint
* Move up tp a more capable Arduino model, such as the Mega
* Split up the functions of the bot and handle them with multiple microcontrollers

I'm learning towards that last solution. I have a lot of Arduinos sitting around – let's just say that I've taken advantage of the Sparkfun Arduino Day sale for the past couple of years – so I'm thinkin that what I might do is make one the communications controller, letting it handle the wireless communication, and it can prompt it's sibling controller via a serial interface. I could also use other boards: I've got a few ESP8266 wifi modules around that would be good candidate, I think.

### An aside

I just took a look at the offical [Arduno Robot](http://www.arduino.cc/en/Main/Robot), and it seems they take the multiple MCU approach as well. In that design, both the Control and Motor boards have their own ATmega32u4 MCUs. The board communicate with each other via serial, which is pretty much how I'd like to do it, too. If I stick with the nRF8001 or CC3000 boards for communications, they'll use SPI on the control MCU, leaving the hardware serial available to talk to the motor MCU. Assuming that I can keep the communication protocol between to two halves of the Tankbot brain fairly lightweight, the design gets more capacity for sensors and actuators, too.
