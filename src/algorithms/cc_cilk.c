/**
 * @file cc_cilk.c
 * @brief Optimized OpenCilk implementations for connected components.
 *
 * - Variant 0: Label propagation (per-worker local flags, relaxed atomics)
 * - Variant 1: Union-Find with Rem's algorithm (dynamic task scheduling)
 *
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

#include "connected_components.h"

/* ------------------------- Union-Find Utilities --------------------------- */

static inline uint32_t
find_compress(uint32_t *label, uint32_t x)
{
    uint32_t root = x;
    while (label[root] != root) root = label[root];
    while (x != root) {
        uint32_t next = label[x];
        if (label[x] == next) break;
        label[x] = root;
        x = next;
    }
    return root;
}

static inline void
union_rem(uint32_t *label, uint32_t a, uint32_t b)
{
    const int MAX_RETRIES = 10;
    for (int r = 0; r < MAX_RETRIES; r++) {
        a = find_compress(label, a);
        b = find_compress(label, b);
        if (a == b) return;
        if (a > b) { uint32_t t = a; a = b; b = t; }
        uint32_t expected = b;
        if (__atomic_compare_exchange_n(&label[b], &expected, a,
                                        0, __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
            return;
        }
        b = expected;
    }
    /* fallback */
    a = find_compress(label, a);
    b = find_compress(label, b);
    if (a != b) {
        if (a > b) { uint32_t t = a; a = b; b = t; }
        __atomic_store_n(&label[b], a, __ATOMIC_RELEASE);
    }
}

/* ------------------------- Union-Find (Cilk) ------------------------------ */

static int
cc_union_find(const CSCBinaryMatrix *matrix)
{
    if (!matrix || matrix->nrows == 0) return 0;

    const uint32_t n = (uint32_t)matrix->nrows;
    uint32_t *label = malloc(n * sizeof(uint32_t));
    if (!label) return -1;

    cilk_for (uint32_t i = 0; i < n; i++) label[i] = i;

    cilk_for (uint32_t col = 0; col < matrix->ncols; col++) {
        uint32_t start = matrix->col_ptr[col];
        uint32_t end   = matrix->col_ptr[col + 1];
        for (uint32_t j = start; j < end; j++) {
            uint32_t row = matrix->row_idx[j];
            if (row < n) union_rem(label, row, col);
        }
    }

    cilk_for (uint32_t i = 0; i < n; i++) find_compress(label, i);

    uint32_t count = 0;
    cilk_for (uint32_t i = 0; i < n; i++) {
        if (label[i] == i) {
            __atomic_fetch_add(&count, 1, __ATOMIC_RELAXED);
        }
    }

    free(label);
    return (int)count;
}

/* ------------------------- Label Propagation (Cilk) ---------------------- */

static int
cc_label_propagation(const CSCBinaryMatrix *matrix)
{
    if (!matrix || matrix->nrows == 0) return 0;

    uint32_t *label = malloc(sizeof(uint32_t) * matrix->nrows);
    if (!label) return -1;

    for (size_t i = 0; i < matrix->nrows; i++)
        label[i] = i;

    uint8_t finished;
    do {
        finished = 1;

        // Per-column processing with per-worker local change flag
        cilk_for (size_t col = 0; col < matrix->ncols; col++) {
            uint8_t local_changed = 0;

            for (uint32_t j = matrix->col_ptr[col]; j < matrix->col_ptr[col + 1]; j++) {
                uint32_t row = matrix->row_idx[j];
                uint32_t lc = label[col];
                uint32_t lr = label[row];

                if (lc != lr) {
                    uint32_t minval = lc < lr ? lc : lr;
                    if (lc != minval)
                        __atomic_store_n(&label[col], minval, __ATOMIC_RELAXED);
                    else
                        __atomic_store_n(&label[row], minval, __ATOMIC_RELAXED);

                    local_changed = 1;
                }
            }

            // Single atomic write per worker if any change occurred
            if (local_changed)
                finished = 0;
        }

    } while (!finished);

    // Bitmap-based counting (faster than sorting in parallel)
    size_t bitmap_size = (matrix->nrows + 63) / 64;
    uint64_t *bitmap = calloc(bitmap_size, sizeof(uint64_t));
    if (!bitmap) {
        free(label);
        return -1;
    }
    
    // Bitmap construction: set bit for each unique label
    {
        for (size_t i = 0; i < matrix->nrows; i++) {
            uint32_t val = label[i];
            size_t word = val >> 6;              // Divide by 64
            uint64_t bit = 1ULL << (val & 63);   // Modulo 64
            
            //#pragma omp atomic
            bitmap[word] |= bit;
        }
    }
    
    // Count set bits using hardware popcount
    uint32_t count = 0;
    for (size_t i = 0; i < bitmap_size; i++) {
        count += __builtin_popcountll(bitmap[i]);
    }
    
    free(bitmap);
    free(label);
    return (int)count;
}

/* -------------------------- Public Dispatch -------------------------------- */

int
cc_cilk(const CSCBinaryMatrix *matrix,
        const unsigned int __attribute__((unused)) n_threads,
	    const unsigned int algorithm_variant)
{

    switch (algorithm_variant) {
        case 0: return cc_label_propagation(matrix);
        case 1: return cc_union_find(matrix);
        default: return -1;
    }
}
