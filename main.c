#include "io.h"

const char *program_name = "pardisV0";

int main(int argc, char *argv[]) {

    CSCBinaryMatrix *matrix;

    set_program_name(argv[0]);

    if (argc != 2) {
        print_error(__func__, "invalid arguments", 0);
        return 1;
    }
    
    matrix = load_sparse_matrix(argv[1], "Problem", "A");
    if (!matrix)
        return 1;

    print_sparse_matrix(matrix);

    free_sparse_matrix(matrix);
    return 0;
}
