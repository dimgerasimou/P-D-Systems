#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include <matio.h>
#include "io.h"

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

CSCBinaryMatrix *csc_load_matrix(const char *filename,
                                    const char *matrix_name,
                                    const char *field_name)
{
    mat_t *matfp = Mat_Open(filename, MAT_ACC_RDONLY);
    if (!matfp) {
        print_error(__func__, "[matio] failed to open file", errno);
        return NULL;
    }

    matvar_t *Problem = Mat_VarRead(matfp, matrix_name);
    if (!Problem || Problem->class_type != MAT_C_STRUCT) {
        print_error(__func__, "[matio] error reading struct", 0);
        Mat_Close(matfp);
        return NULL;
    }

    matvar_t *field = Mat_VarGetStructFieldByName(Problem, field_name, 0);
    if (!field || field->class_type != MAT_C_SPARSE) {
        print_error(__func__, "[matio] field invalid type or not found", 0);
        Mat_VarFree(Problem);
        Mat_Close(matfp);
        return NULL;
    }

    if (field->data_type != MAT_T_DOUBLE) {
        print_error(__func__, "[matio] invalid datatype", 0);
        Mat_VarFree(Problem);
        Mat_Close(matfp);
        return NULL;
    }

    if (field->rank!=2 && field->dims[0] != field->dims[1]) {
        print_error(__func__, "[matio] invalid matrix dimensions", 0);
        Mat_VarFree(Problem);
        Mat_Close(matfp);
        return NULL;
    }

    CSCBinaryMatrix *m = malloc(sizeof(CSCBinaryMatrix));
    if (!m) {
        print_error(__func__, "malloc() failed", errno);
        return NULL;
    }

    mat_sparse_t *s = (mat_sparse_t*)field->data;

    m->nrows = field->dims[0];
    m->ncols = field->dims[1];

    m->nnz = s->jc[m->ncols];

    m->row_idx = NULL;
    m->col_ptr = NULL;

    m->row_idx = malloc(sizeof(uint32_t) * m->nnz);
    m->col_ptr = malloc(sizeof(uint32_t) * m->ncols + 1);

    if (m->row_idx==NULL || m->col_ptr== NULL) {
        print_error(__func__, "malloc() failed", errno);

        csc_free_matrix(m);
        Mat_VarFree(Problem);
        Mat_Close(matfp);
        return NULL;
    }

    memcpy(m->row_idx, s->ir, m->nnz * sizeof(uint32_t));
    memcpy(m->col_ptr, s->jc, (m->ncols + 1) * sizeof(uint32_t));

    Mat_VarFree(Problem);
    Mat_Close(matfp);

    return m;
}

void csc_free_matrix(CSCBinaryMatrix *m) {
    if (!m)
        return;

    if(m->row_idx){
        free(m->row_idx);
        m->row_idx = NULL;
    }

    if(m->col_ptr){
        free(m->col_ptr);
        m->col_ptr = NULL;
    }

    free(m);
    m = NULL;
}

int num_digits(int n) {
    if (n == 0) return 1;
    if (n < 0) n = -n;

    return (int)log10(n) + 1;
}

void csc_print_matrix(CSCBinaryMatrix *m) {
    int di = 0;
    int dj = 0;
    unsigned int nline = 1;
    const unsigned int maxline = 10;

    di = num_digits(m->nrows);
    dj = num_digits(m->ncols);

    printf("Binary Sparse Matrix:\nN:%zu, M:%zu, Non-Zero Elements:%zu\n\n", m->nrows, m->ncols, m->nnz);

    for (size_t i = 0; i < m->ncols; i++) {
        for(size_t j = m->col_ptr[i]; j < m->col_ptr[i+1]; j++) {
            printf("(%*d,%*d)", di, m->row_idx[j] + 1, dj, (int) i + 1);

            if (nline < maxline) {
                printf(" ");
            } else {
                printf("\n");
                nline = 0;
            }

            nline++;
        }
    }

    printf("\n");
}