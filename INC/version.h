
#ifndef _VERSION_H_
#define _VERSION_H_

#define IPMC_VER_MAIN                0// released times
#define IPMC_VER_SUB                   0//new componment or module added
#define IPMC_VER_FIX                     1// fixed time


#define _VER_STR(r, x, y)       #r"."#x"."#y
#define VER_STR(r, x, y)       _VER_STR(r, x, y)

#define VER_INT(r, x, y)       ((r<<16)+(x<<8)+(y))

#define IPMC_VER_STR      VER_STR(IPMC_VER_MAIN, IPMC_VER_SUB, IPMC_VER_FIX) 
#define IPMC_VER_INT        VER_INT(IPMC_VER_MAIN, IPMC_VER_SUB, IPMC_VER_FIX)


#define BDNAME  ""
#define BDSER   "hds-2017081400001"
#define BDVER   "1.0.0 2017-08"
#define BDDES   "by Green XY, HangZhou 2017"
#define SOFTV   IPMC_VER_STR


//unsigned char code TimeStr[]=__TIME__;


#endif
/*EOF*/
