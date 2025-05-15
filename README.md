# ACPower Sound
This program will read ACPI status file of AC power supply to see if a change in
state (whether the AC was plugged or unplugged) happened and play a sound accordingly, all in a single source file.

This solution uses an infinite loop to read a file every second (or another custom set delay time)
to detect state changes and play a set sound, the 'sleep' function is used carefully since it's not
resource free, and to keep things simple, a shell program is used for playing sound

## Usage
Check the help message by using parameter '-h'
```txt
usage: acpower-sound [-h] [-l] [-v] [-m]

Reads AC status and plays sound depending of state changes.

option:
	-h	Show this help message and exit.
	-l	Enables lite mode - run once and play sound depending of current AC state.
	-m	Mute - skip playing audio when the state changes.
	-v	Enable verbose output - prints current state every loop.
```

## Configuration
Change the necessary defines in the source file.

## Performance
You can use the lite mode with the mute option to measure the time it takes the program to run, I think it's about as fast, and as light on resources as I can make.
