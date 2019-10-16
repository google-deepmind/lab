#ifndef THIRD_PARTY_MD5_MD5_H_
#define THIRD_PARTY_MD5_MD5_H_

/* MD5.H - header file for MD5C.C */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
   rights reserved.

   License to copy and use this software is granted provided that it
   is identified as the "RSA Data Security, Inc. MD5 Message-Digest
   Algorithm" in all material mentioning or referencing this software
   or this function.

   License is also granted to make and use derivative works provided
   that such works are identified as "derived from the RSA Data
   Security, Inc. MD5 Message-Digest Algorithm" in all material
   mentioning or referencing the derived work.

   RSA Data Security, Inc. makes no representations concerning either
   the merchantability of this software or the suitability of this
   software for any particular purpose. It is provided "as is"
   without express or implied warranty of any kind.

   These notices must be retained in any copies of any part of this
   documentation and/or software.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "third_party/md/global.h"

/* MD5 context. */
typedef struct {
  UINT4 state[4];                                   /* state (ABCD) */
  UINT4 count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} MD5_CTX;

void MD5Init PROTO_LIST((MD5_CTX *));

void MD5Update PROTO_LIST((MD5_CTX *, const unsigned char *, unsigned int));

void MD5Final PROTO_LIST((unsigned char [16], MD5_CTX *));

/* Compatibility shim for Aladdin Enterprises's libmd5-rfc. */
typedef unsigned char md5_byte_t;
typedef uint32_t md5_word_t;
typedef MD5_CTX md5_state_t;

static inline void md5_init(md5_state_t *pms) {
  MD5Init(pms);
}

static inline void md5_append(md5_state_t *pms, const md5_byte_t *data, int nbytes) {
  MD5Update(pms, (unsigned char *) data, nbytes);
}

static inline void md5_finish(md5_state_t *pms, md5_byte_t digest[16]) {
  MD5Final(digest, pms);
}

#ifdef __cplusplus
}
#endif

#endif  // THIRD_PARTY_MD5_MD5_H_
