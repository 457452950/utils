#include "wutils/ssl/md5.h"

namespace wutils {

ByteArray MD5(ByteArrayView str) {
    uint8_t out[MD5_DIGEST_LENGTH];

    ::MD5(str.data(), str.size(), out);

    return ByteArray(out, MD5_DIGEST_LENGTH);
}

} // namespace wutils
