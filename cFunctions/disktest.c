#include <stdint.h>
#include <stdio.h>

#define SECTOR_SIZE 128

int main() {
    uint32_t sector0[SECTOR_SIZE];
    uint32_t sector1[SECTOR_SIZE];
    uint32_t sector2[SECTOR_SIZE];
    uint32_t sector3[SECTOR_SIZE];
    uint32_t sector4[SECTOR_SIZE];

    // (In practice these would be loaded from disk via DMA, but here we assume
    //  they’re already in memory.)

    for (int i = 0; i < SECTOR_SIZE; i++) {
        sector4[i] = sector0[i]
                   + sector1[i]
                   + sector2[i]
                   + sector3[i];
    }

    // (Then you’d write sector4[] back to disk sector 4 via DMA.)
    return 0;
}
