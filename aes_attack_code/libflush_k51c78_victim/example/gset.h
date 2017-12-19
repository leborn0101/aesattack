//
// Created by f on 2016/11/9.
//

#ifndef DPLSTEE_TEST_AES_GSET_H
#define DPLSTEE_TEST_AES_GSET_H

#define LINE_LENGTH_LOG2 6
#define NUMBER_OF_SETS 512
#include<stdlib.h>
#include<sys/types.h>
#include<stdint.h>


size_t
get_set_index(void* physical_address)
{
  return ((uintptr_t)physical_address >> LINE_LENGTH_LOG2) % NUMBER_OF_SETS;
}

#endif //DPLSTEE_TEST_AES_GSET_H
