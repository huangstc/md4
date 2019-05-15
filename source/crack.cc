#include "crack.h"

#include <cstring>
#include <iostream>

#include "md4.h"

// Util functions.
void rand_message(INT32* m) {
  for (int i = 0; i < 16; i++) {
    m[i] = (rand() << 12 ^ rand() << 24 ^ rand());
  }
}

void minus(INT32* ds, const INT32* s1, const INT32* s2, size_t len) {
  for (size_t i = 0; i < len; i++) {
    ds[i] = s1[i] - s2[i];
  }
}

void add(INT32* sum, const INT32* s1, const INT32* s2, size_t len) {
  for (size_t i = 0; i < len; i++) {
    sum[i] = s1[i] + s2[i];
  }
}

//	Global Constants
const INT32 g_dm[16] = {
    0x00000000, 0x80000000, 0x70000000, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    static_cast<INT32>(-0x00010000), 0, 0, 0
};

const INT32 g_diff[4][6] = {
    {0, 0, 0, 0x10000, 0x2400000, 0xFF720000},
    {0, 0x2000000, 0x1000, 0x80000000, 0x40000, 0},
    {0, 0x380, 0x1c0000, static_cast<INT32>(-0x20000000), 0, 0},
    {0, 0x40, 0x2000, 0xFE180000, 0xc000000, 0}
};

const INT32 g_mes_index[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 4, 8, 12, 1, 5
};

const INT32 g_rotation[2][4] = {{3, 7, 11, 19}, { 3, 5, 9, 13}};

//  Macros and Inline Functions
#define CMDLOG(str) do {       \
    std::cout << str << "\n";  \
  } while(0)

#define TEST_FAILED(i) do {                              \
    std::cout << "failed at condition " << i <<  "\n";   \
  } while(0)

#define OUTPUT_FAILED(i) do {                             \
    std::cout << "output failed at step " << i <<  "\n";  \
  } while(0)

// pick the s-th bit of x, from 0 - 31.
#define PICKBIT(x, s) (((x)>>(s)) & 0x0001)

#define FLIP_BIT(x, s) ((x)^=(0x01 << (s)))

inline INT32 REG(INT32 step) {
  return (101 - step) % 4;
}

inline INT32 OFFSET(INT32 step) {
  return (step-1) / 4 + 1;
}

inline void updatem2(CrackContext& cxx, INT32 pos) {
  cxx.m2[pos] = cxx.m1[pos] + g_dm[pos];
}

inline bool success(CrackContext& cxx) {
  return (cxx.h1[0]==cxx.h2[0]) && (cxx.h1[1]==cxx.h2[1]) &&
         (cxx.h1[2]==cxx.h2[2]) && (cxx.h1[3]==cxx.h2[3]);
}

//////////////////////////////////////////////////////
//  Differential Conditions, Very Important Parameters
DiffCondition g_first_conds[] = {
// step	  rstep	type bit ref1 ref2
  {1,  3,  2,  6,  1,    0},      // No.0
  {2,  4,  2,  7,  0,    1},      // d_{1,7} = a_{1,7}, for Step 4
  {2,  4,  2,  10, 0,    1},      // d_{1,10} = a_{1,10}, for Step 4
  {2,  2,  0,  6,  0xff, 0xff},   // d_{1,6} = 0
  {3,  5,  2,  25, 3,    1},
  {3,  4,  1,  6,  0xff, 0xff},   // c_{1,6} = 1, for Step 4
  {3,  3,  1,  7,  0xff, 0xff},   // c_{1,7} = 1
  {3,  3,  0,  10, 0xff, 0xff},   // c_{1,10} = 0
  {4,  4,  0,  25, 0xff, 0xff},   // b_{1,25}=0
  {4,  5,  1,  6,  0xff, 0xff},   // b_{1,6}=1, for Step 5 avoid \delta d_1
  {4,  5,  0,  7,  0xff, 0xff},   // b_{1,7}=0, for Step 5 avoid \delta c_1
  {4,  5,  0,  10, 0xff, 0xff},   // b_{1,10}=0, for Step 5
  {5,  6,  1,  7,  0xff, 0xff},   // a_{2,7}=1, for Step 6 to eliminate \delta c_1
  {5,  6,  1,  10, 0xff, 0xff},   // a_{2,10}=1, for Step 6
  {5,  7,  2,  13, 1,    1},      // a_{2,13}=b_{1,13}, for Step 7
  {5,  6,  0,  25, 0xff, 0xff},   // a_{2,25}=0, for Step 6
  {6,  6,  0,  13, 0xff, 0xff}, 	// d_{2,13} = 0, pattern
  {6,  7,  1,  25, 0xff, 0xff},   // d_{2,25} = 1, for step 7 avoid b1
  {6,  8,  2,  18, 0,    2},      // d_{2,18} == a_{2,18}, for Step 8 avoid c2
  {6,  8,  2,  19, 0,    2},      // d_{2,19} == a_{2,19}, for Step 8 avoid c2
  {6,  8,  2,  20, 0,    2},      // d_{2,20} == a_{2,20}, for Step 8 avoid c2
  {6,  8,  2,  21, 0,    2},      // d_{2,21} == a_{2,21}, for Step 8 avoid c2
  {7,  8,  0,  13, 0xff, 0xff},   // c_{2,13}=0, for step 8 avoid d2
  {7,  7,  0,  18, 0xff, 0xff},   // c_{2,18}=0, pattern
  {7,  7,  0,  19, 0xff, 0xff},   // c_{2,19}=0, pattern
  {7,  7,  1,  20, 0xff, 0xff},   // c_{2,20}=1, pattern
  {7,  7,  0,  21, 0xff, 0xff},   // c_{2,21}=0, pattern
  {7,  9,  2,  12, 3,    2},      // c_{2,12}=d_{2,12}, for step 9 avoid b2
  {7,  9,  2,  14, 3,    2},      // c_{2,14}=d_{2,14}, for step 9 avoid b2
  {8,  8,  1,  12, 0xff, 0xff},   // b_{2,12} = 1, pattern
  {8,  8,  1,  13, 0xff, 0xff},   // b_{2,13} = 1, pattern
  {8,  8,  0,  14, 0xff, 0xff},   // b_{2,14} = 0, pattern
  {8,  10, 2,  16, 2,    2},      // b_{2,16} = c_{2,16} for step 10 avoid a3
  {8,  9,  0,  18, 0xff, 0xff},   // b_{2,18} = 0, for step 9 avoid c2
  {8,  9,  0,  19, 0xff, 0xff},   // b_{2,19} = 0, for step 9 avoid c2
  {8,  9,  0,  20, 0xff, 0xff},   // b_{2,20} = 0, for step 9 avoid c2
  {8,  9,  0,  21, 0xff, 0xff},   // b_{2,21} = 0, for step 9 avoid c2
  {9,  10, 1,  12, 0xff, 0xff},   // a_{3,12} = 1, for step 10, ???
  {9,  10, 1,  13, 0xff, 0xff},   // a_{3,13} = 1, for step 10, ???
  {9,  10, 1,  14, 0xff, 0xff},   // a_{3,14} = 1, for step 10, ???
  {9,  9,  0,  16, 0xff, 0xff},   // a_{3,16} = 0, pattern
  {9,  10, 0,  18, 0xff, 0xff},   // a_{3,18} = 0, for step 10, ???
  {9,  10, 0,  19, 0xff, 0xff},   // a_{3,19} = 0, for step 10, ??????
// {9,  11, 2,  19, 1,  2},       // a_{3,19} = b_{2,19}=0, for step 11 avoid d3
  {9,  10, 0,  20, 0xff, 0xff},   // a_{3,20} = 0, for step 10, ??????
// {9,  11, 2, 20, 1, 2},		// a_{3,20} = b_{2,20}=0, for step 11 avoid d3
  {9,  10, 1,  21, 0xff, 0xff},   // a_{3,21} = 1, for step 10, ??????
// {9,  11, 3, 21, 1, 2},		// a_{3,21} = b_{2,21}+1=1, for step 11 avoid d3
  {9,  11, 2,  22, 1,    2},      // a_{3,22} = b_{2,22} for step 11 avoid d3
  {9,  11, 2,  25, 1,    2},      // a_{3,25} = b_{2,25} for step 11 avoid d3
  {10, 11, 1,  12, 0xff, 0xff},   // d_{3,12} = 1 for step 11 avoid b2
  {10, 11, 1,  13, 0xff, 0xff},   // d_{3,13} = 1 for step 11 avoid b2
  {10, 11, 1,  14, 0xff, 0xff},   // d_{3,14} = 1 for step 11 avoid b2
  {10, 11, 0,  16, 0xff, 0xff},   // d_{3,16} = 0, for step 11 avoid a3
  {10, 10, 0,  19, 0xff, 0xff},   // d_{3,19} = 0, pattern
  {10, 10, 1,  20, 0xff, 0xff},   // d_{3,20} = 1, pattern
  {10, 10, 1,  21, 0xff, 0xff},   // d_{3,21} = 1, pattern
  {10, 10, 0,  22, 0xff, 0xff},   // d_{3,22} = 0, pattern
  {10, 10, 1,  25, 0xff, 0xff},   // d_{3,25} = 1, pattern
  {10, 12, 2,  29, 0,    3},      // d_{3,29} = a_{3,29}, for step 12 avoid c3
  {11, 12, 1,  16, 0xff, 0xff},   // c_{3,16} = 1, for step 12 avoid a3
  {11, 12, 0,  19, 0xff, 0xff},   // c_{3,19} = 0, for step 12 avoid d3
  {11, 12, 0,  20, 0xff, 0xff},   // c_{3,20} = 0, for step 12 avoid d3
  {11, 12, 0,  21, 0xff, 0xff},   // c_{3,21} = 0, for step 12 avoid d3
  {11, 12, 0,  22, 0xff, 0xff},   // c_{3,22} = 0, for step 12 avoid d3
  {11, 12, 0,  25, 0xff, 0xff},   // c_{3,25} = 0, for step 12 avoid d3
  {11, 11, 1,  29, 0xff, 0xff},   // c_{3,29} = 1, pattern
  {11, 13, 2,  31, 3,    3},      // c_{3,31} = d_{3,31}, step 13 avoid b3
  {12, 13, 0,  19, 0xff, 0xff},   // b_{3,19} = 0, for step 13 produce 2^22
  {12, 13, 1,  20, 0xff, 0xff},   // b_{3,20} = 1, for step 13 avoid d3
  {12, 13, 1,  21, 0xff, 0xff},   // b_{3,21} = 1, for step 13 avoid d3
  {12, 13, 0,  22, 0xff, 0xff},   // b_{3,22} = 0, for step 13 produce 2^25
// {12, 14, 2, 22, 2, 3},  // b_{3,22} = c_{3,22} = 0, for step 14 avoid a4
  {12, 13, 1,  25, 0xff, 0xff},   // b_{3,25} = 1, for step 13 avoid d3
// {12, 14, 3, 25, 2, 3},  // b_{3,25} = c_{3,25} + 1=1, for step 14 eliminate 2^25
  {12, 13, 0,  29, 0xff, 0xff},   // b_{3,29} = 0, for step 13 avoid c3
  {12, 12, 0,  31, 0xff, 0xff},   // b_{3,31} = 0, pattern
  {13, 13, 0,  22, 0xff, 0xff},   // a_{4,22} = 0, pattern
  {13, 13, 0,  25, 0xff, 0xff},   // a_{4,25} = 0, pattern
  {13, 15, 2,  26, 1,    3},      // a_{4,26} = b_{3,26}, for step 15 avoid d4
  {13, 15, 2,  28, 1,    3},      // a_{4,28} = b_{3,28}, for step 15 avoid d4
  {13, 14, 1,  29, 0xff, 0xff},   // a_{4,29} = 1, for step 14 avoid b3
// {13, 15, 3,	29,	1,	3},  // a_{4,29} = b_{3,29}+1=1, for step 15 eliminate -2^29
  {13, 14, 0,  31, 0xff, 0xff},   // a_{4,31} = 0, for step 14 avoid c3
  {14, 15, 0,  22, 0xff, 0xff},   // d_{4,22} = 0, for step 15 avoid a4
  {14, 15, 0,  25, 0xff, 0xff},   // d_{4,25} = 0, for step 15 avoid a4
  {14, 14, 1,  26, 0xff, 0xff},   // d_{4,26} = 1, pattern
  {14, 14, 1,  28, 0xff, 0xff},   // d_{4,28} = 1, pattern
  {14, 14, 0,  29, 0xff, 0xff},   // d_{4,29} = 0, pattern
  {14, 15, 1,  31, 0xff, 0xff},   // d_{4,31} = 1, for step 15 avoid b3
  {15, 17, 2,  18, 3,    4},      // c_{4,18} = d_{4,18}, for step 17 ???
  {15, 16, 1,  22, 0xff, 0xff},   // c_{4,22} = 1, for step 16 avoid a4
  {15, 16, 1,  25, 0xff, 0xff},   // c_{4,25} = 1, for step 16 avoid a4
  {15, 16, 0,  26, 0xff, 0xff},   // c_{4,26} = 0, for step 16 avoid d4
  {15, 16, 0,  28, 0xff, 0xff},   // c_{4,28} = 0, for step 16 avoid d4
  {15, 16, 0,  29, 0xff, 0xff},   // c_{4,29} = 0, for step 16 avoid d4
  {16, 17, 2,  25, 2,    4},      // b_{4,25} = c_{4,25}=1, for step 18 ???
  {16, 17, 3,  26, 2,    4},      // b_{4,26} = c_{4,26}+1=1, for step 17&18 ???
  {16, 17, 3,  28, 2,    4},      // b_{4,28} = c_{4,28}+1=1, for step 17&18 ???
  {16, 17, 2,  29, 2,    4},      // b_{4,29} = c_{4,29}=0, for step 17 ???
  {16, 17, 2,  31, 2,    4},      // b_{4,31} = c_{4,31}, for step 18 ???
  {16, 16, 0,  18, 0xff, 0xff}    // b_{4,18} = 0, pattern
};

bool __modify_type_01(CrackContext& cxx, DiffCondition& cond, INT32 sw) {
  INT32 reg = REG(cond.step);
  INT32 offset = OFFSET(cond.step);

  if (PICKBIT(cxx.r1[reg][offset], cond.bit)==sw) {
    return true;	// success
  }

  INT32 rnd = 0;
  if (cond.step < 17) {
    rnd = 0;
  } else if (cond.step < 33) {
    rnd = 1;
  } else {
    rnd = 2;
  }

  // modify the message
  INT32 shf = cond.bit - g_rotation[rnd][(cond.step-1) % 4];
  if (shf < 0) {
    shf += 32;
  }

  if (sw==0) {
    cxx.m1[g_mes_index[cond.step-1]] -= (0x01 << shf);
  } else {
    cxx.m1[g_mes_index[cond.step-1]] += (0x01 << shf);
  }

  updatem2(cxx,g_mes_index[cond.step-1]);
  select_md4(cxx, cond.step);

  return PICKBIT(cxx.r1[reg][offset], cond.bit) == sw;
}

bool modify_type_0(CrackContext& cxx, DiffCondition& cond) {
  return __modify_type_01(cxx, cond, 0);
}

bool modify_type_1(CrackContext& cxx, DiffCondition& cond) {
  return __modify_type_01(cxx, cond, 1);
}

bool modify_type_2(CrackContext& cxx, DiffCondition& cond) {
  if (PICKBIT(cxx.r1[cond.ref1][cond.ref2], cond.bit) == 0) {
    return modify_type_0(cxx, cond);
  } else {
    return modify_type_1(cxx, cond);
  }
}

bool modify_type_3(CrackContext& cxx, DiffCondition& cond) {
  return modify_type_1( cxx,  cond); // @todo
}

// return the number of violated conditions
int check_condition(CrackContext& cxx, DiffCondition* cnds, int len) {
  int count = 0;
  for (int i=0; i < len; i++) {
    DiffCondition& cond = cnds[i];
    INT32 reg = (101 - cond.step) % 4;
    INT32 offset = (cond.step-1) / 4 + 1;

    switch (cond.type) {
      case 0:
        if (PICKBIT(cxx.r1[reg][offset], cond.bit) != 0) {
          count++;
        }
        break;
      case 1:
        if (PICKBIT(cxx.r1[reg][offset], cond.bit) != 1) {
          count++;
        }
        break;
      case 2:
        if (PICKBIT(cxx.r1[reg][offset], cond.bit) !=
            PICKBIT(cxx.r1[cond.ref1][cond.ref2], cond.bit)) {
          count++;
        }
        break;
      case 3:
        if (PICKBIT(cxx.r1[reg][offset], cond.bit) !=
            PICKBIT(cxx.r1[cond.ref1][cond.ref2], cond.bit)) { // @todo
          count++;
        }
        break;
      default:
        ;
    }
  }
  return count;
}

int test_output(CrackContext& cxx) {
  INT32 diff[4][13];
  minus(diff[0], cxx.r2[0], cxx.r1[0], 13);
  minus(diff[1], cxx.r2[1], cxx.r1[1], 13);
  minus(diff[2], cxx.r2[2], cxx.r1[2], 13);
  minus(diff[3], cxx.r2[3], cxx.r1[3], 13);

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 5; j++) {
      if (diff[i][j] != g_diff[i][j]) {
        return i * 4 + j;
      }
    }
  }
  return 0;
}

bool modify_switcher(CrackContext& cxx, DiffCondition& cond) {
  switch (cond.type) {
    case 0:
      return modify_type_0(cxx, cond);
    case 1:
      return modify_type_1(cxx, cond);
    case 2:
      return modify_type_2(cxx, cond);
    case 3:
      return modify_type_3(cxx, cond);
    default:
      return false;
  }
}

int modify_first_round(CrackContext& cxx, DiffCondition* cnds, int nocond) {
  int count = 0;
  for (int i = 1; i <= 16; i++) {
    select_md4(cxx, i);
    while ((cnds[count].step==i) && (count<nocond)) {
      if (!modify_switcher(cxx, cnds[count++])) {
        return i * 10000 + count;
      }
    }
  }
  return test_output(cxx);  // 0 for success
}

////////////////////////////////////////////////////////
// Try to modify the second round, not complete yet
DiffCondition g_second_conds[] = {
// step rstep type bit ref1 ref2
  {17, 18, 2, 18, 2,    4},     // a_{5,18} = c_{4,18}, for step 18 avoid b4
  {17, 17, 1, 25, 0xff, 0xff},  // a_{5,25} = 1, pattern
  {17, 17, 0, 26, 0xff, 0xff},  // a_{5,26} = 0, pattern
  {17, 17, 1, 28, 0xff, 0xff},  // a_{5,28} = 1, pattern
  {17, 17, 1, 31, 0xff, 0xff},  // a_{5,31} = 1, pattern
  {18, 19, 2, 18, 0,    5},     // d_{5,18} = a_{5,18} in step 19 to avoid b4
  {18, 19, 2, 25, 1,    4},     // d_{5,25} = b_{4,25} in step 19 to avoid a5
  {18, 19, 2, 26, 1,    4},     // d_{5,26} = b_{4,26}
  {18, 19, 2, 28, 1,    4},     // d_{5,28} = b_{4,28}
  {18, 19, 2, 31, 1,    4},     // d_{5,31} = a_{5,31} ???? b_{4,31}?
  {19, 20, 2, 25, 3,    5},     // c_{5,25} = d_{5,25}, in step 20 to avoid a5
  {19, 20, 2, 26, 3,    5},     // c_{5,26} = d_{5,26}, in step 20 to avoid a5
  {19, 20, 2, 28, 3,    5},     // c_{5,28} = d_{5,28}, in step 20 to avoid a5
  {19, 20, 2, 31, 3,    5}      // c_{5,31} = d_{5,31}, in step 20 to avoid a5
};

bool modify_second_round(CrackContext& cxx, DiffCondition& cnd) {
  if (!modify_switcher(cxx, cnd)) {
    return false;
  }

  // recomputer affected messages in first round
  int afs = g_mes_index[cnd.step - 1] + 1; // affected step in first round
  INT32 olda = cxx.r1[REG(afs)][OFFSET(afs)];
  select_md4(cxx, afs);

  for (int i = 1; i < 4; i++) {
    INT32 reg = REG(afs + i);
    INT32 offset = OFFSET(afs + i);
    INT32 old = cxx.r1[reg][offset];  // store old register

    select_md4(cxx, afs + i);	        // recomputer this step
    INT32 dif = cxx.r1[reg][offset] - old;
    if (dif > 0) {
      cxx.m1[afs + i - 1] -= RRT(g_rotation[0][(afs + i - 1) % 4], dif);
    } else if (dif < 0) {
      dif = -dif;
      cxx.m1[afs + i - 1] += RRT(g_rotation[0][(afs + i - 1) % 4], dif);
    }

    updatem2(cxx, afs + i - 1);
    select_md4(cxx, afs + i);
    dif = cxx.r1[reg][offset] - old;
    if (dif) {
      return false;
    }
  }

  // for the fouth affect register
  INT32 diffa = cxx.r1[REG(afs)][OFFSET(afs)] - olda;
  if (diffa > 0) {
    cxx.m1[afs + 3] -= diffa;
  } else {
    diffa = -diffa;
    cxx.m1[afs + 3] += diffa;
  }
  updatem2(cxx, afs + 3);
  olda = cxx.r1[REG(afs + 4)][OFFSET(afs + 4)];
  select_md4(cxx, afs + 4);
  diffa = cxx.r1[REG(afs + 4)][OFFSET(afs + 4)] - olda;
  if (diffa) {
    return false;
  }

  return true;
  /*
  int rt = check_condition(cxx, g_first_conds,
                           sizeof(g_first_conds)/sizeof(DiffCondition));
  int rt = modify_first_round(cxx, g_first_conds,
                              sizeof(g_first_conds)/sizeof(DiffCondition));
  rt = check_condition(cxx, g_second_conds,
                       sizeof(g_second_conds)/sizeof(DiffCondition));
  */
}

int test_output2(CrackContext& cxx) {
  for (int i=1; i<=17; i++) {
    select_md4(cxx, i);
  }

  INT32 diff[4][13];
  minus(diff[0], cxx.r2[0], cxx.r1[0], 13);
  minus(diff[1], cxx.r2[1], cxx.r1[1], 13);
  minus(diff[2], cxx.r2[2], cxx.r1[2], 13);
  minus(diff[3], cxx.r2[3], cxx.r1[3], 13);

  if (diff[0][5]!=g_diff[0][5]) {
    return 17;
  }
  return 0;
}

int modify_second_round(CrackContext& cxx, DiffCondition* cnds, int nocond) {
  int count = 0;
  for (int i = 17; i <= 19; i++) {
    select_md4(cxx, i);
    while ((cnds[count].step==i) && (count<nocond)) {
      if (!modify_second_round(cxx, cnds[count++])) {
        return i*10000+count-1;
      }
    }
  }
  return  0;  // test_output2(cxx);  // 0 for success @todo
}
// End of Incomplete Codes for the Second Round
////////////////////////////////////////////////////////////

void run(int seed, int iterations) {
  srand(seed);

  CrackContext cxx;
  memset(&cxx, 0, sizeof(cxx));

  cxx.r1[0][0] = cxx.r2[0][0] = INITIAL_VALUE[0];
  cxx.r1[1][0] = cxx.r2[1][0] = INITIAL_VALUE[1];
  cxx.r1[2][0] = cxx.r2[2][0] = INITIAL_VALUE[2];
  cxx.r1[3][0] = cxx.r2[3][0] = INITIAL_VALUE[3];

  int i=0;
  int err_count = 0;
  for (i=0; (i<iterations) && (err_count<100); i++) {
    rand_message(cxx.m1);
    add(cxx.m2, cxx.m1, g_dm, 16);

    // First Round Modification
    int rt = modify_first_round(cxx, g_first_conds,
                                sizeof(g_first_conds) / sizeof(DiffCondition));
    if (rt < 10000 && rt!=0 )  {  // failed at output
      OUTPUT_FAILED(rt);
      err_count++;
      continue;
    } else if (rt!=0)   {  // failed at condition rt-1
      TEST_FAILED(rt);
      err_count++;
      continue;
    }

    // Second Round Modification
    rt = modify_second_round(cxx, g_second_conds, 14);
    /*
    if (rt<10000 && rt!=0)  { // failed in output
      OUTPUT_FAILED(rt);
    err_count++;
      continue;
  } else if (rt!=0) {
      // failed at condtion rt-1 ( second round condition index )
      TEST_FAILED(rt);
      err_count++;
      continue; // do nothing now
    }
    */

    ////////////////////////////
    //  Test if it is a collision
    md4(cxx.m1, INITIAL_VALUE, cxx.h1,
        cxx.r1[0], cxx.r1[1], cxx.r1[2], cxx.r1[3]);
    md4(cxx.m2, INITIAL_VALUE, cxx.h2,
        cxx.r2[0], cxx.r2[1], cxx.r2[2], cxx.r2[3]);

    if (success(cxx)) {
      CMDLOG("success in " << i << "-th step!");
      break;
    }
    if ((i%100000)==0) {
      CMDLOG("searched " << i << " messages");
    }
  }

  add(cxx.m2, cxx.m1, g_dm, 16);
  md4(cxx.m1, INITIAL_VALUE, cxx.h1,
      cxx.r1[0], cxx.r1[1], cxx.r1[2], cxx.r1[3]);
  md4(cxx.m2, INITIAL_VALUE, cxx.h2,
      cxx.r2[0], cxx.r2[1], cxx.r2[2], cxx.r2[3]);

  if (success(cxx)) {
    CMDLOG("Collision:");
    dump(cxx, std::cout);
  }
}

void dump(CrackContext& cxx, std::ostream& os) {
  os << "\nMessage 1 :\n";
  dump(cxx.m1, 16, os);

  os << "\nMessage 2 :\n";
  dump(cxx.m2, 16, os);

  os << "\nHash 1 :\n";
  dump(cxx.h1, 4, os);

  os << "\nHash 2 :\n";
  dump(cxx.h2, 4, os);

  /*
  INT32 da[13], db[13], dc[13], dd[13];
  minus(da, cxx.r2[0], cxx.r1[0], 13);
  minus(db, cxx.r2[1], cxx.r1[1], 13);
  minus(dc, cxx.r2[2], cxx.r1[2], 13);
  minus(dd, cxx.r2[3], cxx.r1[3], 13);

  std::cout << "da :\n"; dump(da+1, 12);
  std::cout << "db :\n"; dump(db+1, 12);
  std::cout << "dc :\n"; dump(dc+1, 12);
  std::cout << "dd :\n"; dump(dd+1, 12);
  */
}

void select_md4(CrackContext& cxx, int step) {
  switch (step) {
    case 1:
      STEP01(cxx.r1[0], cxx.r1[1], cxx.r1[2], cxx.r1[3], cxx.m1);
      STEP01(cxx.r2[0], cxx.r2[1], cxx.r2[2], cxx.r2[3], cxx.m2);
      break;
    case 2:
      STEP02(cxx.r1[0], cxx.r1[1], cxx.r1[2], cxx.r1[3], cxx.m1);
      STEP02(cxx.r2[0], cxx.r2[1], cxx.r2[2], cxx.r2[3], cxx.m2);
      break;
    case 3:
      STEP03(cxx.r1[0], cxx.r1[1], cxx.r1[2], cxx.r1[3], cxx.m1);
      STEP03(cxx.r2[0], cxx.r2[1], cxx.r2[2], cxx.r2[3], cxx.m2);
      break;
    case 4:
      STEP04(cxx.r1[0], cxx.r1[1], cxx.r1[2], cxx.r1[3], cxx.m1);
      STEP04(cxx.r2[0], cxx.r2[1], cxx.r2[2], cxx.r2[3], cxx.m2);
      break;
    case 5:
      STEP05(cxx.r1[0], cxx.r1[1], cxx.r1[2], cxx.r1[3], cxx.m1);
      STEP05(cxx.r2[0], cxx.r2[1], cxx.r2[2], cxx.r2[3], cxx.m2);
      break;
    case 6:
      STEP06(cxx.r1[0], cxx.r1[1], cxx.r1[2], cxx.r1[3], cxx.m1);
      STEP06(cxx.r2[0], cxx.r2[1], cxx.r2[2], cxx.r2[3], cxx.m2);
      break;
    case 7:
      STEP07(cxx.r1[0], cxx.r1[1], cxx.r1[2], cxx.r1[3], cxx.m1);
      STEP07(cxx.r2[0], cxx.r2[1], cxx.r2[2], cxx.r2[3], cxx.m2);
      break;
    case 8:
      STEP08(cxx.r1[0], cxx.r1[1], cxx.r1[2], cxx.r1[3], cxx.m1);
      STEP08(cxx.r2[0], cxx.r2[1], cxx.r2[2], cxx.r2[3], cxx.m2);
      break;
    case 9:
      STEP09(cxx.r1[0], cxx.r1[1], cxx.r1[2], cxx.r1[3], cxx.m1);
      STEP09(cxx.r2[0], cxx.r2[1], cxx.r2[2], cxx.r2[3], cxx.m2);
      break;
    case 10:
      STEP10(cxx.r1[0], cxx.r1[1], cxx.r1[2], cxx.r1[3], cxx.m1);
      STEP10(cxx.r2[0], cxx.r2[1], cxx.r2[2], cxx.r2[3], cxx.m2);
      break;
    case 11:
      STEP11(cxx.r1[0], cxx.r1[1], cxx.r1[2], cxx.r1[3], cxx.m1);
      STEP11(cxx.r2[0], cxx.r2[1], cxx.r2[2], cxx.r2[3], cxx.m2);
      break;
    case 12:
      STEP12(cxx.r1[0], cxx.r1[1], cxx.r1[2], cxx.r1[3], cxx.m1);
      STEP12(cxx.r2[0], cxx.r2[1], cxx.r2[2], cxx.r2[3], cxx.m2);
      break;
    case 13:
      STEP13(cxx.r1[0], cxx.r1[1], cxx.r1[2], cxx.r1[3], cxx.m1);
      STEP13(cxx.r2[0], cxx.r2[1], cxx.r2[2], cxx.r2[3], cxx.m2);
      break;
    case 14:
      STEP14(cxx.r1[0], cxx.r1[1], cxx.r1[2], cxx.r1[3], cxx.m1);
      STEP14(cxx.r2[0], cxx.r2[1], cxx.r2[2], cxx.r2[3], cxx.m2);
      break;
    case 15:
      STEP15(cxx.r1[0], cxx.r1[1], cxx.r1[2], cxx.r1[3], cxx.m1);
      STEP15(cxx.r2[0], cxx.r2[1], cxx.r2[2], cxx.r2[3], cxx.m2);
      break;
    case 16:
      STEP16(cxx.r1[0], cxx.r1[1], cxx.r1[2], cxx.r1[3], cxx.m1);
      STEP16(cxx.r2[0], cxx.r2[1], cxx.r2[2], cxx.r2[3], cxx.m2);
      break;
    case 17:
      STEP17(cxx.r1[0], cxx.r1[1], cxx.r1[2], cxx.r1[3], cxx.m1);
      STEP17(cxx.r2[0], cxx.r2[1], cxx.r2[2], cxx.r2[3], cxx.m2);
      break;
    case 18:
      STEP18(cxx.r1[0], cxx.r1[1], cxx.r1[2], cxx.r1[3], cxx.m1);
      STEP18(cxx.r2[0], cxx.r2[1], cxx.r2[2], cxx.r2[3], cxx.m2);
      break;
    case 19:
      STEP19(cxx.r1[0], cxx.r1[1], cxx.r1[2], cxx.r1[3], cxx.m1);
      STEP19(cxx.r2[0], cxx.r2[1], cxx.r2[2], cxx.r2[3], cxx.m2);
      break;
    default:
      return;
  }
}
