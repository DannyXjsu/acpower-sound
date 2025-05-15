# ACPower Sound
This program will read ACPI status file of AC power supply to see if a change in 
state (whether the AC was plugged or unplugged) happened and play a sound accordingly, all in a single source file.

This solution uses an infinite loop to read a file every second (or another custom set delay time)
to detect state changes and play a set sound, the 'sleep' function is used carefully since it's not
resource free, and to keep things simple, a shell program is used for playing sound

## Usage
Check the help message by using parameter '-h'
```
usage: acpower-sound [-h] [-l] [-v]

Reads AC status and play sound depending of state changes.

option:
	-h	Show this help message and exit.
	-l	Enables lite mode - run once and play sound depending of current AC state.
	-v	Enable verbose output - prints current state every loop.
```

## Configuration
Change the necessary defines in the source file.

## Performance
The performance is okay, I first made a shell script doing the exact same thing this program does, since this is for a laptop, it has to be as
efficient as possible, and I know a bash script is not that performant, so I wrote this program, pretty much a direct translation ignoring the
helper functions and similar.

I've used 'time' to compare more or less how long it takes to run the lite version of this program and the lite
version of the script, they have pretty much the same time, so either this means this program is terribly optimized (which is very likely) or
the bottleneck really is the I/O.
