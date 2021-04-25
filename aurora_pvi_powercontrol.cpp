#include <stdio.h>
#include <inttypes.h>

//#include "aurora/comm.c"

/*--------------------------------------------------------------------------
    crc16
                                         16   12   5
    this is the CCITT CRC 16 polynomial X  + X  + X  + 1.
    This is 0x1021 when x is 2, but the way the algorithm works
    we use 0x8408 (the reverse of the bit pattern).  The high
    bit is always assumed to be set, thus we only use 16 bits to
    represent the 17 bit value.
----------------------------------------------------------------------------*/
uint16_t crc16_CCITT(char *data_p, unsigned short length) {
    constexpr uint16_t poly = 0x8408; /* 1021H bit reversed */
    unsigned char i;
    unsigned int data;
    unsigned int crc = 0xffff;
    if (length == 0)
        return ~crc;
    do {
        for (i = 0, data = (unsigned int)0xff & *data_p++;
             i < 8;
             i++, data >>= 1)
        {
            if ((crc & 0x0001) ^ (data & 0x0001)) {
                crc = (crc >> 1) ^ poly;
            } else {
                crc >>= 1;
            }
        }
    } while (--length);
    crc = ~crc;
    return crc;
}


class AuroraPVI {
public:
    AuroraPVI() {
    }
};

int main() {
    printf("Foo");
}