/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
    int all_one_res = x&y;
    int all_zero_res = (~x) & (~y);
    return (~all_one_res) & (~all_zero_res);
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
    return (1 << 31);
}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
    return !(~x ^ (x + 1)) ^ (!~x);
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
    int k = (0xAA);
    int c = k;
    k = (k << 8) + c;
    k = (k << 8) + c;
    k = (k << 8) + c;
    
    return !((x & k) ^ k);
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
    return ~x + 1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
    int a = 0x30;
    int a1 = x + (~a + 1);
    /*小于'0',res1 = 1*/
    int res1 = (a1 >> 31) & 1; 
    int b = 0x39;
    int b1 = b + (~x + 1);
    /*大于'9',res2 = 0*/
    int res2 = (b1 >> 31) & 1;
    return (!res1 & !res2);
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
    /*实现一个开关语句,使得语句两边只有一个成立*/
    /*x != 0,y表达式为y,x == 0,y表达式为0*/
    /*x != 0,x表达式为0,x == 0,x表达式为1*/
    int res = ((!x + (~0)) & y) + ((!(!x) + (~0)) & z);
    return res;
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
    int sign_x = (x >> 31) & 1;
    int sign_y = (y >> 31) & 1;
    /*0表示同号,1表示异号*/
    int sign_compare = sign_x ^ sign_y;
    /*如果同号,肯定不会溢出,直接比较结果,sign_res 为0,说明结果正确*/
    int sign_res = ((y + (~x + 1)) >> 31 ) & 1;
    /*如果异号,可能会溢出,但是只需要知道 y>=0 和 x < 0的情况即可*/
    int res = ((!sign_compare) & !sign_res) + 
    ((sign_compare) & (!sign_y & sign_x)); 

    return res;
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
    /* if(x)  return 0;*/
    /* if(!x) return 1;*/
    /* 只有0的负数的符号位和他本身相同*/
    int a_high_bit = (~(x >> 31)) & 1;
    int b_high_bit = (~(~x + 1)) >> 31 & 1;
    return (a_high_bit & b_high_bit); 
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complem
 ent
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
    /* 最后能节省的一定 = 符号位,那么只要知道符号位是多少 */
    /* 对于正数而言,找到最左边的1,对于负数而言,找到最左边的0 */
    // int high_bit = (x >> 31) & 1;
    /* bit != high_bit ? 当前位:原本位 */
    
}
//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
    unsigned sign = uf >> 31 & 1;
    unsigned x = uf & ((1 << 31) - 1);
    unsigned expoent = x >> 23;
    unsigned significand = x & ((1 << 23) - 1);

    if(expoent == ((1 << 8) - 1)) // NAN
        return uf;
    if(expoent == 0) { // 非规格化数
        significand = significand << 1;
        return (sign << 31) + (expoent << 23) + significand; 
    } 
    // 规格化数
    expoent = expoent + 1;
    return (sign << 31) + (expoent << 23) + significand; 
}
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
    unsigned sign = uf >> 31 & 1;
    unsigned x = uf & ((1 << 31) - 1);
    unsigned expoent = x >> 23;
    unsigned significand = x & ((1 << 23) - 1);
    unsigned bias = (1 << 7) - 1;
    int E = expoent - bias;
    int i = 0;
    unsigned sum = 0;
    if(E > 31)
        return 0x80000000u;
    if(E < 0) 
        return 0u;
    if(E == 31) {
        if(sign && !significand)
            return 1 << 31;
        return 0x80000000u;
    }
    sum = 1 << E;
    for(i = 0;i < 23;i ++ ) {
        int x = E - 23 + i;
        if(x < 0) continue;
        sum = sum + (significand >> i & 1) << x;
    }
    if(sign)
        sum *= -1;
    return sum;
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
    unsigned Bias = (1 << 7) - 1;
    unsigned max_expoent = (1 << 8) - 1;
    unsigned res = 0;
    unsigned INF = max_expoent << 23;
    if(x < 0) {
        if(x < -23)
            return 0;
        res |= (1 << (23 + x));
        return res;
    }
    if(x > (max_expoent - Bias))
        return INF;
    
    res = x + Bias;
    res = res << 23;
    return res;
}
