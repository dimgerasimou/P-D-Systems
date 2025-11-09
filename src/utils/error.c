#include <string.h>
#include <stdio.h>

#include "error.h"

extern const char *program_name;

void set_program_name(const char *argv0) {
	if (argv0) {
		const char *slash = strrchr(argv0, '/');
		program_name = slash ? slash + 1 : argv0;
	}
}

void print_error(const char *func, const char *msg, int err) {
	if (err)
		fprintf(stderr, "%s: %s: %s: %s\n", program_name, func, msg, strerror(err));
	else
		fprintf(stderr, "%s: %s: %s\n", program_name, func, msg);
}
