#pragma once

#include <stdint.h>
#include <sys/mman.h>
#include <sched.h>
#include <sys/sysinfo.h>
typedef int32_t int32;
typedef uint32_t uint32;
typedef int16_t int16;
typedef uint16_t uint16;
typedef uint8_t uint8;

typedef struct {
	uint32_t	skey[64];	/**< Key schedule (either encrypt or decrypt) */
	uint16_t	rounds;		/**< Number of rounds */
	uint16_t	type;		/**< PS_AES_ENCRYPT or PS_AES_DECRYPT (inverse) key */
} psAesKey_t;

typedef struct {
	uint16_t	t0_si[256];	/** aes t1 start address map to set */
	uint16_t	t1_si[256];	/** aes t2 start address map to set */
	uint16_t	t2_si[256];	/** aes t3 start address map to set */
	uint16_t	t3_si[256];	/** aes t4 start address map to set */
} aesTableSet;

void aes_ecb_encrypt(const unsigned char *ckey, const unsigned char *pt, unsigned char *ct, aesTableSet *tss);
