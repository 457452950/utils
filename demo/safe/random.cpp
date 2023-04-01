
#include "random.h"
#include "mpuint.h"
// #include <conio.h>

#include <random>

static int RandomKey(void) {
    srand(time(nullptr));
    return rand() & 0xFF;
}

void Random(mpuint &x) {
    for(unsigned i = 0; i < x.length; i++)
        x.value[i] = RandomKey() << 8 | RandomKey();
}
