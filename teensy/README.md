# Source code for Teensy 4.1 microcontroller. 
This is inspired by various other works which I have done and seen, but
is a ground-up implementation. We will bring the parts up one-at-a-time:

1. Bring up to Blink with CLI
1. USB Serial with Putty
1. TeensyView
1. Sensors
   1. LSM9DS1
   1. ICM-20948
   1. BME680
   1. ICM-42688 (when it arrives)
   1. AK09916 (when it arrives)
   1. ...
1. PPS
1. GPS
1. MicroSD
1. User interface

# Bring up with CLI
I included the PJRC repo as a submodule:

```shell
cd rocketometer/teensy
git submodule add https://github.com/PaulStoffregen/teensy_loader_cli.git
```

The README.md for that project said that Ubuntu users would have to install a package:

```shell
sudo apt install libusb-dev
```

The `Makefile` was already set up for Linux, so the compile worked.

The command line tools do not include the compiler. The documented way for doing this
is to install Teensyduino first, then grab the Makefile from it and snip out certain parts.

I got a fresh install of Arduino IDE 1.8.19, and unpacked it in my home folder. I then ran
the `TeensyduinoInstall.linux64` installer, pointed at the new Arduino install. The `Makefile` is
in `arduino-1.8.19/hardware/teensy/avr/cores/teensy4/`. I had to uncomment the Teensy 4.1 stuff and
comment out the Teensy 4.0 stuff in the header for the compiler to work. Once that was done,
I ran `make` in that folder to test it. The compiler built all the code, then Teensyduino fired up
and uploaded the code to the Teensy.

I duplicated the Makefile to this project, and pointed it at the Arduino installation:

```makefile
ARDUINOPATH ?= ../../../arduino-1.8.19/
```

I copied the linker script `arduino-1.8.19/hardware/teensy/avr/cores/teensy4/imxrt1062_t41.ld`
into this project. I haven't made any changes to it yet, but I reserve the right to do so.

I copied the program `arduino-1.8.19/hardware/teensy/avr/cores/teensy4/Blink.cc` into this project



