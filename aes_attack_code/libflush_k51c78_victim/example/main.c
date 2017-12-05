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

#define BIND_TO_CPU 0
#define AES_KEY_LENGTH 16       // the length of key
#define AES_PLAINTEXT_LENGTH 16 // the length of plaintext

void doaes(unsigned char *inaes, aesTableSet *tables); // execute aes algorithm with a given plaintext

unsigned char key[AES_KEY_LENGTH] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
                                     0x77, 0x00, 0x11, 0x22, 0x33, 0x44,
                                     0x55, 0x66, 0x77};                                                              

int main(int argc, char* argv[]) {

  aesTableSet tables; // the struct contains aes t-table's start addr
  libflush_session_t *libflush_session;

  /* Define parameters */
  size_t cpu = BIND_TO_CPU;

  size_t number_of_cpus = sysconf(_SC_NPROCESSORS_ONLN);

  fprintf(stdout, "NUMBER OF CPUS: %zu\n", number_of_cpus);

  /* Bind to CPU */
  cpu = cpu % number_of_cpus;

  if (libflush_bind_to_cpu(cpu) == false) {
    fprintf(stderr, "Warning: Could not bind to CPU: %zu\n", cpu);
  } else {
    fprintf(stdout, "attack process bind to cpu %d\n", BIND_TO_CPU);
  }

  fprintf(stdout, "now start victim program\n");

  unsigned char inaes[AES_PLAINTEXT_LENGTH] = {0x61, 0x62, 0x63, 0x64, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75};
  doaes(inaes, &tables);

  while (true) {
    doaes(inaes, &tables);
  }

  aes_ecb_encrypt(NULL, NULL, NULL, &tables);
  fprintf(stdout, "victime program completed\n");
  return 0;
}

void doaes(unsigned char *inaes, aesTableSet *tables) {
  unsigned char outaes[16] = {0};
  aes_ecb_encrypt(key, inaes, outaes, tables);
}

