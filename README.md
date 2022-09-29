# Raspberry pi led driver

Led driver for raspberry pi 1 b

## Motivation
These are the examples presented on the youtube's video 
"Understanding the Structure of a Linux Kernel Device Driver"
https://www.youtube.com/watch?v=pIUTaMKq0Xc working on a raspberry pi.

## Requirements
 - gcc compiler running on raspberry pi
 - The appropiate linux headers according to your kernel version
 
 ## How to use it
 - open the console 
 - run make
 - use sudo insmod ./file_name.ko to load the module
 - use sudo rmmod file_name to unload the module
 - To see what the driver can do, refer to the video mentioned earlier.
 
 
## Note
These examples make use of the GPIO22 pin on
a raspberry pi 1 model b.
