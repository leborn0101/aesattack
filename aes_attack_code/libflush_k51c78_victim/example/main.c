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

#define _STR(x) #x
#define STR(x) _STR(x)

void doaes(unsigned char *inaes, aesTableSet *tables); // execute aes algorithm with a given plaintext

unsigned char key[AES_KEY_LENGTH] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
                                     0x77, 0x00, 0x11, 0x22, 0x33, 0x44,
                                     0x55, 0x66, 0x77};

static void print_help(char* argv[]) {
  fprintf(stdout, "Usage: %s [OPTIONS]\n", argv[0]);
  fprintf(stdout, "\t-c, -cpu <value>\t Bind to cpu (default: " STR(BIND_TO_CPU) ")\n");
  fprintf(stdout, "\t-t, -help\thelp page\n");
}                                                              

int main(int argc, char* argv[]) {

  aesTableSet tables; // the struct contains aes t-table's start addr
  libflush_session_t *libflush_session;

  /* Define parameters */
  size_t cpu = BIND_TO_CPU;

  static const char* short_options = "c:h";
  static struct option long_options[] = {
    {"cpu", required_argument, NULL, 'c'},
    {"help", no_argument, NULL, 'h'},
    { NULL,             0, NULL, 0}
  };

  size_t number_of_cpus = sysconf(_SC_NPROCESSORS_ONLN);
  fprintf(stdout, "NUMBER OF CPUS: %zu\n", number_of_cpus);

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

  if (libflush_bind_to_cpu(cpu) == false) {
    fprintf(stderr, "Warning: Victim could not bind to CPU: %zu\n", cpu);
  } else {
    fprintf(stdout, "victim process bind to cpu %zu\n", cpu);
  }

  fprintf(stdout, "now start victim program\n");

  unsigned char inaes[AES_PLAINTEXT_LENGTH] = {0x61, 0x62, 0x63, 0x64, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x61, 0x61, 0x62, 0x63, 0x64, 0x65};
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

