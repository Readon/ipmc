


#ifndef _COMP_ADC_H
#define _COMP_ADC_H


#include <stm32f2xx.h>

typedef struct voltage {
    int TLR_MEM0_0V75;
    int TLR_MEM0_1V5;
    int TLR_MEM1_0V75;
    int TLR_MEM1_1V5;
    int TLR_CORE_0V95;
    int TLR_CORE_I;
    int FP1_DC0V9;
    int FP1_CORE_I;
    int FP2_CORE_I;
    int FP2_DC0V9;
    int FP_MEM0_0V75;
    int FP_MEM0_1V5;
    
    int DC1V8;
    int PLX1V0;
    int DC1V2;
    int DC1V0;
    int AUX3V3;// MB_3V3_AUX
    int DC5V;
    int DC3V3;// PC2
    int DC2V5;
    int DC3V;
    int DC1V1;
    int DC12V;
    int FP1_DC1V0;
    int FP2_DC1V0;
    int FP3_DC1V0;
    int FP4_DC1V0;
    int FP5_DC1V0;
    int FP6_DC1V0;
    int ISL_VCORE_IMON;
    int FP3V3;
    int MB3V3;
    int MB2V5;
    int FP1V8;
    int FP1V2;
    int FP1V0;
    int PLX8_1V0;
    int PLX4_1V0;
    int MB5V;
}VOLTAGE_MONITOR_t;


#define VOLTAGE_MON_AMF_2V5     0
#define VOLTAGE_MON_AMF_3V0     1
#define VOLTAGE_MON_AMF_1V4     2
#define VOLTAGE_MON_AMF_1V2     3
#define VOLTAGE_MON_AMF_1V1     4
#define VOLTAGE_MON_AMF_1V0     5
#define VOLTAGE_MON_AMF_1V8     6

#endif
/*EOF*/

