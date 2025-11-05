#ifndef IO_H
#define IO_H

#include <stddef.h>
#include <stdint.h>

/**
 * @struct CSCBinaryMatrix
 * @brief Compressed Sparse Column (CSC) representation of a binary matrix.
 *
 * Stores a sparse binary matrix (0/1) in CSC format. Non-zero values
 * are implicitly 1, so only row indices and column pointers are stored.
 */
typedef struct {
    size_t nrows;           /**< Number of rows in the matrix */
    size_t ncols;           /**< Number of columns in the matrix */
    size_t nnz;             /**< Number of non-zero (1) entries */
    uint32_t *row_idx;  /**< Row indices of non-zero elements (length nnz) */
    uint32_t *col_ptr;  /**< Column pointers (length ncols + 1) */
} CSCBinaryMatrix;

/**
 * @brief Load a sparse binary matrix from a file.
 *
 * @param filename Path to the file containing the matrix.
 * @param matrix_name Name of the matrix variable in the file.
 * @param field_name Name of the field containing the data.
 * @return Pointer to a newly allocated CSCBinaryMatrix on success,
 *         NULL on failure.
 *
 * @note The returned matrix must be freed using csc_free_matrix().
 */
CSCBinaryMatrix *csc_load_matrix(const char *filename, const char *matrix_name, const char *field_name);

/**
 * @brief Free a CSCBinaryMatrix and its internal arrays.
 *
 * @param m Pointer to the CSCBinaryMatrix to free.
 */
void csc_free_matrix(CSCBinaryMatrix *m);

/**
 * @brief Print a sparse binary matrix in coordinate format.
 *
 * @param m Pointer to the CSCBinaryMatrix to print.
 *
 * @note Prints as (row, col) pairs. Row and column indices are 1-based.
 */
void csc_print_matrix(CSCBinaryMatrix *m);

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

#endif
