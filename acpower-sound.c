#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <uchar.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <linux/limits.h>
#include <sys/wait.h>

// ##### EXPLANATION #####
// This program will read ACPI status file of AC power supply to see if a change in
// state (whether the AC was plugged or unplugged) happened and play a sound accordingly
//
// This solution uses an infinite loop to read a file every second (or another custom set delay time)
// to detect state changes and play a set sound, the 'sleep' function is used carefully since it's not
// resource free, and to keep things simple, a shell program is used for playing sound
//
// Customize the defines to your needs

// ##### PROGRAM DEFINES #####

// Main program runs in a while loop, it's highly inefficient to keep reading a file that barely ever changes, save some CPU usage by waiting a little between each loop
#define SLEEP_TIME 1

// ##### ACPI STUFF #####

// Change this path according to you system - ACPI is a mess and location of these files vary a lot
const char *AC_PATH = "/sys/class/power_supply/AC0";
// This is the file that stores the status of the AC, the "online" provives a 0 or a 1
const char *AC_STATUS_FILE = "online";

// ##### SOUNDS #####

// Calling shell program saves me from a headache, change this if needed
// NOTE: Any output to stdout from the player will be suppressed
#define SHELL_PLAYER "ffplay" // or aplay, mpv, ffplay, etc...
// Additional arguments for the player
#define PLAYER_ARGS "-v", "0", "-nodisp", "-autoexit"

const char *SOUNDS_PATH = "/usr/share/sounds/freedesktop/stereo";
const char *SOUND_PLUG = "power-plug.oga";
const char *SOUND_UNPLUG = "power-unplug.oga";

// ##### FLAGS #####

unsigned int flags = 0;
const unsigned int flag_verbose	= 	(1<<0);	// 1
const unsigned int flag_lite	=	(1<<1);	// 2
const unsigned int flag_mute	=	(1<<2);	// 4

// ##### ENUM #####

typedef enum Status{
	UNPLUGGED,
	PLUGGED,
}ACStatus;

typedef struct{
	// Location to where the AC power supply status file is located at
	char status_file[PATH_MAX];
	// Holds the current status of the AC power supply status
	ACStatus status;
}ACPI;

// ##### HELPER FUNCTIONS #####

#define nameof(var) #var

static inline void print_help(bool unknown, char *arg)
{
	printf("usage: acpower-sound [-h] [-l] [-v] [-m]\n\n");
	if (unknown){
		printf("acpower-sound: error: Unknown arguments: %s\n", arg);
	} else {
		printf("Reads AC status and plays sound depending of state changes.\n\n");
		printf("option:\n");
		printf("\t-h\tShow this help message and exit.\n");
		printf("\t-l\tEnables lite mode - run once and play sound depending of current AC state.\n");
		printf("\t-m\tMute - skip playing audio when the state changes.\n");
		printf("\t-v\tEnable verbose output - prints current state every loop.\n");
	}
}

// Returns false if program should exit, true if should continue to main loop
static inline bool set_flags(int argc, char **argv){
    for (size_t i = 1; i < argc; i++){
		// If argument found
		if (argv[i][0] == '-')
			switch(argv[i][1]){
				case 'v':
					flags |= flag_verbose;
					break;
				case 'l':
					flags |= flag_lite;
					break;
				case 'm':
					flags |= flag_mute;
					break;
				case 'h':
					print_help(0, NULL);
					return false;
					break;
				default:
					print_help(1, argv[i]);
					return false;
					break;
			}
	}
	return true;
}

static inline bool is_flag_set(unsigned int flag){
	return (flags & flag);
}

// Open file and only read the first character (which is the only one relevant, and present)
static inline int read_status_file(char *status_file, unsigned int *out){
	FILE *file = fopen(status_file, "r");
	if (file == NULL) {
		perror("Error opening AC status file");
		return errno;
	}

	int ch = fgetc(file);
	fclose(file);
	if (ch == EOF) perror("Missing status!");
	else *out = ch - '0'; // Convert char to int by subtracting ascii value of 0

	return 0;
}

// Prints to stdout only if the verbose flag is set
static inline int verbose_printf(const char *format, ...){
	if(!is_flag_set(flag_verbose))
		return 0;
	size_t len = 0;
	va_list argv;
	va_start(argv, format);

	// Loop through the string
	for (size_t i = 0; format[i] != '\0'; i++)
	{
		if (format[i] != '%'){
			putchar(format[i]);
			len++;
		} else {
			i++;
			switch (format[i]){
				case 's':
				{
					// Get the elipsis arguments
					char *str = va_arg(argv, typeof(str));
					int j;
					for (j = 0; str[j] != '\0'; j++){
						putchar(str[j]);
						len++;
					}
					break;
				}
				/*case 'd': // I don't really need this
					char *str;
					str = va_arg(argv, char *) + '0';
					int j;
					for (j = 0; str[j] != '\0'; j++){
						putchar(str[j]);
						len++;
					}
					break;
					*/
				default:
					break;
			}
		}
	}
	va_end(argv);
	return len;
}

static inline void play_sound(const char *filename){
	pid_t pid = fork();
	if (pid < 0) {
		// Fork failed
		perror("Unable to fork process");
		return;
	}
	
	if (pid == 0) {
		// In child process

		// Suppress stdout/stderr:
		freopen("/dev/null", "w", stdout);
		//freopen("/dev/null", "w", stderr);

		char *const args[] = {
			SHELL_PLAYER,
			PLAYER_ARGS,
			(char *)filename,
			NULL
		};
		execvp(SHELL_PLAYER, args);

		// If exec fails:
		perror("Unable to execute shell player");
		exit(EXIT_FAILURE);
	} else {
		// parent: wait for child to finish
		waitpid(pid, NULL, 0);
	}
}

// ##### MAIN #####
int main(int argc, char **argv){
	// ARGUMENTS
	if(!set_flags(argc, argv))
		return 0;

	// PROGRAM
	ACPI acpi;
	acpi.status = UNPLUGGED;
	// This holds the status number in integer, gets updated only when there's a change in status
	ACStatus acpi_status_previous = PLUGGED;

	// CONVERSIONS
	// Convert the AC define paths into a single string
	snprintf(acpi.status_file, sizeof(acpi.status_file), "%s/%s", AC_PATH, AC_STATUS_FILE);

	// Convert the sounds define into a single string

	// Get current AC status before starting loop
	errno = read_status_file(acpi.status_file, &acpi_status_previous);
	if (errno != 0) return errno;

	// If verbose; print info, otherwise get to looping
	verbose_printf("PROGRAM STARTING WITH VERBOSE FLAG SET\n");
	verbose_printf("%s:\t%s\n", nameof(acpi.status_file), acpi.status_file);
	verbose_printf("sound_plug:\t%s/%s\n", SOUNDS_PATH, SOUND_PLUG);
	verbose_printf("sound_unplug:\t%s/%s\n", SOUNDS_PATH, SOUND_PLUG);
	// Verbosing flags
	if (is_flag_set(flag_lite))
		verbose_printf("Lite mode is enabled\n");
	if (is_flag_set(flag_mute))
		verbose_printf("Mute flag set, playing audio will be skipped\n");
	verbose_printf("\nLOG START:\n");

	// MAIN LOOP
	do {
		// Get current status
		errno = read_status_file(acpi.status_file, &acpi.status);
		if (errno != 0) return errno;

		// If no change happened, wait and go to next loop iteration
		if (!is_flag_set(flag_lite) && acpi_status_previous == acpi.status){
			//verbose_printf("No state change detected\n", argv[0]); // This bombs log files lol
			sleep(SLEEP_TIME);
			continue;
		}

		// If change happened then
		acpi_status_previous = acpi.status;
		// If state changed to plugged
		if(acpi.status == PLUGGED){
		    verbose_printf("AC was plugged\n", argv[0]);
			if(!is_flag_set(flag_mute)){
				char sound_plug[strlen(SOUNDS_PATH) + strlen(SOUND_PLUG) + 2];
				snprintf(sound_plug, sizeof(sound_plug), "%s/%s", SOUNDS_PATH, SOUND_PLUG);
				play_sound(sound_plug);
			}
			continue;
		}
		// If state changed to unplugged - no if needed
		verbose_printf("AC was unplugged\n", argv[0]);
		if(!is_flag_set(flag_mute)){
			char sound_unplug[strlen(SOUNDS_PATH) + strlen(SOUND_UNPLUG) + 2];
			snprintf(sound_unplug, sizeof(sound_unplug), "%s/%s", SOUNDS_PATH, SOUND_UNPLUG);
			play_sound(sound_unplug);
		}
	} while (!is_flag_set(flag_lite));

	return 0;
}
