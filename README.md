# MP3 Player Project 

As a part of Real-Time Embedded System Co-Design course at SJSU we developed an MP3 player using  **Adafruit standard LCD 16x2, VS1053b MP3 decoder, SJ2 board**

## Design
- Only two tasks to play the music and read the SD card
- CPU utilization of 12% 
- SPI high-speed, full-duplex bus driver 
- GPIO data bus - driver 
- LCD driver
- MP3 decoder driver 
- SPI used between mp3 decoder and SJ2 board 
- GPIO 8-bit data transfer used for LCD display 

## Functionality 

- User friendly LCD display with playlist and settings
- Ability to play, pause, forward, or rewind to diff song
- Ability to change bass, trebble, and volume
- Ability to connect to external speakers or headphones

# SJ2-C Development Environment

An `SJ2` board is used at San Jose State University (SJSU) to teach Embedded Systems' courses.  It was designed to **learn the low level workings of a microcontroller platform with an RTOS**:

Project highlights:

- Fully implemented in C
- Minimalistic design with little to no abstractions
- Follows good coding principles (such as YAGNI and DRY)

Infrastructure highlights:

- Supports Mac, Linux, Windows out of the box
- Version controlled toolchains and other supplementary tools
- No VMs
- No WSL dependency on Windows

----

