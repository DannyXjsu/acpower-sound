# Depracated
This program has evolved into [Battery Watcher](https://github.com/DannyXjsu/battery-watcher).

It works similarly like this program, but it does not play sounds, it allows to run scripts instead.
Giving more freedom to do whatever you want depending on state changes.

# ACPower Sound
This program will read ACPI status file of AC power supply to see if a change in
state (whether the AC was plugged or unplugged) happened and play a sound accordingly, all in a single source file.

This solution uses an infinite loop to read a file every second (or another custom set delay time)
to detect state changes and play a set sound, the 'sleep' function is used carefully since it's not
resource free, and to keep things simple, a shell program is used for playing sound.


## Usage
Check the help message by using parameter '-h'

```
usage: acpower-sound [-h] [-l] [-v] [-m]

Reads AC status and plays sound depending of state changes.

option:
	-h	Show this help message and exit.
	-l	Enables lite mode - run once and play sound depending of current AC state.
	-m	Mute - skip playing audio when the state changes.
	-v	Enable verbose output - prints current state every loop.
```

### Autostart

I recommend creating a systemd user service so it starts everytime you login, there is an example systemd service file.

```bash
# Copy/move this file to user data folder
$ cp acpi-sound.service $XDG_CONFIG_HOME/systemd/user/

# Reload daemon (updates systemd stuff)
$ systemctl --user daemon-reload

# Enable it 
$ systemctl --user enable --now acpi-sound
```


## Configuration
Change the necessary defines in the source file.


## Performance
You can use the lite mode with the mute option to measure the time it takes the program to run, I think it's about as fast and as light on resources as I can make.
