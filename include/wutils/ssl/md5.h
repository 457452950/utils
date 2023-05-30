#ifndef MD5_H
#define MD5_H

#include <openssl/md5.h>

#include "wutils/ByteArray.h"

namespace wutils {

ByteArray MD5(ByteArrayView str);

}

#endif // MD5_H
