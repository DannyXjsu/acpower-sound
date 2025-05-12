# ACPower Sound
Plays a sound whenever the AC state changes, when you plug or unplug your laptop, all in a single source file.

## Configuration
Change the necessary defines in the source file.

## Lite
The \*-lite source file is for testing, it runs once and plays a sound depending of your current AC state instead of watching for state changes.

## Performance
The performance is okay, I first made a shell script doing the exact same thing this program does, since this is for a laptop, it has to be as
efficient as possible, and I know a bash script is not that performant, so I wrote this program, pretty much a direct translation ignoring the
helper functions and similar. I've used 'time' to compare more or less how long it takes to run the lite version of this program and the lite
version of the script, they have pretty much the same time, so either this means this program is terrible optimized (which is very likely) or
the bottleneck really is the I/O.
