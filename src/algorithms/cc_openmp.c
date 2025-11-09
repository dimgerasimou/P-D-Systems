#include <stdlib.h>
#include <omp.h>

#include "connected_components.h"


static int swap_min(uint32_t *label, uint32_t i, uint32_t j) {
    if (label[i] == label[j])
        return 0;
    
    if (label[i] < label[j]) {
        label[j] = label[i];
    } else {
        label[i] = label[j];
    }
    return 1;
}

static int cmp_uint32(const void *a, const void *b) {
    uint32_t x = *(uint32_t*)a;
    uint32_t y = *(uint32_t*)b;
    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}

int cc_openmp(const CSCBinaryMatrix *matrix, const int n_threads __attribute__((unused))) {

    uint32_t *label=malloc(sizeof(uint32_t)*matrix->nrows);
    for(size_t i=0;i<matrix->nrows;i++){
        label[i]=i;
    }

    uint8_t finished;
    do {

        
        finished = 1;
        
        #pragma  omp parallel 
        {
            uint8_t changed = 0;
            
            #pragma omp for
            for (size_t i = 0; i < matrix->ncols; i++) {
                for(uint32_t j = matrix->col_ptr[i]; j < matrix->col_ptr[i+1]; j++) {
                    uint32_t r, c;

                    c = i;
                    r = matrix->row_idx[j];

                    if (swap_min(label, c, r)) {
                        changed = 1;
                    }
                }
            }

            if (changed) {
                    #pragma omp atomic write
                    finished = 0;
            }
        }
    } while (!finished);

    qsort(label, matrix->nrows, sizeof(uint32_t), cmp_uint32);

    uint32_t uniqueCount = 0;
    for (size_t i = 0; i < matrix->nrows; i++) {
        if (i == 0 || label[i] != label[i-1])
            uniqueCount++;
    }

    free(label);

    return (int) uniqueCount;
}
