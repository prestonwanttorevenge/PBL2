#ifndef _NODE_BLF_H_
#define _NODE_BLF_H_

#include <sys/types.h>

#ifdef __sun
#define u_int8_t uint8_t
#define u_int16_t uint16_t
#define u_int32_t uint32_t
#define u_int64_t uint64_t
#endif

#ifdef _WIN32
#define u_int8_t unsigned __int8
#define u_int16_t unsigned __int16
#define u_int32_t unsigned __int32
#define u_int64_t unsigned __int64
#endif

#if defined(_WIN32) || defined(_WIN64)
#  if defined(_WIN64)
     typedef __int64 LONG_PTR;
#  else
     typedef long LONG_PTR;
#  endif
  typedef LONG_PTR SSIZE_T;
  typedef SSIZE_T ssize_t;
#endif

#ifdef __MVS__
typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned int u_int32_t;
typedef unsigned long long u_int64_t;
#endif

#define BCRYPT_VERSION '2'
#define BCRYPT_MAXSALT 16
#define BCRYPT_BLOCKS 6
#define BCRYPT_MINROUNDS 16

#define BLF_N	16
#define BLF_MAXKEYLEN ((BLF_N-2)*4)
#define BLF_MAXUTILIZED ((BLF_N+2)*4)

#define _PASSWORD_LEN   128
#define _SALT_LEN       32

typedef struct BlowfishContext {
	u_int32_t S[4][256];
	u_int32_t P[BLF_N + 2];
} blf_ctx;

void Blowfish_encipher(blf_ctx *, u_int32_t *, u_int32_t *);
void Blowfish_decipher(blf_ctx *, u_int32_t *, u_int32_t *);
void Blowfish_initstate(blf_ctx *);
void Blowfish_expand0state(blf_ctx *, const u_int8_t *, u_int16_t);
void Blowfish_expandstate
(blf_ctx *, const u_int8_t *, u_int16_t, const u_int8_t *, u_int16_t);

void blf_key(blf_ctx *, const u_int8_t *, u_int16_t);
void blf_enc(blf_ctx *, u_int32_t *, u_int16_t);
void blf_dec(blf_ctx *, u_int32_t *, u_int16_t);

void blf_ecb_encrypt(blf_ctx *, u_int8_t *, u_int32_t);
void blf_ecb_decrypt(blf_ctx *, u_int8_t *, u_int32_t);

void blf_cbc_encrypt(blf_ctx *, u_int8_t *, u_int8_t *, u_int32_t);
void blf_cbc_decrypt(blf_ctx *, u_int8_t *, u_int8_t *, u_int32_t);

u_int32_t Blowfish_stream2word(const u_int8_t *, u_int16_t , u_int16_t *);

void bcrypt_gensalt(char, u_int8_t, u_int8_t*, char *);
void node_bcrypt(const char *, size_t key_len, const char *, char *);
void encode_salt(char *, u_int8_t *, char, u_int16_t, u_int8_t);
u_int32_t bcrypt_get_rounds(const char *);

#endif