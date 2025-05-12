#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <linux/limits.h>

// ##### EXPLANATION #####
// This program will read ACPI status file of AC power supply to see if a change in 
// state (whether the AC was plugged or unplugged) happened and play a sound accordingly
//
// This solution uses an infinite loop to read a file every second (or another custom set delay time)
// to detect state changes and play a set sound, the 'sleep' function is used carefully since it's not
// resource free, and to keep things simple, a shell program is used for playing sound
//
// Customize the defines to your needs


// ##### ACPI STUFF #####
// Change this path according to you system - ACPI is a mess and location of these files vary a lot
#define AC_PATH "/sys/class/power_supply/AC0"
// This is the file that stores the status of the AC, the "online" provives a 0 or a 1
#define AC_STATUS_PATH "online"

#define STATUS_PLUGGED   1
#define STATUS_UNPLUGGED 0

// ##### SOUNDS #####
#define SHELL_PLAYER "pw-play" // Calling shell program saves me from a headache, change this if needed
#define SOUNDS_PATH "/usr/share/sounds/freedesktop/stereo"
#define SOUND_PLUG "power-plug.oga"
#define SOUND_UNPLUG "power-unplug.oga"

// ##### PROGRAM DEFINES #####
// Main program runs in a while loop, it's highly inefficient to keep reading a file that barely ever changes, save some CPU usage by waiting a little between each loop
#define SLEEP_TIME 1
// Change size according to your system, mine uses only a single digit number, so size 3 for the number itself, newline if present, and null terminator
#define AC_STATUS_SIZE 3


// ##### HELPER FUNCTIONS #####
// Opening and reading file
static inline int read_status_file(char *status_file, char out[AC_STATUS_SIZE]){
	FILE *file;
	char *line = NULL;
	size_t len = 0;
	ssize_t read = 0;
	
	file = fopen(status_file, "r");
	if (file == NULL) {
		perror("Error opening file, please change the source code in case system uses a different path for AC power supply status");
		return errno;
	}
	
	// Only a single line needed
	if ((read = getline(&line, &len, file)) != -1){
		snprintf(out, AC_STATUS_SIZE, "%s", line);
	} else {
		perror("Unable to read line");
	}

	free(line);
	fclose(file);
	return 0;
}

// Convert string to integer - return value is reserved for errors
static inline int str2int(char *string, int *output){
	int err = sscanf(string, "%d", output);
	if (err <= 0)
		return err;

	return 0;
}

// Uses the 2 above functions to get the integer value of the AC status - returns are for errors
static inline int get_AC_status(char *path_to_AC_status, int *output){
	// Buffer for reading the status from file
	char s_ACStatus[AC_STATUS_SIZE];
	if (read_status_file(path_to_AC_status, s_ACStatus) != 0)
		return errno;

	// Conversion
	errno = str2int(s_ACStatus, output);
	if (errno != 0) return errno;
	return 0;
}

// ##### MAIN #####
int main(int argc, char **argv){
	// ARGUMENTS
	// Making this program compatible with arguments following GNU standard is a headache and too complex for a single source file, so only verbose flag should be available
	int fFlags = 0;
	const int fVerbose = (1>>0);

	for (int i = 1; i < argc; i++){
		if (argv[i][1] == 'v')
			fFlags |= fVerbose;
	}
	
	// PROGRAM
	// This holds the status number in integer, gets updated only when there's a change in status
	int b_ACStatus = STATUS_UNPLUGGED;
	// Same as above, but this is updated every loop
	int b_ACStatus_real = STATUS_UNPLUGGED;
	
	// CONVERSIONS
	// Convert the AC define paths into a single string
	char AC_file[PATH_MAX];
	snprintf(AC_file, PATH_MAX, "%s/%s", AC_PATH, AC_STATUS_PATH);

	// Convert the sounds define into a single string
	char sound_plug[sizeof(SOUNDS_PATH) + sizeof(SOUND_PLUG) + 2];
	char sound_unplug[sizeof(SOUNDS_PATH) + sizeof(SOUND_UNPLUG) + 2];
	snprintf(sound_plug, sizeof(sound_plug), "%s/%s", SOUNDS_PATH, SOUND_PLUG);
	snprintf(sound_unplug, sizeof(sound_unplug), "%s/%s", SOUNDS_PATH, SOUND_UNPLUG);
	
	// Get current AC status before starting loop
	int err;
	err = get_AC_status(AC_file, &b_ACStatus);
	if (err != 0) return err;

	// If verbose; print info, otherwise get to looping
	if (fFlags & fVerbose){
		printf("PROGRAM STARTING WITH VERBOSE FLAG SET\n");
		printf("AC_file:\t%s\n", AC_file);
		printf("sound_plug:\t%s\n", sound_plug);
		printf("sound_unplug:\t%s\n", sound_unplug);
		printf("\nLOG START:\n");
	}

	while(1){
		// Get current status
		err = get_AC_status(AC_file, &b_ACStatus_real);
		if (err != 0) return err;

		// If no change happened, wait and go to next loop iteration 
		if (b_ACStatus == b_ACStatus_real){
			if(fFlags & fVerbose)
				printf("[LOG] %s: No state change detected\n", argv[0]);
			sleep(SLEEP_TIME);
			continue;
		}

		// If change happened then
		b_ACStatus = b_ACStatus_real;
		// If state changed to plugged
		if(b_ACStatus_real == STATUS_PLUGGED){
			if(fFlags & fVerbose)
				printf("[LOG] %s: AC was plugged\n", argv[0]);
			char shell_command[sizeof(SHELL_PLAYER) + sizeof(sound_plug) + 2];
			snprintf(shell_command, sizeof(shell_command),"%s %s", SHELL_PLAYER, sound_plug);
			system(shell_command);

			continue;
		}
		// If state changed to unplugged - no if needed
		if(fFlags & fVerbose)
			printf("[LOG] %s: AC was unplugged\n", argv[0]);
		char shell_command[sizeof(SHELL_PLAYER) + sizeof(sound_unplug) + 2];
		snprintf(shell_command, sizeof(shell_command),"%s %s", SHELL_PLAYER, sound_unplug);
		system(shell_command);
		//continue;	
	}

	return 0;
}
