#ifndef BITS_H
#define BITS_H

#define ANY_SET(id,set) (set)[(id)>>3] |= 1<<((id)&7)
#define ANY_CLR(id,set) (set)[(id)>>3] &= ~(1<<((id)&7))
#define ANY_CHK(id,set) ((set)[(id)>>3] & (1<<((id)&7)))
#define ANY_RESET(set,size) memset(set,0,size)
#define ANY_FILL(set,size) memset(set,0xFF,size)

#define BYTES_USED(x) (x/8+(x%8==0?0:1))

#endif
