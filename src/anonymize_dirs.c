#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <ftw.h>
#include <time.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "config.h"
#include "utils.h"

char SPRINTF_BUFFER[1024];
bool TWOCOL = false;
char INPUT_PATH[1024];

/*
 * Callback passed to ftw. For the given path, if that path is a
 * directory, create a corresponding directory in WORKING_DIR/anonymized.
 * If it is a regular file, pipe it to ./bin/anonymize INPUT_PATH (where
 * INPUT_PATH is a CLI argument containing names to replace). If TWOCOL
 * is set (again a CLI flag), "-t" is also passed as a flag, telling
 * ./bin/anonymize to parse a two-column CSV file rather than a simple
 * list of names.
 */
int walk_fn(const char *path, const struct stat *sb, int typeflag)
{
	if (S_ISREG(sb->st_mode)) { /* If this is a regular file: */
		/* Open the input file */
		int input = open(path, O_RDONLY);
		if (input < 0) { perror(path); exit(1); }

		/* Open the output file in WORKING_DIR/anonymized */
		snprintf(SPRINTF_BUFFER, 1024, WORKING_DIR "/anonymized/%s", path);
		int output = creat(SPRINTF_BUFFER, 0644);
		if (output < 0) { perror(SPRINTF_BUFFER); exit(1); }

		/* Run ./bin/anonymize with the appropriate arguments */
		if (TWOCOL) {
			ex_io(input, output, "./bin/anonymize", "-t", INPUT_PATH, NULL);
		} else {
			ex_io(input, output, "./bin/anonymize", INPUT_PATH, NULL);
		}

		/* Clean child processes */
		while (wait(NULL) > 0);
	} else { /* If this is a directory, create it */
		snprintf(SPRINTF_BUFFER, 1024, WORKING_DIR "/anonymized/%s", path);
		mkdir(SPRINTF_BUFFER, 0777);
	}
	return 0;
}

int main(int argc, char **argv)
{
	int arg;
	while ((arg = getopt(argc, argv, "t")) != -1) {
		switch (arg) {
			case 't':
				TWOCOL = true;
				break;
		}
	}

	if (optind + 1 < argc) {
		strncpy(INPUT_PATH, argv[optind], 1024);
	} else {
		exit(1);
	}

	mkdir(WORKING_DIR, 0777);
	mkdir(WORKING_DIR "/anonymized", 0777);
	ftw(argv[optind + 1], walk_fn, 8);
}