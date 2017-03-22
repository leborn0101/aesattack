#pragma once

#define USE_MATRIX_RSA

//----------------------psmalloc.h--------------------

#include <string.h> /* memset, memcpy */

#define MATRIX_NO_POOL		(void *)0x0

/******************************************************************************/
/*
Native memory routines
*/
#include <stdlib.h> 		/* malloc, free, etc... */

#define MAX_MEMORY_USAGE	0
#define psOpenMalloc()		0
#define psCloseMalloc()
#define psDefineHeap(A, B)
#define psAddPoolCache(A, B)
#define psMalloc(A, B)		malloc(B)
#define psCalloc(A, B, C)	calloc(B, C)
#define psMallocNoPool		malloc
#define psRealloc(A, B, C)	realloc(A, B)
#define psFree(A, B)		free(A)
#define psMemset			memset
#define psMemcpy			memcpy

#include <stdint.h>
typedef int32_t int32;
typedef uint32_t uint32;
typedef int16_t int16;
typedef uint16_t uint16;
typedef uint8_t uint8;

typedef int64_t int64;
typedef uint64_t uint64;


typedef int32 psPool_t;



/******************************************************************************/
/*
Universal return codes
*/
#define PS_SUCCESS			0
#define PS_FAILURE			-1
#define PS_FAIL				PS_FAILURE /* Just another name */

/*	NOTE: Failure return codes MUST be < 0 */
/*	NOTE: The range for core error codes should be between -2 and -29 */
#define PS_ARG_FAIL			-6	/* Failure due to bad function param */
#define PS_PLATFORM_FAIL	-7	/* Failure as a result of system call error */
#define PS_MEM_FAIL			-8	/* Failure to allocate requested memory */
#define PS_LIMIT_FAIL		-9	/* Failure on sanity/limit tests */
#define PS_UNSUPPORTED_FAIL	-10 /* Unimplemented feature error */
#define PS_DISABLED_FEATURE_FAIL -11 /* Incorrect #define toggle for feature */
#define PS_PROTOCOL_FAIL	-12 /* A protocol error occurred */
#define PS_TIMEOUT_FAIL		-13 /* A timeout occurred and MAY be an error */
#define PS_INTERRUPT_FAIL	-14 /* An interrupt occurred and MAY be an error */
#define PS_PENDING			-15 /* In process. Not necessarily an error */
#define PS_EAGAIN			-16 /* Try again later. Not necessarily an error */

#define	PS_TRUE		1
#define	PS_FALSE 	0

#define CRTRSA 1
#define NORMALRSA 0

typedef uint32			pstm_digit;
typedef uint64			pstm_word;

/******************************************************************************/

//#define PSPUBLIC extern __declspec(dllimport)

/** Public or private key */
enum PACKED {
	PS_PUBKEY = 1,
	PS_PRIVKEY
};
typedef struct {
	pstm_digit	*dp;
	psPool_t	*pool;
#if defined (__GNUC__) || defined(__llvm__)
	/* Save a little space with compilers we know will handle this right */
	uint32_t	used : 12,
		alloc : 12,
		sign : 1;
#else
	uint16_t	used;
	uint16_t	alloc;
	uint8_t		sign;
#endif
} pstm_int;


typedef struct {
	pstm_int	e, d, N, qP, dP, dQ, p, q;
	psPool_t	*pool;
	uint16_t	size;   	/* Size of the key in bytes */
	uint8_t		optimized;	/* Set if optimized */
} psRsaKey_t;


int32_t psRsaCrypt(psPool_t *pool, psRsaKey_t *key,
	const unsigned char *in, uint16_t inlen,
	unsigned char *out, uint16_t *outlen,
	uint8_t type, void *data);