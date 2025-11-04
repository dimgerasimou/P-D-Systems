#ifndef IO_H
#define IO_H

#include <stddef.h>

typedef struct {
    size_t nrows;       // number of rows
    size_t ncols;       // number of columns
    size_t nnz;         // number of non-zero (1) entries
    unsigned int *row_idx;    // row indices of nonzeros (length nnz)
    unsigned int *col_ptr;    // start indices of each column (length ncols + 1)
} CSCBinaryMatrix;

CSCBinaryMatrix *load_sparse_matrix(const char *filename,
                                    const char *matrix_name,
                                    const char *field_name);

void free_sparse_matrix(CSCBinaryMatrix *m);
void print_sparse_matrix(CSCBinaryMatrix *m);


void set_program_name(const char *argv0);
void print_error(const char *func, const char *msg, int err);

#endif
