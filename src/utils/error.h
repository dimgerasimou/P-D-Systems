#ifndef ERROR_H
#define ERROR_H

/**
 * @brief Set the program name for error reporting.
 *
 * @param argv0 The first argument from main() (program name).
 */
void set_program_name(const char *argv0);

/**
 * @brief Print an error message to stderr.
 *
 * @param func Name of the function reporting the error.
 * @param msg Description of the error.
 * @param err Error code (e.g., errno).
 */
void print_error(const char *func, const char *msg, int err);

#endif // ERROR_H
