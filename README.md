Combine two separate Teensy 3.1 applications and jump between them

This simple example shows an application with a fast-blinking LED jumping to an application with a slow-blinking LED.

This technique can also be used to compile an Arduino application that is launched by a bootloader, e.g. [uTasker](http://www.utasker.com/kinetis.html).

## Instructions

### Setup

* Install Teensyduino (minimum 1.21) onto Arduino (minimum 1.6.1)
* Install srec_cat tool: http://srecord.sourceforge.net/
* Modify boards.txt in the Teensyduino install to give new Linker Script option
    * hardware/teensy/avr/boards.txt
    * Add these lines:  
    menu.linker=Linker Script  
    teensy31.menu.linker.default=Default  
    teensy31.menu.linker.default.build.linkerscript=mk20dx256.ld  
    teensy31.menu.linker.0x8080=0x8080 Offset  
    teensy31.menu.linker.0x8080.build.linkerscript=mk20dx256-8080.ld  
    * Modify one line:
        * find line starting with "teensy31.build.flags.ld"
        * replace "mk20dx256.ld" with "{build.linkerscript}"
        * Full line should be: teensy31.build.flags.ld=-Os -Wl,--gc-sections,--relax,--defsym=__rtc_localtime={extra.time.local} --specs=nano.specs "-T{build.core.path}/{build.linkerscript}"
* Add new linker script mk20dx256-8080.ld into hardware/teensy/avr/cores/teensy3/
* Start/restart Arduino.  Look for new "Linker Script" menu option in Tools menu when using the Teensy 3.1 board type.  Keep this option set to "Default" and your Teensy 3.1 sketches will be linked normally.  Change this option to "0x8080 Offset" and your sketches will be linked to start at offset 0x8080 in Flash (and will not run on the Teensy 3.1 as is).

### Combine two applications

* Load JumpToAppWithOffset sketch and compile with Linker option "Default".
* Navigate to the directory holding the .hex file that was generated (you can find this directory in the Arduino build console).
* Copy JumpToAppWithOffset.cpp.hex to temporary folder (otherwise it will be deleted next time you press "compile")
* Change linker script to 0x8080 offset
* Compile Blink example
* Copy JumpToAppWithOffset.cpp.hex from temporary folder back into Arduino build directory
* From command line in build directory: srec_cat JumpToAppWithOffset.cpp.hex -Intel Blink.cpp.hex -Intel -Output JumpToBlinkWithOffset.hex -Intel
    * This command loads the two hex files, which are in non-overlapping sections of memory, and creates a new .hex file containing both applications.
* Open JumpToBlinkWithOffset.hex in Teensy Loader
* Press button to load to Teensy
* Observe two fast blinks from JumpToAppWithOffset, followed by slower blinks from Blink Example running infinitely






