# What is this?

In short, this repository contains the source code, Ableton remote script and hardware design for the Pulsar Heavy Industries Cenx4 controller.

![Controller picture](http://rundste.in/assets/diy-music-machines-ableton-live-controller-update/front.jpg)

The main features are (some of which are already fully implemented):
- Easy to use: Automatic mapping of selected device's parameters (as seen in the image below). Getting this to work requires almost no setup and no manual mapping in Ableton (just selecting the right remote script).
- Modular: The thing you see is actually two modules connected togeter using [CAN bus](https://en.wikipedia.org/wiki/CAN_bus).
- Updateable: Support firmware update over MIDI and over CAN. The master unit takes care of updating the slaves automatically.
- Open source: Can be used as a basis for many other controllers.

I am still in the process in documenting everything, but please refer to the following posts for more information:

[DIY Music Machines - Introduction](http://rundste.in/2016/12/08/diy-music-machines-intro.html)

[DIY Music Machines - Ableton Live Controller Update](http://rundste.in/2017/03/05/diy-music-machines-ableton-live-controller-update.html)

# Basic steps in getting this to run

## Hardware
 - You will need the following:
   - A JTAG adapter compatible with openocd/gdb. I use an [STM32F4DISCOVERY](http://www.st.com/en/evaluation-tools/stm32f4discovery.html) for this
   - A [Tag-Connect TC2050-IDC](http://www.tag-connect.com/TC2050-IDC) cable
   - A PHI Cenx4 board (TODO: create+link to board page)
- The pinout of the Debug cable is as following (see page 2 [here](http://www.tag-connect.com/Materials/TC2050-IDC%20Datasheet.pdf) for where pin 1 is on the flat cable):
   - VCC: Pin 1
   - GND: Pin 2
   - NRST: Pin 6
   - UART_RX: Pin 7 (currently unused)
   - UART_TX: Pin 8 (currently unused)
   - SWCLK: Pin 9
   - SWDIO: Pin 10
 - If using an [STM32F4DISCOVERY](http://www.st.com/en/evaluation-tools/stm32f4discovery.html) board as JTAG, remove the jumpers on CN3 (labeled "ST-LINK"), and connect the following to CN2 (labeled "SWD" - pin 1 marked with a dot):
   - 1: VCC (Pin 1 on TagConnect flat cable)
   - 2: SWCLK (Pin 9 on TagConnect flat cable)
   - 3: GND (Pin 2 on TagConnect flat cable))
   - 4: SWDIO (Pin 10 on TagConnect flat cable)
   - 5: NRST (Pin 6 on TagConnect flat cable)
   - 6: Unused
 - You will need at least one Cenx4 in Master configuration - this means CAN terminator jumpers installed (labeled CAN_TERM) and a USB connector soldered on 

## Software
- Download and extract [ChibiOS 16.1.7](https://sourceforge.net/projects/chibios/files/ChibiOS_RT%20stable/Version%2016.1.7/ChibiOS_16.1.7.zip/download) into `vendor/`
- Download and extract [uGfx 2.7](https://community.ugfx.io/files/file/2-%C2%B5gfx-library/?do=download&csrfKey=8ae07e04e710333d9b25ed4ae016a4ba) info `vendor/`
- Make you you have a working [ARM toolchain](https://launchpad.net/gcc-arm-embedded) in your path
- Build the bootloader by running `make` inside `cenx4/firmware/bl`
- Start openocd by running `openocd -f stm32f4discovery-chibios.cfg` inside the `openocd` directory
- Use GDB to download the bootloader:
    ```
    % arm-none-eabi-gdb build/ch.elf
    GNU gdb (GNU Tools for ARM Embedded Processors) 7.10.1.20160616-cvs
    Copyright (C) 2015 Free Software Foundation, Inc.
    License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
    ...
    Reading symbols from build/ch.elf...done.
    (gdb) target remote 127.0.0.1:3333
    Remote debugging using 127.0.0.1:3333
    0x00000000 in ?? ()
    (gdb) monitor reset init
    Unable to match requested speed 1000 kHz, using 950 kHz
    Unable to match requested speed 1000 kHz, using 950 kHz
    adapter speed: 950 kHz
    target state: halted
    target halted due to debug-request, current mode: Thread
    xPSR: 0x01000000 pc: 0x080001a0 msp: 0x20000400
    Unable to match requested speed 8000 kHz, using 4000 kHz
    Unable to match requested speed 8000 kHz, using 4000 kHz
    adapter speed: 4000 kHz
    (gdb) load
    Loading section startup, size 0x1a0 lma 0x8000000
    Loading section .text, size 0x9d84 lma 0x80001a0
    Loading section .data, size 0x4bc lma 0x8009f28
    Start address 0x80001a0, load size 41952
    Transfer rate: 22 KB/sec, 8390 bytes/write.
    (gdb) continue
    Continuing.
    ```
 - The bootloader should now be running.
 - TODO: Instructions on how to use it to load the firmware
 - Build the application by running `make` inside `cenx4/firmware/app`
 - Once again use gdb, similar to the bootloader but use the ELF file `build/fw.elf` (unlike `build/ch.elf`, `fw.elf` is in the format the bootloader expects, so your application would run even when not using the debugger)
 - Copy `cenx4/ableton/Cenx4` into Ableton's Remote Scripts directory. On Mac, this would be `/Applications/Ableton Live 9 Suite.app/Contents/App-Resources/MIDI Remote Scripts`.
 - Start Ableton, and configure the MIDI devide. Make sure to use the Cenx4 remote script.
 
