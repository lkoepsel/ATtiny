//  prng - pseudorandom number generator, based on Xorshift family of prngs
//  https://stackoverflow.com/questions/53886131/how-does-xorshift32-works

#include <inttypes.h>

int main(void)
{
    volatile uint64_t state = 0xACE1;
    state ^= state >> 12;
    state ^= state << 25;
    state ^= state >> 27;
    state = state * UINT64_C(2685821657736338717);

}
