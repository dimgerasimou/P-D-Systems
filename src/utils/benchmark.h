#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "matrix.h"

int benchmark_cc(int (*cc_func)(const CSCBinaryMatrix*, const int), const CSCBinaryMatrix* mat,
                  int n_threads, int n_trials, const char* name);

#endif // BENCHMARK_H