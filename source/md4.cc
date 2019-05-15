#include "md4.h"

#include <cstring>
#include <iostream>

const INT32 INITIAL_VALUE[4] = {
    0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476
};

void md4(const INT32* m, const INT32* iv, INT32* hash,
         INT32* tmp_a, INT32* tmp_b, INT32* tmp_c, INT32* tmp_d) {
  INT32 a[13];	memset(a, 0, sizeof(a));
  INT32 b[13];	memset(b, 0, sizeof(b));
  INT32 c[13];	memset(c, 0, sizeof(c));
  INT32 d[13];	memset(d, 0, sizeof(d));

  a[0] = iv[0];
  b[0] = iv[1];
  c[0] = iv[2];
  d[0] = iv[3];

  /**
   *   ROUND 1
   */
  STEP01(a,b,c,d,m);
  STEP02(a,b,c,d,m);
  STEP03(a,b,c,d,m);
  STEP04(a,b,c,d,m);

  STEP05(a,b,c,d,m);
  STEP06(a,b,c,d,m);
  STEP07(a,b,c,d,m);
  STEP08(a,b,c,d,m);

  STEP09(a,b,c,d,m);
  STEP10(a,b,c,d,m);
  STEP11(a,b,c,d,m);
  STEP12(a,b,c,d,m);

  STEP13(a,b,c,d,m);
  STEP14(a,b,c,d,m);
  STEP15(a,b,c,d,m);
  STEP16(a,b,c,d,m);

  /**
   *   ROUND 2
   */
  STEP17(a,b,c,d,m);
  STEP18(a,b,c,d,m);
  STEP19(a,b,c,d,m);
  STEP20(a,b,c,d,m);

  STEP21(a,b,c,d,m);
  STEP22(a,b,c,d,m);
  STEP23(a,b,c,d,m);
  STEP24(a,b,c,d,m);

  STEP25(a,b,c,d,m);
  STEP26(a,b,c,d,m);
  STEP27(a,b,c,d,m);
  STEP28(a,b,c,d,m);

  STEP29(a,b,c,d,m);
  STEP30(a,b,c,d,m);
  STEP31(a,b,c,d,m);
  STEP32(a,b,c,d,m);

  /**
   *   ROUND 3
   */
  STEP33(a,b,c,d,m);
  STEP34(a,b,c,d,m);
  STEP35(a,b,c,d,m);
  STEP36(a,b,c,d,m);

  STEP37(a,b,c,d,m);
  STEP38(a,b,c,d,m);
  STEP39(a,b,c,d,m);
  STEP40(a,b,c,d,m);

  STEP41(a,b,c,d,m);
  STEP42(a,b,c,d,m);
  STEP43(a,b,c,d,m);
  STEP44(a,b,c,d,m);

  STEP45(a,b,c,d,m);
  STEP46(a,b,c,d,m);
  STEP47(a,b,c,d,m);
  STEP48(a,b,c,d,m);

  hash[0] = a[12] + a[0];
  hash[1] = b[12] + b[0];
  hash[2] = c[12] + c[0];
  hash[3] = d[12] + d[0];

  if (tmp_a!=NULL) {
    memcpy(tmp_a, a, sizeof(a));
  }

  if (tmp_b!=NULL) {
    memcpy(tmp_b, b, sizeof(a));
  }

  if (tmp_c!=NULL) {
    memcpy(tmp_c, c, sizeof(a));
  }

  if (tmp_d!=NULL) {
    memcpy(tmp_d, d, sizeof(a));
  }
}

void dump(const INT32* str, size_t len, std::ostream& os) {
  os.setf(std::ios_base::hex, std::ios_base::basefield);
  os.width(10);

  for (size_t i=0; i<len; ) {
    os << str[i++];
    os.width(10);
  }

  os << "\n";
}
