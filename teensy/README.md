# Source code for Teensy 4.1 microcontroller. 
This is inspired by various other works which I have done and seen, but
is a ground-up implementation. 

# Structure
One main file, `rollercoasterometer.ino`. I am considering a bunch of header-only
classes to handle the other stuff. Each would be in its own `.h` file, but
there would not be any accompanying `.cpp` files.

We will bring the parts up one-at-a-time:

1. Bring up to Blink with CLI
2. USB Serial with Putty
3. TeensyView
4. Sensors
   1. LSM9DS1
   2. ICM-20948
   3. BME680
   4. ICM-42688 (when it arrives)
   5. AK09916 (when it arrives)
   6. BMM384 (when it arrives)
   7. HiMag (when it arrives)
5. PPS
6. GPS
7. MicroSD
8. Packets
9. User interface

# Bring up with CLI
The reason I want to use the command-line method for compiling, rather than
the Arduino IDE, is that I want to customize the Makefile. I want to attach
the source code for the project to the uploaded image.

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

I copied the program `arduino-1.8.19/hardware/teensy/avr/cores/teensy4/Blink.cc` into this project as
`blink.cpp`.

The problem with the Makefile is that it is intended to compile in-place. I
don't want to copy all the code that came with Teensy into this repository,
and I don't want to build in the Teensy folder. 

Everything on the host side seemed to be working, and yet the programmer doesn't program :( I'm going to hang this up
for a while and go on to the next thing.

# USB Serial
In Arduino, this is pretty straightforward. Once it's done there, Putty picks
up on it fine too.

# Sensors
I re-arranged
the base classes to use CRTP, so that I could use compile-time polymorphism
to decide whether a sensor is big- or little- ended. At some point I will
probably add other data-types, and maybe break out the little- and big-end
stuff so that packets can use it too.

* `I2C_sensor`
  * `I2C_sensor_be`
     * `ICM_20948`
  * `I2C_sensor_le`

In this case, we have `I2C_sensor`, which is a template class that takes its 
derived class as its template argument. The derived class supplies the conversion
between byte buffers and larger values, converting between the sensor and host
endian-ness. Since the types are all known at compile time, and since this is a
header-only library, my hope is that the compiler sees through all the templates,
function calls, etc and optimizes it all into clean inline code.
  
I want to have one function called `read()` and one called `write()` which
get data back and forth from the sensor, but normal operator overloading doesn't
work, because `read()` is only distinguished by return type, and you can't overload
on that. So, we use a template member function inside of a template class. Many 
calls to `read()` or `write()` will need to have a type specified as a template
parameter, but that's ok, since "Explicit Is Better Than Implicit". We might
have 17 functions to work on 17 different types, but they are all called `read()`
and they all get pulled in with `using I2C_sensor::read;`

Unfortunately, while template classes with template member functions are possible,
they aren't clean. You can't partially specialize the template function. So, we end
up with code structured like this:

```c++
template<typename T>
class I2C_sensor {
public:
  void read(uint8_t reg_addr, uint8_t buf[], size_t len) {/* Actual I2C read transaction of len bytes */} 
  uint8_t read(uint8_t reg_addr) {/* Actual I2C read transaction special case for 1 byte */} 
  void write(uint8_t reg_addr, uint8_t buf[], size_t len) {/* Actual I2C write transaction of len bytes */} 
  write read(uint8_t reg_addr, uint8_t data) {/* Actual I2C write transaction special case for 1 byte */} 
  template<typename U> U    get(...); //Get a value of type U from the buffer
  template<typename U> void put(...); //Put a value of type U into the buffer
  template<typename U> U   read(...) {/* with inline implementation, using get<U>() */}
  template<typename U> U  write(...) {/* with inline implementation, using put<U>() */}
  /* ... */
}

class I2C_sensor_be:public I2C_sensor<I2C_sensor_be> {
public:
  using I2C_sensor::read;
  using I2C_sensor::write;
  /* ... */
}

/* Full specialization, not for I2C_sensor_be, but for I2C_sensor with
   I2C_sensor_be specified as first parameter. So, this is actually
   code which will be compiled as part of the base class, but is logically
   part of the derived class, and therefore listed here. */
template<> template<> 
inline uint16_t I2C_sensor<I2C_sensor_be>::get(uint8_t buf[]) { /* big-endian extract from buffer */ };
/* There could be another one like this for every unsigned type, including uint8_t */

/* Now we do all the *signed* stuff, where we assume that casting between signed
  and unsigned is a good thing to do. It assumes that the sign convention is the
  same between the host and device. Since all modern electronics use twos complement,
  this is going to be valid. */
template<> template<> 
inline int16_t I2C_sensor<I2C_sensor_be>::get(uint8_t buf[]) {return int16_t(get<uint16_t>(buf)); /* use unsigned get, then cast to signed */};
/* Again, one like this for each signed type, for which there must be an unsigned type.

/* And now the ugly part -- we get to repeat all this boilerplate for little-endian  
   The only difference is in the unsigned get and put for multi-byte values. */
   
class I2C_sensor_le:public I2C_sensor<I2C_sensor_le> {
public:
  using I2C_sensor::read;
  using I2C_sensor::write;
  /* ... */
};

template<> template<> inline uint16_t I2C_sensor<I2C_sensor_be>::get(uint8_t buf[]) { /* ... */ }
template<> template<> inline  int16_t I2C_sensor<I2C_sensor_be>::get(uint8_t buf[]) {return int16_t(get<uint16_t>(buf));}

```

So far my optimization hope seems to be working. It's ugly code to look at, but
the zen of C++ is to hide the ugliness in the library, as long as it is easy to
call.

## ICM20948
The code I had previously written for the Arduino RedBoard works fine. 

## ICM42688
I got 10 of them, and attached one to a breakout board, christening my toaster oven in the process.
Visual inspection revealed it to be OK, but it isn't consistently acting like I hoped. For a while,
the RedBoard was able to read it, but only when I was using the ADC to probe the voltages. Probing with
an ADC like a voltmeter shouldn't make a difference, but it did. Eventually it stopped responding altogether.
I am going to put this a side for a while, and hope that it's just this one part.

I wonder if it's a bad solder joint on the pins connecting to the breadboard?

# SD card
The Teensy library Teensyduino, comes with an SD card library. I was fighting with it for a while,
where I could get it to create files but not to actually put any data into them. It turns out you
need to call the `flush()` method on the file object. There is a `sync()` method too, but I haven't
tried that.

With all the USB and TeensyView stuff turned off, I can get about 2500 packets per second from the
inertial sensor to the card.

The SD card library that comes with the Teensy supports ExFAT, so we will use it. There is a callback
that can be used when the system
needs to know what time it is, so I can hook in the GPS stuff.

# PPS
The General Purpose Timer (GPT) is the correct solution for high-precision measurement of the PPS signal.
It supports:

* Capture input
* 32-bit resolution
* 60MHz count rate (to match the old Loginator). The clock driving the timer has a fixed divide-by-4 from
   the core clock, so in order to get 60MHz, we must have some multiple of 240MHz as the main frequency. 600MHz isn't it, but
   either 480MHz (underclock) or 720MHz (overclock) is fine.
* Automatic rollover at some value other than 0xFFFFFFFF. We set it to roll over after 60s, that is 3,600,000,000 counts.

There are other timers which show promise, in particular the quad-timer. This is 4x 16-bit timers which
can be daisy-chained into a 64-bit timer, but cannot be used for capture.

There are two GPT modules, and each module has two capture channels. Unfortunately, GPT1 does not have
its capture inputs wired to any externally visible pins. The two capture channels for GPT2 can be wired
to externally visible pins, D15 for channel 1 and D41/A17 for channel 2.

In order to get the timer working, the following things need to be set up:

* Set the clock speed to 480MHz, in the Arduino Tools/CPU Speed menu. This will set up the peripheral clock to run at 120MHz
* Enable clock gating to GPT2
* Set the I/O muxes to route the capture input correctly, with proper hysteresis, pull/keep, daisy-chaining, etc.
* Set the control register in the GPT2, with proper prescale, compare value, reset on compare, capture on the correct channel, etc.
* DO NOT use `pinMode()` after everything is set up. I'm not sure exactly what this does, but it probably involves taking the pin back for GPIO and therefore taking it away from the GPT. I spent days chasing this.

All of this was done in a separate program called EchoBoth. It was later pulled into the main Rollercoasterometer sketch.
