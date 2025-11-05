#include "io.h"
#include <stdio.h>
#include <stdlib.h>
#include "cc_union_find_like.h"

const char *program_name = "pardisV0";

void bfs_component(CSCBinaryMatrix *matrix, uint32_t startVertex, uint32_t component_id, uint32_t *labels) {
    uint32_t *queue = malloc(matrix->ncols * sizeof(uint32_t));
    size_t front = 0, rear = 0;

    labels[startVertex] = component_id;
    queue[rear++] = startVertex;

    while (front < rear) {
        uint32_t current = queue[front++];

        for (uint32_t j = matrix->col_ptr[current]; j < matrix->col_ptr[current + 1]; j++) {
            uint32_t neighbor = matrix->row_idx[j];
            if (labels[neighbor] == UINT32_MAX) { // αν δεν έχει label
                labels[neighbor] = component_id;
                queue[rear++] = neighbor;
            }
        }
    }

    free(queue);
}

// Υπολογισμός αριθμού connected components
uint32_t count_connected_components(CSCBinaryMatrix *matrix) {
    uint32_t *labels = malloc(matrix->ncols * sizeof(uint32_t));
    for (size_t i = 0; i < matrix->ncols; i++)
        labels[i] = UINT32_MAX; // αρχικά χωρίς component

    uint32_t component_id = 0;

    for (size_t v = 0; v < matrix->ncols; v++) {
        if (labels[v] == UINT32_MAX) {
            bfs_component(matrix, v, component_id, labels);
            component_id++;
        }
    }

    free(labels);
    return component_id;
}

int main(int argc, char *argv[]) {

    CSCBinaryMatrix *matrix;

    set_program_name(argv[0]);

    if (argc != 2) {
        print_error(__func__, "invalid arguments", 0);
        return 1;
    }
    
    matrix = csc_load_matrix(argv[1], "Problem", "A");
    if (!matrix)
        return 1;

    uint32_t num_components = count_connected_components(matrix);
    printf("Number of connected components: %u\n", num_components);

        printf("%d\n", cc_union_find(matrix));


    csc_free_matrix(matrix);
    return 0;
}
