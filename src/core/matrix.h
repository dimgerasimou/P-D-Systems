/**
 * @file matrix.h
 * @brief Compressed Sparse Column (CSC) matrix utilities for binary matrices.
 *
 * Provides functionality to load, free, and print sparse binary matrices
 * stored in CSC format.
 */

#ifndef MATRIX_H
#define MATRIX_H

#include <stddef.h>
#include <stdint.h>

/**
 * @struct CSCBinaryMatrix
 * @brief Compressed Sparse Column (CSC) representation of a binary matrix.
 *
 * Non-zero entries are implicitly 1. Stores only row indices and column pointers.
 */
typedef struct {
    size_t nrows;       /**< Number of rows in the matrix */
    size_t ncols;       /**< Number of columns in the matrix */
    size_t nnz;         /**< Number of non-zero (1) entries */
    uint32_t *row_idx;  /**< Row indices of non-zero elements (length nnz) */
    uint32_t *col_ptr;  /**< Column pointers (length ncols + 1) */
} CSCBinaryMatrix;

/**
 * @brief Load a sparse binary matrix from a MAT file.
 *
 * @param filename Path to the MAT file containing the matrix.
 * @param matrix_name Name of the struct variable in the file.
 * @param field_name Name of the field containing the sparse matrix.
 * @return Pointer to a newly allocated CSCBinaryMatrix on success, NULL on failure.
 *
 * @note The returned matrix must be freed using csc_free_matrix().
 */
CSCBinaryMatrix *csc_load_matrix(const char *filename,
                                 const char *matrix_name,
                                 const char *field_name);

/**
 * @brief Free a CSCBinaryMatrix and its internal arrays.
 *
 * @param m Pointer to the CSCBinaryMatrix to free. Safe to call with NULL.
 */
void csc_free_matrix(CSCBinaryMatrix *m);

/**
 * @brief Print a sparse binary matrix in coordinate format.
 *
 * @param m Pointer to the CSCBinaryMatrix to print.
 *
 * @note Prints as (row, col) pairs. Indices are 1-based.
 */
void csc_print_matrix(CSCBinaryMatrix *m);

#endif /* MATRIX_H */
