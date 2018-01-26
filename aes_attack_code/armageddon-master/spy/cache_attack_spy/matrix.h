#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

typedef struct matrix
{
    size_t ac;             // addresses count
    size_t cc;             // character count
    size_t *addresses;    // pointer to address array
    uint64_t thres;        // use 0 to represent the values below thres
    uint64_t *thresholds;  // threshold for every address
    char **characters;     // pointer to character array
    size_t **matrixvalues; // pointer to the matrix of values
} Matrix;

bool initMatrix(Matrix **mtr);

bool isCharacter(size_t *values, int vc, size_t *valueline);

int getCharacters(Matrix *mtr, uint64_t *values, int **chars);

void printMatrix(FILE *const _Stream, Matrix *mtr);