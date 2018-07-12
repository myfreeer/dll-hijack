#define DLL_NAME "msimg32"
#define vSetDdrawflag _real_vSetDdrawflag
#define AlphaBlend _real_AlphaBlend
#define DllInitialize _real_DllInitialize
#define GradientFill _real_GradientFill
#define TransparentBlt _real_TransparentBlt
#include "hijack.h"
#undef vSetDdrawflag
#undef AlphaBlend
#undef DllInitialize
#undef GradientFill
#undef TransparentBlt

EXPORT vSetDdrawflag() NOP_FUNC(1);
EXPORT AlphaBlend() NOP_FUNC(2);
EXPORT DllInitialize() NOP_FUNC(3);
EXPORT GradientFill() NOP_FUNC(4);
EXPORT TransparentBlt() NOP_FUNC(5);
