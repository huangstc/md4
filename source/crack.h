
#ifndef CRACK_H_
#define CRACK_H_

#include "md4.h"

struct CrackContext {
  INT32 m1[16], m2[16];
  INT32 r1[4][13], r2[4][13];
  INT32 h1[4], h2[4];
};

struct DiffCondition {
  int step;
  int rstep; // require by rstep, not important, just a note
  int type;  // 0, 1, 2(=), 3(+1)
  int bit;   // the bit position
  int ref1;   // only required by type 2 and 3
  int ref2;	// only required by type 2 and 3
};

void select_md4(CrackContext& ctx, int step);
void dump(CrackContext& ctx, std::ostream& os = std::cout);
void run(int seed, int iterations);

// generate a 512-bit random message.
void rand_message(INT32* m);

// ds = s1 - s2;
void minus(INT32* ds, const INT32* s1, const INT32* s2, size_t len);

// sum = s1 + s2;
void add(INT32* sum, const INT32* s1, const INT32* s2, size_t len);

#endif  // CRACK_H_
