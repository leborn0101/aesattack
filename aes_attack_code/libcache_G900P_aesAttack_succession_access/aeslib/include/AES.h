#pragma once

#include <stdint.h>
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

void aes_ecb_encrypt(const unsigned char *ckey, const unsigned char *pt,unsigned char *ct, unsigned char flag);
