//
// Created by shixinling on 2016/6/28.
//
#include <jni.h>
#include <fcntl.h>
#include <string.h>
#include <android/log.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "androidlog.h"
#include <time.h>
#include "tee_client_api.h"
#include "RSA.h"
#include "pstm.h"
#include "AES.h"



#define ERROR 1
#define INIT_CONTEXT_FAILED 0x0100
#define OPEN_SESSION_FAILED 0x0101

#define setDESKey 0x0001
#define setRSAKey 0x0002
#define setCRTRSAKey 0x0003
#define doDES 0x0004
#define doRSA 0x0005
#define doCRTRSA 0x0006

#define BIG_INT_LENGTH 256
#define BUFFER_SIZE 2048

char hostName[] = "tta";
TEEC_UUID uuid_ta = {0xC0CC32C8, 0x7D9C, 0x4BCA, {0x96, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09}};
TEEC_Context context;
TEEC_Session session;
uint32_t returnOrigin;


uint32_t test_sub()
{
   // a > b
    pstm_int a,b,c;
    pstm_digit b_1[32] = { 0xB5E25201, 0xBBFC144C, 0x9036FDF3, 0xE16875AC, 0x2DFE455F, 0x56BB1D97, 0x17D5AD8B, 0x77B50BBD, 0x0786F663, 0x524B5E42, 0x3149907A, 0x020382B9, 0x5C875E82, 0x8DC0DC9C, 0xE0547BA7, 0x3EEA2862, 0x9F396625, 0xAF2BE42B, 0xB28D77F7, 0x27E8F9F1, 0x04BD5CE8, 0x293EB5E0, 0x37699E12, 0x40E76643, 0x0F054EAC, 0xAC656E89, 0x1801FFD7, 0xFBC013EC, 0xA3A9E6B8, 0xE0A45538, 0x9580DDC5, 0x9BCB4589 };
    pstm_digit a_1[32] = { 0x84767599, 0xB6066B40, 0x08A59CFF, 0xBA0F943A, 0xABBFD755, 0xEA457DBA, 0xEE840BBC, 0x545FFB49, 0x343F1778, 0x0EA3ABA9, 0xBC4A518B, 0x6BC51282, 0x60AC28D0, 0x15D287AF, 0xB9EB965C, 0xFF2D717C, 0x58595C4E, 0x6E7DBE6B, 0xE91C84A2, 0xCDAA0C94, 0xCD0CCA13, 0xBBC59CEB, 0xDFBC8929, 0x0B60D355, 0x0F3B517A, 0x14C26A0B, 0xA78004B0, 0x8EC57C2A, 0x62344C0E, 0x44A68C0C, 0x9F278929, 0x9DD73066 };
    pstm_digit c_1[32] = { 0 };
    uint32_t time;
 //   struct timeval tv_begin, tv_end;
    struct timespec time1, time2;

    a.dp = a_1;
    a.alloc = 32;
    a.used = 32;
    a.sign = 0;

    b.dp = b_1;
    b.alloc = 32;
    b.used = 32;
    b.sign = 0;

    c.dp = c_1;
    c.alloc = 32;
    c.used = 0;
    c.sign = 0;

    clock_gettime(CLOCK_THREAD_CPUTIME_ID,&time1);
    pstm_sub(&a, &b, &c);
    clock_gettime(CLOCK_THREAD_CPUTIME_ID,&time2);
  //  gettimeofday(&tv_begin, NULL);

  //  gettimeofday(&tv_end, NULL);
  //  time = tv_end.tv_usec - tv_begin.tv_usec + (tv_end.tv_sec - tv_begin.tv_sec)*1000000;
    time = time2.tv_nsec - time1.tv_nsec + (time2.tv_sec - time1.tv_sec) * 1000000000;
    return time;
}
/*
 void nop(void)
{
    asm(
   "NOP\n\t"
   );
 }
 */


uint32_t RSA_s(unsigned char *in,unsigned char *out)
{
	pstm_digit e[] = { 0x10001 };
	pstm_digit d[] = { 0xB5E25201, 0xBBFC144C, 0x9036FDF3, 0xE16875AC, 0x2DFE455F, 0x56BB1D97, 0x17D5AD8B, 0x77B50BBD, 0x0786F663, 0x524B5E42, 0x3149907A, 0x020382B9, 0x5C875E82, 0x8DC0DC9C, 0xE0547BA7, 0x3EEA2862, 0x9F396625, 0xAF2BE42B, 0xB28D77F7, 0x27E8F9F1, 0x04BD5CE8, 0x293EB5E0, 0x37699E12, 0x40E76643, 0x0F054EAC, 0xAC656E89, 0x1801FFD7, 0xFBC013EC, 0xA3A9E6B8, 0xE0A45538, 0x9580DDC5, 0x0BCB4589 };
	pstm_digit dP[] = { 0x1E233541, 0x1758FE0E, 0xD9D32F3D, 0x9A6FD5E7, 0x38F49258, 0xF2CADEAC, 0x50BCA1DE, 0xCCC566E0, 0x81E36AF8, 0xF00B8EED, 0x3CFAA2D9, 0xE5D00AFE, 0xFDC13C4D, 0xBF3F5B4E, 0x0731EDEB, 0x3D25ADCD };
	pstm_digit dQ[] = { 0xAE02CCB9, 0xB9E92568, 0x40357C0F, 0xB2A33ABB, 0x76F43F02, 0xAA60D1BD, 0x026B8EBF, 0x3A8F0FC5, 0xEB6592F7, 0x560DF43E, 0x08245F33, 0xC5091A5B, 0xF9341203, 0xEF236387, 0x4799F5A9, 0x5A2479BF };
	pstm_digit qP[] = { 0x625D775B, 0xA762F183, 0x526CB928, 0xAD4913AD, 0x2230FE4F, 0x48B55F02, 0x24D76078, 0xFB25AC5C, 0xA4154B15, 0x66FE4D80, 0x1D16FCD3, 0x2894639D, 0x800E16C4, 0xC16EE593, 0x801F08D5, 0x0EB41A42 };
	pstm_digit N[] = { 0x84767599, 0xB6066B40, 0x08A59CFF, 0xBA0F943A, 0xABBFD755, 0xEA457DBA, 0xEE840BBC, 0x545FFB49, 0x343F1778, 0x0EA3ABA9, 0xBC4A518B, 0x6BC51282, 0x60AC28D0, 0x15D287AF, 0xB9EB965C, 0xFF2D717C, 0x58595C4E, 0x6E7DBE6B, 0xE91C84A2, 0xCDAA0C94, 0xCD0CCA13, 0xBBC59CEB, 0xDFBC8929, 0x0B60D355, 0x0F3B517A, 0x14C26A0B, 0xA78004B0, 0x8EC57C2A, 0x62344C0E, 0x44A68C0C, 0x9F278929, 0x9DD73066 };
	pstm_digit p[] = { 0x85C20AC1, 0x61DF74FE, 0x260E4816, 0x2583B46B, 0x65596535, 0xB8DFB610, 0x953BD67F, 0x12A15D1B, 0x5F676AB7, 0x755C0122, 0x52D0E37C, 0x8F317E4A, 0xA9696ED5, 0x6CED5391, 0xED4E1CBE, 0xD0BAD286 };
	pstm_digit q[] = { 0x2E8958D9, 0x5963BEC0, 0x6765D115, 0x72C1F9FF, 0xB32A341E, 0x25D214B4, 0xE1FF85F4, 0xDBD47FA4, 0x47398AA5, 0xD55CC1D2, 0xF5039C2B, 0x080B33C8, 0x787019D6, 0xB1C2BBEB, 0xE0333FCB, 0xC1960C87 };

	pstm_int key_e, key_d, key_N, key_qP, key_dP, key_dQ, key_p, key_q;
	psPool_t pool[4096] = { 0 };
	psRsaKey_t RSAkey = { 0 };

    struct timespec time1, time2;

	uint16_t inlen = 128;
	uint16_t outlen = 128;
	void *data = NULL;
	psRsaKey_t *key = &RSAkey;
	key_e.dp = e;
	key_e.alloc = 0;
	key_e.used = 1;
	key_e.sign = 0;

	key_d.dp = d;
	key_d.alloc = 0;
	key_d.used = 32;
	key_d.sign = 0;

	key_N.dp = N;
	key_N.alloc = 0;
	key_N.used = 32;
	key_N.sign = 0;

	key_qP.dp = qP;
	key_qP.alloc = 0;
	key_qP.used = 16;
	key_qP.sign = 0;

	key_dP.dp = dP;
	key_dP.alloc = 0;
	key_dP.used = 16;
	key_dP.sign = 0;

	key_dQ.dp = dQ;
	key_dQ.alloc = 0;
	key_dQ.used = 16;
	key_dQ.sign = 0;

	key_p.dp = p;
	key_p.alloc = 0;
	key_p.used = 16;
	key_p.sign = 0;

	key_q.dp = q;
	key_q.alloc = 0;
	key_q.used = 16;
	key_q.sign = 0;

	key->size = 128;
	key->e = key_e;
	key->d = key_d;
	key->dP = key_dP;
	key->dQ = key_dQ;
	key->N = key_N;
	key->p = key_p;
	key->q = key_q;
	key->qP = key_qP;
	psPool_t keypool[8192] = { 0 };
	key->pool = keypool;
	//key->optimized = NORMALRSA;
    key->optimized = CRTRSA;
	uint32_t result = 0;

    clock_gettime(CLOCK_THREAD_CPUTIME_ID,&time1);


    nop();

    psRsaCrypt(pool, key, in, inlen, out, &outlen,PS_PRIVKEY, data);

    nop();

    clock_gettime(CLOCK_THREAD_CPUTIME_ID,&time2);

	result = time2.tv_nsec - time1.tv_nsec + (time2.tv_sec - time1.tv_sec) * 1000000000;

	return result;
}

unsigned char* decodeJavaByteArray(JNIEnv* env, jbyteArray data, int* byteArrayLength)
{
	unsigned char *buffer = NULL;
	int dataLen = 0;
	dataLen = (*env)->GetArrayLength(env, data);
	buffer = (unsigned char*)malloc(dataLen);
	(*env)->GetByteArrayRegion(env, data, 0, dataLen, buffer);
	return buffer;
}
jbyteArray encodeJavaByteArray(JNIEnv* env, unsigned char *data, int dataLength)
{
	jbyteArray resultByteArray = {0};
	resultByteArray = (*env)->NewByteArray(env, dataLength);
	(*env)->SetByteArrayRegion(env, resultByteArray, 0, dataLength, data);
	return resultByteArray;
}
/*
* Class:     com_dplslab_shixl_dplstee_test_TEENative
* Method:    openTEE
* Signature: ()I
*/
jint JNICALL Java_com_dplslab_shixl_dplstee_1test_TEENative_openTEE(JNIEnv * env, jobject thiz)
{
/*
    int returnValue = 0;
    TEEC_Result result;
    // 连接指定的TEE
    result = TEEC_InitializeContext(hostName, &context);

    if (result != TEEC_SUCCESS)
    {
        return result;
    }


    // 连接TEE中某个具体的TA，并打开会话
    result = TEEC_OpenSession(&context, &session, &uuid_ta, TEEC_LOGIN_PUBLIC, NULL, NULL, &returnOrigin);

    if (result != TEEC_SUCCESS)
    {
        TEEC_FinalizeContext(&context);
        return result;
    }

    return returnValue;
    */
    return 0;
}

/*
 * Class:     com_dplslab_shixl_dplstee_test_TEENative
 * Method:    closeTEE
 * Signature: ()I
 */
jint JNICALL Java_com_dplslab_shixl_dplstee_1test_TEENative_closeTEE(JNIEnv * env, jobject thiz)
{
    TEEC_CloseSession(&session);
    TEEC_FinalizeContext(&context);
    return 0;
}
//keyType==1 des
//keyType==2 rsa 按照E、D、N排布
//keyType==3 crt-rsa
jbyteArray doSetKey(JNIEnv* env,jbyteArray key,int keyType)
{
    TEEC_Operation operation;
    TEEC_Result result;
    TEEC_SharedMemory mykey;
    int keyLength;
    uint32_t commandID;
    unsigned char *keyB;

    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_PARTIAL_INPUT, TEEC_VALUE_INOUT, TEEC_VALUE_INOUT, TEEC_VALUE_INOUT);
    operation.started = 1;

    if(keyType==1){
        keyLength=8;
        commandID=setDESKey;
    }else if(keyType==2){
        //2048bit*3
        keyLength=768;
        commandID=setRSAKey;
    }else{
        keyLength=1408;
        commandID=setCRTRSAKey;
    }

    keyB=decodeJavaByteArray(env, key, &keyLength);
    mykey.size=keyLength;
    mykey.flags=TEEC_MEM_INPUT;
    TEEC_AllocateSharedMemory(&context,&mykey);

    memcpy(mykey.buffer,keyB,keyLength);
    operation.params[0].memref.parent = &mykey;
    operation.params[0].memref.offset = 0;
    operation.params[0].memref.size = keyLength;

    result = TEEC_InvokeCommand(&session, commandID, &operation, NULL);

    TEEC_ReleaseSharedMemory(&mykey);
    if(result!=TEEC_SUCCESS){
         return (*env)->NewByteArray(env, 99);
    }
    return (*env)->NewByteArray(env, 1);
}
//keyType==1 des
//keyType==2 rsa 按照E、D、N排布
//keyType==3 crt-rsa
jbyteArray doAlg(JNIEnv* env,jbyteArray input,int keyType){
    TEEC_Operation operation;
    TEEC_Result result;
    TEEC_SharedMemory myinput;
    TEEC_SharedMemory myoutput;
    int inputLength;
    uint32_t commandID;
    unsigned char *inputB;

    uint32_t *time;
    struct timeval tv_begin, tv_end;
  //  psDes3_t *ctx = (psDes3_t *)malloc(sizeof(psDes3_t));
  //  psDes3Clear(psDes3_t *ctx);

    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_PARTIAL_INPUT, TEEC_MEMREF_PARTIAL_OUTPUT, TEEC_VALUE_INOUT, TEEC_VALUE_INOUT);
    operation.started = 1;

    if(keyType==1){
        inputLength=16;
        commandID=doDES;
    }else if(keyType==2){
        //2048bit*3
        inputLength=128;
        commandID=doRSA;
    }else{
        inputLength=128;
        commandID=doCRTRSA;
    }

 //   char output[132] = {0};
  //   inputB = decodeJavaByteArray(env, input, &inputLength);
    char output[20] = {0};

    unsigned char key[16] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
//    unsigned char inaes[16] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
//    unsigned char outaes[16] = {0};

  /*  inputB = decodeJavaByteArray(env, input, &inputLength);

    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();

    aes_ecb_encrypt(key,inputB,output+4);
    nop();
    nop();
    nop();
    nop();
    nop();
    aes_ecb_encrypt(key,inputB,output+4);
    nop();
    nop();
    nop();
    nop();
    nop();

    aes_ecb_encrypt(key,inputB,output+4);
    nop();
    nop();
    nop();
    nop();
    nop();

    aes_ecb_encrypt(key,inputB,output+4);
    nop();
    nop();
    nop();
    nop();
    nop();

    aes_ecb_encrypt(key,inputB,output+4);
    nop();
    nop();
    nop();
    nop();
    nop();
    aes_ecb_encrypt(key,inputB,output+4);


    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
*/
    aes_ecb_encrypt(key,inputB,output+4,0);
    aes_ecb_encrypt(key,inputB,output+4,0);
    aes_ecb_encrypt(key,inputB,output+4,0);
    aes_ecb_encrypt(key,inputB,output+4,0);
    aes_ecb_encrypt(key,inputB,output+4,0);
    aes_ecb_encrypt(key,inputB,output+4,0);
    aes_ecb_encrypt(key,inputB,output+4,0);
    aes_ecb_encrypt(key,inputB,output+4,0);
    aes_ecb_encrypt(key,inputB,output+4,0);
    aes_ecb_encrypt(key,inputB,output+4,0);







 //   time=(uint32_t *)output;
 //   *time = test_sub();
 //   *time = RSA_s(inputB,output+4);

    jbyteArray bresult = encodeJavaByteArray( env, output, inputLength+4);
  //  bresult = encodeJavaByteArray( env, inputB, inputLength);
    return bresult;
/*
    myinput.size=inputLength;
    myinput.flags=TEEC_MEM_INPUT;
    TEEC_AllocateSharedMemory(&context,&myinput);
    memcpy(myinput.buffer,inputB,inputLength);
    operation.params[0].memref.parent = &myinput;
    operation.params[0].memref.offset = 0;
    operation.params[0].memref.size = inputLength;

    myoutput.size=inputLength+4;
    myoutput.flags=TEEC_MEM_OUTPUT;
    TEEC_AllocateSharedMemory(&context,&myoutput);
    operation.params[1].memref.parent = &myoutput;
    operation.params[1].memref.offset = 0;
    operation.params[1].memref.size = inputLength+4;

  //  gettimeofday(&tv_begin, NULL);
    result = TEEC_InvokeCommand(&session, commandID, &operation, NULL);
  //  gettimeofday(&tv_end, NULL);
  //  memcpy(operation.params[1].memref.parent->buffer,output,operation.params[1].memref.parent->size);
   // time=(uint32_t *)output;
  //  *time = tv_end.tv_usec - tv_begin.tv_usec + (tv_end.tv_sec - tv_begin.tv_sec)*1000000;

    jbyteArray bresult=encodeJavaByteArray(env, operation.params[1].memref.parent->buffer, operation.params[1].memref.parent->size);
    TEEC_ReleaseSharedMemory(&myinput);
    TEEC_ReleaseSharedMemory(&myoutput);
    if(result!=TEEC_SUCCESS){
        //长度99，表示失败
        return (*env)->NewByteArray(env, 99);
    }
    return bresult;
   */
}

/*
* Class:     com_dplslab_shixl_dplstee_test_TEENative
* Method:    doAction
* Signature: (IIII[B)[B
*/
//jbyteArray JNICALL Java_com_dplslab_shixl_dplstee_1test_TEENative_doAction(JNIEnv * env, jobject thiz, jint algType, jint para1, jint para2, jint para3, jbyteArray para4)
jbyteArray doAction(int algType, int para1, int para2, int para3, unsigned char* para4)
{
    unsigned char key[16] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
    unsigned char inaes[16] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
    unsigned char outaes[16] = {0};

    aes_ecb_encrypt(key,inaes,outaes,0);

/*
    if(algType==0){
        //des算法
        if(para1==0){
            //set key
            return doSetKey(env,para4,1);
        }
        if(para1==1){
            return doAlg(env,para4,1);
        }
    }
    if(algType==1){
        //rsa算法
        if(para1==0){
            //set key
            return doSetKey(env,para4,2);
        }
        if(para1==1){
            return doAlg(env,para4,2);
        }
    }
    if(algType==2){
        //rsa算法
        if(para1==0){
            //set key
            return doSetKey(env,para4,3);
        }
        if(para1==1){
            return doAlg(env,para4,3);
        }
    }
    */
    //jbyteArray bresult = encodeJavaByteArray( env, outaes, 16);
    return outaes;
}
