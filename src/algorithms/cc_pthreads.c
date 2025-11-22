/**
 * @file cc_pthreads.c
 * @brief Further optimized parallel algorithms for computing connected components using Pthreads.
 *
 * This module implements two parallel algorithms for finding connected
 * components in an undirected graph using Pthreads:
 *
 * - Label Propagation (variant 0): Iterative parallel label propagation
 *   with optimized atomic updates and bitmap-based counting.
 *
 * - Union-Find with Rem's Algorithm (variant 1): Lock-free parallel
 *   union-find using compare-and-swap (CAS) operations and path compression.
 *
 * Key optimizations:
 * - Label propagation: Conditional atomics to reduce contention
 * - Union-find: Restored optimal retry count and chunk size
 */

#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <stdatomic.h>

#include "connected_components.h"

/* ========================================================================= */
/*                          UNION-FIND UTILITIES                              */
/* ========================================================================= */

/**
 * @brief Finds the root of a node with path compression.
 *
 * Traverses parent pointers until reaching the root, and compresses the
 * path by making all nodes along the path point directly to the root.
 * The early-exit optimization avoids redundant writes if the path is
 * already compressed.
 *
 * @param label Array of parent pointers representing disjoint sets.
 * @param x Node index to find the root for.
 * @return Root of the set containing x.
 */
static inline uint32_t
find_compress(uint32_t *label, uint32_t x)
{
	uint32_t root = x;

	while (label[root] != root)
		root = label[root];

	while (x != root) {
		uint32_t next = label[x];
		if (label[x] == next) break;  // Already compressed
		label[x] = root;
		x = next;
	}

	return root;
}

/**
 * @brief Unites two disjoint sets using CAS with optimal retry count.
 *
 * Implements Rem's algorithm for parallel union-find.
 * REVERTED: Restored original retry count (10) for better performance.
 *
 * Canonical ordering ensures deterministic results even in parallel execution.
 *
 * @param label Array of parent pointers representing disjoint sets.
 * @param a First node.
 * @param b Second node.
 */
static inline void
union_rem(uint32_t *label, uint32_t a, uint32_t b)
{
	const int MAX_RETRIES = 10;  // Restored original value

	for (int retries = 0; retries < MAX_RETRIES; retries++) {
		a = find_compress(label, a);
		b = find_compress(label, b);

		if (a == b)
			return;

		if (a > b) {
			uint32_t tmp = a; a = b; b = tmp;
		}

		uint32_t expected = b;
		if (__atomic_compare_exchange_n(&label[b], &expected, a,
										0, __ATOMIC_RELAXED, __ATOMIC_RELAXED))
			return;

		b = expected;
	}

	// Fallback after max retries
	a = find_compress(label, a);
	b = find_compress(label, b);
	if (a != b) {
		if (a > b) { uint32_t tmp = a; a = b; b = tmp; }
		__atomic_store_n(&label[b], a, __ATOMIC_RELEASE);
	}
}

/* ========================================================================= */
/*                       UNION-FIND WORKER (THREAD)                           */
/* ========================================================================= */

/**
 * @struct uf_args_t
 * @brief Arguments for the union-find worker thread.
 *
 * Each thread uses these arguments to perform unions on a chunk of columns
 * from the sparse matrix.
 */
typedef struct {
	const CSCBinaryMatrix *mat; ///< Input CSC binary matrix.
	uint32_t *label;            ///< Label array representing disjoint sets.
	atomic_uint *next_col;      ///< Atomic counter for dynamic scheduling of columns.
	uint32_t ncols;             ///< Total number of columns in the matrix.
} uf_args_t;

/**
 * @brief Worker function for parallel union-find.
 *
 * Each thread grabs a chunk of columns from the global atomic counter
 * and performs union operations on all edges in those columns.
 * Uses atomic CAS operations to ensure lock-free updates.
 *
 * REVERTED: Restored original chunk size (4096) for better performance.
 *
 * @param arg Pointer to uf_args_t structure containing arguments.
 * @return NULL
 */
static void *
uf_worker(void *arg)
{
	uf_args_t *a = arg;
	const uint32_t CHUNK = 4096;  // Restored original value

	while (1) {
		uint32_t col = atomic_fetch_add(a->next_col, CHUNK);
		if (col >= a->ncols) break;

		uint32_t endcol = col + CHUNK;
		if (endcol > a->ncols) endcol = a->ncols;

		for (uint32_t c = col; c < endcol; c++) {
			uint32_t start = a->mat->col_ptr[c];
			uint32_t end   = a->mat->col_ptr[c + 1];

			for (uint32_t j = start; j < end; j++) {
				uint32_t row = a->mat->row_idx[j];
				union_rem(a->label, row, c);
			}
		}
	}

	return NULL;
}

/* ========================================================================= */
/*                   COUNT ROOTS (THREAD-LOCAL REDUCTION)                     */
/* ========================================================================= */

/**
 * @struct count_args_t
 * @brief Arguments for counting roots in a subset of the label array.
 *
 * Each thread counts the number of roots in its assigned range.
 */
typedef struct {
	uint32_t *label;    ///< Label array representing disjoint sets.
	uint32_t begin;     ///< Start index of the range.
	uint32_t end;       ///< End index of the range (exclusive).
	uint32_t local;     ///< Thread-local count of roots.
} count_args_t;

/**
 * @brief Worker function to count roots in a given range.
 *
 * Each thread iterates over its assigned slice of the label array and
 * counts how many elements are roots (label[i] == i).
 *
 * @param arg Pointer to count_args_t.
 * @return NULL
 */
static void *
count_worker(void *arg)
{
	count_args_t *a = arg;
	uint32_t cnt = 0;

	for (uint32_t i = a->begin; i < a->end; i++)
		if (a->label[i] == i)
			cnt++;

	a->local = cnt;
	return NULL;
}

/* ========================================================================= */
/*                       PUBLIC: UNION-FIND (PTHREADS)                        */
/* ========================================================================= */

/**
 * @brief Computes connected components using parallel union-find.
 *
 * Algorithm phases:
 * 1. Initialize each node as its own root.
 * 2. Perform parallel union operations on edges using multiple threads.
 * 3. Flatten all paths to roots for accurate counting.
 * 4. Count roots in parallel using thread-local accumulation.
 *
 * REVERTED: Restored original chunk size for optimal performance.
 *
 * @param matrix Sparse CSC binary matrix.
 * @param n_threads Number of Pthreads to use.
 * @return Number of connected components, or -1 on error.
 */
static int
cc_union_find(const CSCBinaryMatrix *matrix, unsigned int n_threads)
{
	if (!matrix || matrix->nrows == 0) return 0;
	const uint32_t n = matrix->nrows;

	uint32_t *label = malloc(n * sizeof(uint32_t));
	if (!label) return -1;

	for (uint32_t i = 0; i < n; i++)
		label[i] = i;

	atomic_uint next_col;
	atomic_store(&next_col, 0);

	pthread_t th[n_threads];
	uf_args_t args = { .mat = matrix, .label = label, .next_col = &next_col, .ncols = matrix->ncols };

	for (unsigned i = 0; i < n_threads; i++)
		pthread_create(&th[i], NULL, uf_worker, &args);
	for (unsigned i = 0; i < n_threads; i++)
		pthread_join(th[i], NULL);

	// Flatten all paths
	for (uint32_t i = 0; i < n; i++)
		find_compress(label, i);

	// Count roots in parallel
	uint32_t total = 0;
	uint32_t chunk = (n + n_threads - 1) / n_threads;
	count_args_t cargs[n_threads];

	for (unsigned i = 0; i < n_threads; i++) {
		cargs[i].label = label;
		cargs[i].begin = i * chunk;
		cargs[i].end   = (cargs[i].begin + chunk > n ? n : cargs[i].begin + chunk);
		pthread_create(&th[i], NULL, count_worker, &cargs[i]);
	}
	for (unsigned i = 0; i < n_threads; i++) {
		pthread_join(th[i], NULL);
		total += cargs[i].local;
	}

	free(label);
	return (int)total;
}

/* ========================================================================= */
/*                       LABEL PROPAGATION WORKER                              */
/* ========================================================================= */

/**
 * @struct lp_args_t
 * @brief Arguments for the label propagation worker thread.
 *
 * Each thread processes a subset of columns and updates labels atomically.
 */
typedef struct {
	const CSCBinaryMatrix *mat;    ///< Input CSC binary matrix.
	uint32_t *label;               ///< Label array.
	atomic_uint *next_col;         ///< Atomic column counter for dynamic scheduling.
	atomic_uint *global_change;    ///< Atomic flag indicating if any label changed.
} lp_args_t;

/**
 * @brief Worker function for optimized parallel label propagation.
 *
 * Each thread grabs a chunk of columns dynamically, then iterates over all
 * edges in the chunk, updating the labels of connected nodes to the minimum
 * value using conditional atomic stores. Sets a global flag if any label changed.
 *
 * Optimization: Only performs atomic stores when the value actually changes,
 * dramatically reducing atomic operation overhead and contention.
 *
 * Larger chunks (4096) reduce scheduling overhead.
 *
 * @param arg Pointer to lp_args_t structure.
 * @return NULL
 */
static void *
lp_worker(void *arg)
{
	lp_args_t *a = arg;
	const uint32_t CHUNK = 4096;  // Larger chunks for less overhead

	while (1) {
		uint32_t col = atomic_fetch_add(a->next_col, CHUNK);
		if (col >= a->mat->ncols) break;

		uint32_t endcol = col + CHUNK;
		if (endcol > a->mat->ncols) endcol = a->mat->ncols;

		uint8_t changed = 0;

		for (uint32_t c = col; c < endcol; c++) {
			for (uint32_t j = a->mat->col_ptr[c]; j < a->mat->col_ptr[c+1]; j++) {
				uint32_t r = a->mat->row_idx[j];
				uint32_t lc = a->label[c];
				uint32_t lr = a->label[r];

				if (lc != lr) {
					uint32_t m = lc < lr ? lc : lr;
					
					// Conditional atomic stores: only update if value changes
					if (lc > m) {
						__atomic_store_n(&a->label[c], m, __ATOMIC_RELAXED);
						changed = 1;
					}
					if (lr > m) {
						__atomic_store_n(&a->label[r], m, __ATOMIC_RELAXED);
						changed = 1;
					}
				}
			}
		}

		if (changed)
			atomic_store(a->global_change, 1);
	}

	return NULL;
}

/* ========================================================================= */
/*                       PUBLIC: LABEL PROPAGATION                            */
/* ========================================================================= */

/**
 * @brief Computes connected components using optimized parallel label propagation.
 *
 * Algorithm phases:
 * 1. Initialize each node with its own label.
 * 2. Iterate until convergence:
 *    - Each thread updates labels of connected nodes with conditional atomics.
 *    - A global atomic flag indicates whether any changes occurred.
 * 3. Construct a bitmap of unique labels to count components efficiently.
 *
 * Key optimization: Only perform atomic stores when values actually change,
 * which dramatically reduces atomic operation overhead.
 *
 * @param matrix Sparse CSC binary matrix.
 * @param n_threads Number of Pthreads to use.
 * @return Number of connected components, or -1 on error.
 */
static int
cc_label_propagation(const CSCBinaryMatrix *matrix, unsigned int n_threads)
{
	const uint32_t n = matrix->nrows;
	uint32_t *label = malloc(n * sizeof(uint32_t));
	if (!label) return -1;

	for (uint32_t i = 0; i < n; i++)
		label[i] = i;

	atomic_uint global_change;

	do {
		atomic_store(&global_change, 0);
		atomic_uint next_col;
		atomic_store(&next_col, 0);

		pthread_t th[n_threads];
		lp_args_t args = { .mat = matrix, .label = label, .next_col = &next_col, .global_change = &global_change };

		for (unsigned i = 0; i < n_threads; i++)
			pthread_create(&th[i], NULL, lp_worker, &args);
		for (unsigned i = 0; i < n_threads; i++)
			pthread_join(th[i], NULL);

	} while (atomic_load(&global_change));

	// Bitmap counting
	size_t bsz = (n + 63) / 64;
	uint64_t *bitmap = calloc(bsz, sizeof(uint64_t));
	if (!bitmap) { free(label); return -1; }

	for (uint32_t i = 0; i < n; i++) {
		uint32_t v = label[i];
		bitmap[v >> 6] |= (1ULL << (v & 63));
	}

	uint32_t count = 0;
	for (size_t i = 0; i < bsz; i++)
		count += __builtin_popcountll(bitmap[i]);

	free(bitmap);
	free(label);
	return count;
}

/* ========================================================================= */
/*                        PUBLIC ENTRYPOINT                                   */
/* ========================================================================= */

/**
 * @brief Computes connected components using Pthreads.
 *
 * Dispatches to the appropriate algorithm based on the variant parameter.
 *
 * @param matrix Sparse CSC binary matrix.
 * @param n_threads Number of threads to use.
 * @param variant Algorithm variant: 0 = label propagation, 1 = union-find
 * @return Number of connected components, or -1 on error.
 */
int
cc_pthreads(const CSCBinaryMatrix *matrix,
            unsigned int n_threads,
            unsigned int variant)
{
	switch (variant) {
	case 0: return cc_label_propagation(matrix, n_threads);
	case 1: return cc_union_find(matrix, n_threads);
	default: return -1;
	}
}