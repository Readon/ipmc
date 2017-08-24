#include <stdio.h>
#include <stm32f2xx.h>
#include <stm32f2xx_adc.h>
#include <system.h>

#include "IPMC.h"

/*
PA0/WKUP/ADC123	      FP5_DC1V0_ADC	FPGA5�ں˵�ѹ�����ֵ��1.0v����ѹ��Χ��0.97--1.03v
PA1/ADC123_IN1	      MB_1V8_ADC	���ֵ��1.8v
PA2/ADC123_IN2	      MB_1V2_ADC	���ֵ��1.2v
PA3/ADC123_IN3	      FP2_DC1V0_ADC	FPGA2�ں˵�ѹ�����ֵ��1.0v����ѹ��Χ��0.97--1.03v
PA4/ADC12_IN4	      TLR_MEM0_VTT_0V75_ADC	TILERA�ڴ�VTT��ѹ�����ֵ��0.75v
PA5/ADC12_IN5	      FP3_DC1V0_ADC	FPGA3�ں˵�ѹ�����ֵ��1.0v����ѹ��Χ��0.97--1.03v
PA6/ADC12_IN6	      TLR_CORE_0V95_ADC	TILERA�ں˵�Դ�����ֵ��0.95v��������������1.05v
PA7/ADC12_IN7	      FP4_DC1V0_ADC	FPGA4�ں˵�ѹ�����ֵ��1.0v����ѹ��Χ��0.97--1.03v
PB0/ADC12_IN8	      TLR_DDR0_1V5_ADC	TILERA�ڴ��ѹ�����ֵ��1.5v
PB1/ADC12_IN9	      ISL_VCORE_IMON	TILERA�ں˵�Դ������������⣬��ѹС��1.12v
PC0/ADC123_IN10	      FP_3V3_ADC	FPGA IO bank��ѹ�����ֵ��3.3v
PC1/ADC123_IN11	      MB_5V_2V5_ADC	PSU��Դ5v���룬���ֵ��2.5v
PC2/ADC123_IN12	      MB_3V3_ADC	PSU��Դ3.3v���룬���ֵ��3.3v
PC3/ADC123_IN13	      FP1_DC1V0_ADC	FPGA1�ں˵�ѹ�����ֵ��1.0v����ѹ��Χ��0.97--1.03v
PC4/ADC12_IN14	      PLX_X8_1V0_ADC	���ֵ��1.0v
PC5/ADC12_IN15	      FP_1V8_ADC	���ֵ��1.8v
PF3/ADC3_IN9	      FPGA_MEM0_VTT_0V75_ADC	FPGA�ڴ�VTT��ѹ�����ֵ��0.75v
PF4/ADC3_IN14	      FP_DDR3_1V5_ADC	FPGA�ڴ��ѹ�����ֵ��1.5v
PF5/ADC3_IN15	      FP6_DC1V0_ADC	FPGA6�ں˵�ѹ�����ֵ��1.0v����ѹ��Χ��0.97--1.03v
PF6/ADC3_IN4	      FP_1V2_ADC	���ֵ��1.2v
PF7/ADC3_IN5	      MB_3V3_AUX_ADC	������Դ�����ֵ��3.3v
PF8/ADC3_IN6	      MB_2V5_ADC	���ֵ��2.5v
PF9/ADC3_IN7	      MB_12V_2V_ADC	PSU��Դ12v���룬���ֵ��2v
PF10/ADC3_IN8	      PLX_X4_1V0_ADC	���ֵ��1.0v


FP1_DC1V0_ADC
FP2_DC1V0_ADC
FP3_DC1V0_ADC
FP4_DC1V0_ADC
FP5_DC1V0_ADC
FP6_DC1V0_ADC

FP_1V2_ADC
FP_3V3_ADC
FP_DDR3_1V5_ADC
FP_1V8_ADC
FPGA_MEM0_VTT_0V75_ADC

MB_1V8_ADC
MB_1V2_ADC
MB_5V_2V5_ADC
MB_3V3_ADC
MB_3V3_AUX_ADC
MB_2V5_ADC
MB_12V_2V_ADC

PLX_X4_1V0_ADC
PLX_X8_1V0_ADC

TLR_DDR0_1V5_ADC
TLR_MEM0_VTT_0V75_ADC
TLR_CORE_0V95_ADC

ISL_VCORE_IMON


*/
#define ADC_REF_VOLTAGE   3.3 //stm32оƬ�ڲ�ADת���Ƚϵ�ѹ
#define ADC_CONVERT_BITS  12 //12λADת��
#define ADC_CONVERT_VAL   (float)0.0824 //ADC_CONVERT_VAL = ADC_REF_VOLTAGE*10/(1<<ADC_CONVERT_BITS)
#define ADC_CONVERT_VAL_12DC  (float)0.4944
#define ADC_CONVERT_VAL_5DC  (float)0.1662
extern IPMC_DATA_AREA local_data_pool;
extern char send_byte(USART_TypeDef * UARTx, unsigned char ch);
//

void adcConvertOn()
{
    ADC1->CR1   |=  ( 1UL <<  5);
}
void adcConvertOff()
{
    ADC1->CR1   &=  ~( 1UL <<  5);
}
void adcValToHumanInterface()
{
    
	adcConvertOff();
    //calculate ad value
    sys_data.sys_vtg_cur->FP5_DC1V0=     (float)sys_data.aADCTripleConvertedValue[0]   * ADC_CONVERT_VAL;//
    sys_data.sys_vtg_cur->DC1V8 =        (float)sys_data.aADCTripleConvertedValue[3]   * ADC_CONVERT_VAL;//
    sys_data.sys_vtg_cur->DC1V2 =        (float)sys_data.aADCTripleConvertedValue[6]   * ADC_CONVERT_VAL;//
    sys_data.sys_vtg_cur->FP2_DC1V0=     (float)sys_data.aADCTripleConvertedValue[9]   * ADC_CONVERT_VAL;//
    sys_data.sys_vtg_cur->TLR_MEM0_0V75= (float)sys_data.aADCTripleConvertedValue[12]  * ADC_CONVERT_VAL;//
    sys_data.sys_vtg_cur->FP3_DC1V0=     (float)sys_data.aADCTripleConvertedValue[15]  * ADC_CONVERT_VAL;//
    sys_data.sys_vtg_cur->TLR_CORE_0V95= (float)sys_data.aADCTripleConvertedValue[18]  * ADC_CONVERT_VAL;//
    sys_data.sys_vtg_cur->FP4_DC1V0   =  (float)sys_data.aADCTripleConvertedValue[21]  * ADC_CONVERT_VAL;//
    
    sys_data.sys_vtg_cur->TLR_MEM0_1V5=  (float)sys_data.aADCTripleConvertedValue[1]  * ADC_CONVERT_VAL;//
    sys_data.sys_vtg_cur->ISL_VCORE_IMON=(float)sys_data.aADCTripleConvertedValue[4]  * ADC_CONVERT_VAL;//
    sys_data.sys_vtg_cur->FP3V3=         (float)sys_data.aADCTripleConvertedValue[7]  * ADC_CONVERT_VAL;//
    sys_data.sys_vtg_cur->MB5V=          (float)sys_data.aADCTripleConvertedValue[10] * ADC_CONVERT_VAL_5DC;//
    sys_data.sys_vtg_cur->MB3V3=         (float)sys_data.aADCTripleConvertedValue[13] * ADC_CONVERT_VAL;
    sys_data.sys_vtg_cur->FP1_DC1V0 =    (float)sys_data.aADCTripleConvertedValue[16] * ADC_CONVERT_VAL;
    sys_data.sys_vtg_cur->PLX8_1V0=      (float)sys_data.aADCTripleConvertedValue[19] * ADC_CONVERT_VAL;
  	sys_data.sys_vtg_cur->FP1V8=         (float)sys_data.aADCTripleConvertedValue[22] * ADC_CONVERT_VAL;
  	
    sys_data.sys_vtg_cur->FP_MEM0_0V75=  (float)sys_data.aADCTripleConvertedValue[2]  * ADC_CONVERT_VAL;
    sys_data.sys_vtg_cur->FP_MEM0_1V5=   (float)sys_data.aADCTripleConvertedValue[5]  * ADC_CONVERT_VAL;
    sys_data.sys_vtg_cur->FP6_DC1V0=     (float)sys_data.aADCTripleConvertedValue[8]  * ADC_CONVERT_VAL;
    sys_data.sys_vtg_cur->FP1V2=         (float)sys_data.aADCTripleConvertedValue[11] * ADC_CONVERT_VAL;
    sys_data.sys_vtg_cur->AUX3V3=        (float)sys_data.aADCTripleConvertedValue[14] * ADC_CONVERT_VAL;
    sys_data.sys_vtg_cur->MB2V5=         (float)sys_data.aADCTripleConvertedValue[17] * ADC_CONVERT_VAL;
    sys_data.sys_vtg_cur->DC12V=         (float)sys_data.aADCTripleConvertedValue[20] * ADC_CONVERT_VAL_12DC;
    sys_data.sys_vtg_cur->PLX4_1V0=      (float)sys_data.aADCTripleConvertedValue[23] * ADC_CONVERT_VAL;
    adcConvertOn();
    
}

void adcCmpLimit()
{
    int i;
    int *actval, *limval;
    //actval = local_data_pool.sensorStaTab.vtg.fpga_core_vtg;
    //limval = local_data_pool.sensorLimitTab.vtg.fpga_core_vtg;
    for(i=0; i<24; i++)
    {
        if(*(actval+i) - *(limval+i) > 0)
        {
            local_data_pool.sensorAlarmTab.vtg |= (0x1<<i);
        }
    }
}
void ADC_IRQHandler (void)
{
    adcConvertOff();
    adcValToHumanInterface();
    adcCmpLimit();
    adcConvertOn();
}




/*EOF*/

