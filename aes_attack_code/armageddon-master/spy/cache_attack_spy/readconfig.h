#include<stdbool.h>
#include<stdio.h>
#include<string.h>

#include "matrix.h"

#define LINECOUNT 1024
#define WORDLENGTH 16
#define AC_HEADER "address count:"
#define CC_HEADER "character count:"
#define THRES_HEADER "thres:"
#define ADDRESSES_HEADER "addresses:"
#define THRESHOLDS_HEADER "thresholds:"
#define CHARACTERS_HEADER   "characters:"
#define MATRIXVALUES_HEADER "matrixvalues:"
#define VALUE_SEPARATOR '\t'

int
getOffset(FILE* file, char* scanstring);

int
readNextInt(FILE* file);

int64_t
readNextHexInt(FILE* file);

char*
readNextWord(FILE* file);

bool
readConfigFile(FILE* configFile, Matrix* mtr);

size_t
readAddressCount(FILE* configFile);

size_t
readCharacterCount(FILE* configFile);

uint64_t
readThres(FILE* configFile);

void
readAddresses(FILE* configFile, size_t addressCount, size_t** addresses);

void
readThresholds(FILE* configFile, size_t addressCount, uint64_t** thresholds);

void
readCharacters(FILE* configFile, size_t characterCount, char*** characters);

void
readMatrixValues(FILE* configFile, size_t addressCount, size_t characterCount, size_t*** matrixValues);