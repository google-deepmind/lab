#include <stddef.h>
#include <stdint.h>

#include "third_party/md/md4.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  unsigned char digest[16];
  MD4_CTX ctx;

  MD4Init(&ctx);
  MD4Update(&ctx, data, size);
  MD4Final(digest, &ctx);

  return 0;
}
