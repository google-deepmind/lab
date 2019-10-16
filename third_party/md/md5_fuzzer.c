#include <stddef.h>
#include <stdint.h>

#include "third_party/md/md5.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  unsigned char digest[16];
  MD5_CTX ctx;

  MD5Init(&ctx);
  MD5Update(&ctx, data, size);
  MD5Final(digest, &ctx);

  return 0;
}
