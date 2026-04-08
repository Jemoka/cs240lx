// jfc sorry dawson

#ifndef __repeat__h__
#define __repeat__h__

#define REPEAT_INNER(F, N) REPEAT_##N(F)

#define REPEAT_0(F)
/* #define REPEAT_1(F)  REPEAT_0(F)  F(0) */
#define REPEAT_2(F)  REPEAT_0(F)  F(1) // trust (to prevent 0 from being used and getting optimized away)
#define REPEAT_3(F)  REPEAT_2(F)  F(2)
#define REPEAT_4(F)  REPEAT_3(F)  F(3)
#define REPEAT_5(F)  REPEAT_4(F)  F(4)
#define REPEAT_6(F)  REPEAT_5(F)  F(5)
#define REPEAT_7(F)  REPEAT_6(F)  F(6)
#define REPEAT_8(F)  REPEAT_7(F)  F(7)
#define REPEAT_9(F)  REPEAT_8(F)  F(8)
#define REPEAT_10(F) REPEAT_9(F)  F(9)
#define REPEAT_11(F) REPEAT_10(F) F(10)
#define REPEAT_12(F) REPEAT_11(F) F(11)
#define REPEAT_13(F) REPEAT_12(F) F(12)
#define REPEAT_14(F) REPEAT_13(F) F(13)
#define REPEAT_15(F) REPEAT_14(F) F(14)
#define REPEAT_16(F) REPEAT_15(F) F(15)
#define REPEAT_17(F) REPEAT_16(F) F(16)
#define REPEAT_18(F) REPEAT_17(F) F(17)
#define REPEAT_19(F) REPEAT_18(F) F(18)
#define REPEAT_20(F) REPEAT_19(F) F(19)
#define REPEAT_21(F) REPEAT_20(F) F(20)
#define REPEAT_22(F) REPEAT_21(F) F(21)
#define REPEAT_23(F) REPEAT_22(F) F(22)
#define REPEAT_24(F) REPEAT_23(F) F(23)
#define REPEAT_25(F) REPEAT_24(F) F(24)
#define REPEAT_26(F) REPEAT_25(F) F(25)
#define REPEAT_27(F) REPEAT_26(F) F(26)
#define REPEAT_28(F) REPEAT_27(F) F(27)
#define REPEAT_29(F) REPEAT_28(F) F(28)
#define REPEAT_30(F) REPEAT_29(F) F(29)
#define REPEAT_31(F) REPEAT_30(F) F(30)

#endif 

