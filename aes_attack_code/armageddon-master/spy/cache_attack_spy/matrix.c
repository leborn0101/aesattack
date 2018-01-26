#include "matrix.h"

bool initMatrix(Matrix **mtr)
{
  if (*mtr == NULL)
  {
    *mtr = (Matrix *)malloc(sizeof(Matrix));
  }
  (*mtr)->ac = 0;
  (*mtr)->cc = 0;
  (*mtr)->addresses = NULL;
  (*mtr)->thres = 0;
  (*mtr)->thresholds = NULL;
  (*mtr)->characters = NULL;
  (*mtr)->matrixvalues = NULL;

  return true;
}

size_t getValue(uint64_t num, uint64_t thres, uint64_t threshold)
{
  // if (num <= thres)
  //   return 0;
  // else
  if (num >= threshold)
    return 2;
  else
    return 0;
}

bool isCharacter(size_t *values, int vc, size_t *valueline)
{
  int i;
  size_t t;
  for (i = 0; i < vc; ++i)
  {
    if (valueline[i] == 0)
      continue;
    else if (valueline[i] == values[i])
      return true;
  }
  return false;
}

int getCharacters(Matrix *mtr, uint64_t *values, int **chars)
{
  unsigned int i, j;
  int charcount;
  bool *characters;
  size_t *vs = (size_t *)malloc(sizeof(size_t) * mtr->ac);

  for (unsigned int i = 0; i < mtr->ac; i++)
  {
    vs[i] = getValue(values[i], 0, mtr->thresholds[i]);
    // fprintf(stdout, "%d\t", vs[i]);
  }
  // fprintf(stdout, "\n");

  characters = (bool *)malloc(sizeof(bool) * mtr->cc);
  charcount = 0;

  for (unsigned int i = 0; i < mtr->cc; ++i)
  {
    if (isCharacter(vs, mtr->ac, mtr->matrixvalues[i]))
    {
      // printf("%s\n", mtr->characters[i]);
      characters[i] = true;
      ++charcount;
    }
    else
      characters[i] = false;
  }

  *chars = (int *)malloc(sizeof(int) * charcount);

  for (i = j = 0; i < mtr->cc; ++i)
  {
    if (characters[i])
      (*chars)[j++] = i;
  }

  free(characters);

  // fprintf(stderr, "getCharacters is finished\n");

  return charcount;
}

void printMatrix(FILE *const _Stream, Matrix *mtr)
{
  if (mtr->ac != 0)
    fprintf(_Stream, "address count:\n%d\n", mtr->ac);
  if (mtr->cc != 0)
    fprintf(_Stream, "character count:\n%d\n", mtr->cc);
  if (mtr->thres != 0)
    fprintf(_Stream, "thres:\n%" PRIu64 "\n", mtr->thres);
  if (mtr->addresses != NULL)
  {
    fprintf(_Stream, "addresses:\n");
    for (unsigned int i = 0; i < mtr->ac; ++i)
    {
      fprintf(_Stream, "%8p\t", (void *)mtr->addresses[i]);
    }
    fprintf(_Stream, "\n");
  }

  if (mtr->thresholds != NULL)
  {
    fprintf(_Stream, "threshold:\n");
    for (unsigned int i = 0; i < mtr->ac; ++i)
    {
      fprintf(_Stream, "%" PRIu64 "\t", mtr->thresholds[i]);
    }
    fprintf(_Stream, "\n");
  }

  if (mtr->characters != NULL)
  {
    fprintf(_Stream, "chracters:\n");
    for (unsigned int i = 0; i < mtr->cc; ++i)
    {
      fprintf(_Stream, "%s\t", mtr->characters[i]);
    }
    fprintf(_Stream, "\n");
  }

  if (mtr->matrixvalues != NULL)
  {
    fprintf(_Stream, "matrixvalues:\n");
    for (unsigned int i = 0; i < mtr->cc; ++i)
    {
      for (unsigned int j = 0; j < mtr->ac; j++)
      {
        fprintf(_Stream, "%d\t", mtr->matrixvalues[i][j]);
      }
      fprintf(_Stream, "\n");
    }
    fprintf(_Stream, "\n");
  }
}