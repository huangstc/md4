
#ifndef MD4_H_
#define MD4_H_

#include <cstdint>
#include <iostream>

typedef uint32_t INT32;

#define PH1(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define PH2(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define PH3(x, y, z) ((x) ^ (y) ^ (z))

/* Circular shift
 */
#define LRT(s, x) (((x) << (s)) | ((x) >> (32-(s))))
#define RRT(s, x) (((x) >> (s)) | ((x) << (32-(s))))

#define OP1(na, a, b, c, d, m, s) (na = LRT(s, a+PH1(b,c,d)+m))
#define OP2(na, a, b, c, d, m, s) (na = LRT(s, a+PH2(b,c,d)+m+0x5A827999))
#define OP3(na, a, b, c, d, m, s) (na = LRT(s, a+PH3(b,c,d)+m+0x6ED9EBA1))

/**
 *   ROUND 1
 */
#define	STEP01(a,b,c,d,m) OP1(a[1], a[0], b[0], c[0], d[0], m[0], 3);
#define	STEP02(a,b,c,d,m) OP1(d[1], d[0], a[1], b[0], c[0], m[1], 7);
#define	STEP03(a,b,c,d,m) OP1(c[1], c[0], d[1], a[1], b[0], m[2], 11);
#define	STEP04(a,b,c,d,m) OP1(b[1], b[0], c[1], d[1], a[1], m[3], 19);

#define	STEP05(a,b,c,d,m) OP1(a[2], a[1], b[1], c[1], d[1], m[4], 3);
#define	STEP06(a,b,c,d,m) OP1(d[2], d[1], a[2], b[1], c[1], m[5], 7);
#define	STEP07(a,b,c,d,m) OP1(c[2], c[1], d[2], a[2], b[1], m[6], 11);
#define	STEP08(a,b,c,d,m) OP1(b[2], b[1], c[2], d[2], a[2], m[7], 19);

#define	STEP09(a,b,c,d,m) OP1(a[3], a[2], b[2], c[2], d[2], m[8], 3);
#define	STEP10(a,b,c,d,m) OP1(d[3], d[2], a[3], b[2], c[2], m[9], 7);
#define	STEP11(a,b,c,d,m) OP1(c[3], c[2], d[3], a[3], b[2], m[10], 11);
#define	STEP12(a,b,c,d,m) OP1(b[3], b[2], c[3], d[3], a[3], m[11], 19);

#define	STEP13(a,b,c,d,m) OP1(a[4], a[3], b[3], c[3], d[3], m[12], 3);
#define	STEP14(a,b,c,d,m) OP1(d[4], d[3], a[4], b[3], c[3], m[13], 7);
#define	STEP15(a,b,c,d,m) OP1(c[4], c[3], d[4], a[4], b[3], m[14], 11);
#define	STEP16(a,b,c,d,m) OP1(b[4], b[3], c[4], d[4], a[4], m[15], 19);

/**
 *   ROUND 2
 */
#define	STEP17(a,b,c,d,m) OP2(a[5], a[4], b[4], c[4], d[4], m[0], 3);
#define	STEP18(a,b,c,d,m) OP2(d[5], d[4], a[5], b[4], c[4], m[4], 5);
#define	STEP19(a,b,c,d,m) OP2(c[5], c[4], d[5], a[5], b[4], m[8], 9);
#define	STEP20(a,b,c,d,m) OP2(b[5], b[4], c[5], d[5], a[5], m[12], 13);

#define	STEP21(a,b,c,d,m) OP2(a[6], a[5], b[5], c[5], d[5], m[1], 3);
#define	STEP22(a,b,c,d,m) OP2(d[6], d[5], a[6], b[5], c[5], m[5], 5);
#define	STEP23(a,b,c,d,m) OP2(c[6], c[5], d[6], a[6], b[5], m[9], 9);
#define	STEP24(a,b,c,d,m) OP2(b[6], b[5], c[6], d[6], a[6], m[13], 13);

#define	STEP25(a,b,c,d,m) OP2(a[7], a[6], b[6], c[6], d[6], m[2], 3);
#define	STEP26(a,b,c,d,m) OP2(d[7], d[6], a[7], b[6], c[6], m[6], 5);
#define	STEP27(a,b,c,d,m) OP2(c[7], c[6], d[7], a[7], b[6], m[10], 9);
#define	STEP28(a,b,c,d,m) OP2(b[7], b[6], c[7], d[7], a[7], m[14], 13);

#define	STEP29(a,b,c,d,m) OP2(a[8], a[7], b[7], c[7], d[7], m[3], 3);
#define	STEP30(a,b,c,d,m) OP2(d[8], d[7], a[8], b[7], c[7], m[7], 5);
#define	STEP31(a,b,c,d,m) OP2(c[8], c[7], d[8], a[8], b[7], m[11], 9);
#define	STEP32(a,b,c,d,m) OP2(b[8], b[7], c[8], d[8], a[8], m[15], 13);

/**
 *   ROUND 3
 */
#define	STEP33(a,b,c,d,m) OP3(a[9], a[8], b[8], c[8], d[8], m[0], 3);
#define	STEP34(a,b,c,d,m) OP3(d[9], d[8], a[9], b[8], c[8], m[8], 9);
#define	STEP35(a,b,c,d,m) OP3(c[9], c[8], d[9], a[9], b[8], m[4], 11);
#define	STEP36(a,b,c,d,m) OP3(b[9], b[8], c[9], d[9], a[9], m[12], 15);

#define	STEP37(a,b,c,d,m) OP3(a[10], a[9], b[9], c[9], d[9], m[2], 3);
#define	STEP38(a,b,c,d,m) OP3(d[10], d[9], a[10], b[9], c[9], m[10], 9);
#define	STEP39(a,b,c,d,m) OP3(c[10], c[9], d[10], a[10], b[9], m[6], 11);
#define	STEP40(a,b,c,d,m) OP3(b[10], b[9], c[10], d[10], a[10], m[14], 15);

#define	STEP41(a,b,c,d,m) OP3(a[11], a[10], b[10], c[10], d[10], m[1], 3);
#define	STEP42(a,b,c,d,m) OP3(d[11], d[10], a[11], b[10], c[10], m[9], 9);
#define	STEP43(a,b,c,d,m) OP3(c[11], c[10], d[11], a[11], b[10], m[5], 11);
#define	STEP44(a,b,c,d,m) OP3(b[11], b[10], c[11], d[11], a[11], m[13], 15);

#define	STEP45(a,b,c,d,m) OP3(a[12], a[11], b[11], c[11], d[11], m[3], 3);
#define	STEP46(a,b,c,d,m) OP3(d[12], d[11], a[12], b[11], c[11], m[11], 9);
#define	STEP47(a,b,c,d,m) OP3(c[12], c[11], d[12], a[12], b[11], m[7], 11);
#define	STEP48(a,b,c,d,m) OP3(b[12], b[11], c[12], d[12], a[12], m[15], 15);

// Hash a message with a custom initial value.
void md4(const INT32* message,
         const INT32* iv  /* initial value, a b c d*/,
         INT32* hash,    /* hash result*/
         INT32* temp_a = nullptr,
         INT32* temp_b = nullptr,
         INT32* temp_c = nullptr,
         INT32* temp_d = nullptr);

// Print a message.
void dump(const INT32* str, size_t len, std::ostream& os = std::cout);

extern const INT32 INITIAL_VALUE[4];

#endif  // MD4_H_
