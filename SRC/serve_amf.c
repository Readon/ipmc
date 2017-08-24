
#include "config.h"
#include "system.h"
#include "comp_adc.h"
#include "IPMC.h"
#include "ShMC.h"
#include "version.h"
#include "comp_uart.h"

struct system_data sys_data;
extern IPMC_DATA_AREA local_data_pool;
extern const TEMP_FAN_LEVEL_t fanLevelCtrlTb[3];
extern DEV_UART devuart4;

unsigned char  BUILDDATE[]=__DATE__ "," __TIME__;

//关闭所有中断 
__asm void INT_DISABLE(void) 
{ 
    CPSID I;	    
} 
//开启所有中断 
__asm void INT_ENABLE(void) 
{ 
    CPSIE I;	    
}

int srvAmfDelay(int tick)
{
    int i = tick;
    while(i--)
        ;
}
void srvAmfEarlyInit()
{
    memset((unsigned char*)&sys_data, 0, sizeof(struct system_data));
    memset((unsigned char*)&local_data_pool, 0, sizeof(IPMC_DATA_AREA));
    sys_data.sys_vtg_cur = (VOLTAGE_MONITOR_t *)&local_data_pool.sensorStaTab.vtg.TLR_MEM0_0V75;
    sys_data.sys_temp    = &local_data_pool.sensorStaTab.temp[0];
    sys_data.power      = &local_data_pool.sensorStaTab.power;
    usartBufferInit();

}

void srvAmfLoadDef()
{
    
}

void srvAmfWaitBit()
{
    while(!(sys_data.task_timer_flg & AMF_SYS_TIMER_FLG_400MS))
    {
        sys_data.task_timer_flg &= ~AMF_SYS_TIMER_FLG_400MS;
    }

}
void srvAmfStart()
{
    //enable sys_cycle
    cfgSysTickTimerEnable(1);
    //enable adc convert
    ADC_SoftwareStartConv(ADC1);
  //  IWDG_Enable();                       //启动看门狗  
    USART2->CR1 |= (1 << 5);//enable rx interrupt
  //  ADC1->CR1   |=  ( 1UL <<  5);         /* enable EOC interrupt               */

}
void vUartCmcAnalize(void * pvParameters)
{
    while(1)
    {
        if(sys_data.uart2msg)
        {
            uartCmdAnalize(0);
            sys_data.uart2msg--;
            sys_data.uart2msg %= USART_RX_BUFFR_CNT;
        }
        
        vTaskDelay(5);
    }
}
void vDebugLedTask(void * pvParameters)
{
    while (1)
    {
        AMF_IPMC_DEBUGLED_REVERSE;
        AMF_SYS_LED_OOS_OFF;

        if(GX36_heart_beat())
            AMF_SYS_LED_IS_ON;
        else
            AMF_SYS_LED_IS_OFF;

        I2C_recfg (I2C1);
        I2C_recfg (I2C2);
        I2C_recfg (I2C3);
        max6581ReadTemp();
        flush_IO();
        vTaskDelay(500);
    }
}
void vSensorScanTask(void * pvParameters)
{
    while (1)
    {

        adcValToHumanInterface();
        isl68201ReadFpgaPower();
        //adcCmpLimit();
        //tmpCmpLimit();
        /* Suspend this task until is resume by end of adc convertion. */
        //vTaskSuspend(xHandle[xTASK_SENSOR_SCAN]);
        vTaskDelay(100);
    }
}

void vPowerManageTask(void * pvParameters)
{
    vTaskDelay(1000);
    while(1)
    {
        ipmcPowerManagement();
        vTaskDelay(4);
    }

}

void vCaseManagemene(void * pvParameters)
{
    while(1)
    {
        //check temperature alarm table
        shmcFanProc();
        vTaskDelay(1000);
    }

}
void vFishEcho(void * pvParameters)
{
    while(1)
    {
        fish_main();
        flush_IO();
        vTaskDelay(15);
    }
}
void vHeartBeatDect(void * pvParameters)
{
    static unsigned char hb;
    while(1)
    {
        hb = local_data_pool.gx36HeartBeat;
        vTaskDelay(2500);
        if(local_data_pool.gx36HeartBeat != hb)
            local_data_pool.sensorStaTab.cpuHealth = 1;
        else
            local_data_pool.sensorStaTab.cpuHealth = 0;
            
    }
}

void srvAmfDC12vOn(void)
{
    int i;
    //GPIOG->BSRRH = 1UL <<  11;//12v DC
    GPIOG->ODR |= 1UL <<  11;
    //delay a little bit
    i=1000;
    while(i--)
        ;

}


void srvAmfDC12vIpmcUpCtrOn(void)
{
    int i;
    //GPIOG->BSRRH = 1UL <<  11;//12v DC
    GPIOE->ODR |= 1UL <<  3;
    i=0xffffff;
    while(--i);
        ;
}


void srvAmfDC12vIpmcUpCtrOff(void)
{
    int i;
    //GPIOG->BSRRH = 1UL <<  11;//12v DC
    GPIOE->ODR &= ~(1UL <<  3);
    i=1000;
    while(i--)
        ;
}


void srvAmfDC12vOff(void)
{
    GPIOG->BSRRH |= 1UL <<  11;//12v DC
}


void srvAmfPayloadOn(void)
{
    //GPIOB->BSRRH = 1UL <<  13;//payload
    GPIOF->BSRRL |= 1UL <<  15;

}

void srvAmfPayloadOff(void)
{
    GPIOF->BSRRH |= 1UL <<  15;//payload
}



#define DISPLAY_INFO                           \
"=======================================\r\n\
DEV   VOTAGE VAL   DEV   VOTAGE VAL \r\n\r\n\
FP01  %0.2f[1V0]   FP02  %0.2f[1V0] \r\n\
FP03  %0.2f[1V0]   FP04  %0.2f[1V0] \r\n\
FP05  %0.2f[1V0]   FP06  %0.2f[1V0] \r\n\
FP1V2 %0.2f        FP3V3 %0.2f \r\n\
FP1V5 %0.2f(DDR)   FP1V8 %0.2f \r\n\
FP0V75  %0.2f(MEM0)\r\n\
MB_1V8  %0.2f      MB_1V2  %0.2f \r\n\
MB_5V0  %0.2f      MB_3V3  %0.2f \r\n\
MB_3V3  %0.2f(AUX) MB2V5   %0.2f \r\n\
MB12V   %0.2f \r\n\
PLXX4_1V0    %0.2f \r\n\
PLXX8_1V0    %0.2f \r\n\
TLR_1V5 DDR0 %0.2f \r\n\
TLR0V75 MEM0 %0.2f \r\n\
TLR0V95 CORE %0.2f \r\n\
ISL_VCORE_IMON %0.2f \r\n\
------------\r\n\
Board temp:\r\n\
TLR_Co   %3d  TLR_En  %3d\r\n\
FP1_Co   %3d  FP1_En  %3d\r\n\
FP2_Co   %3d  FP2_En  %3d\r\n\
FP3_Co   %3d  FP3_En  %3d\r\n\
FP4_Co   %3d  FP4_Co  %3d\r\n\
FP5_Co   %3d  FP5_Co  %3d\r\n\
FP6_Co   %3d  \r\n\
Left_in  %3d  Right_in   %3d  \r\n\
TLR_out   %3d\r\n\
Fpga Power:  %d %d %d %d %d %d   W\r\n\
Board running:  %4d - %02d:%02d:%02d\r\n\
------------\r\n\
Board Name: %s\r\n\
BS:    %s\r\n\
HW:    %s\r\n\
BUILD: %s\r\n\
SV:    %s\r\n\
\
\r\n" 


//human interface
void srvAmfDisplay(void)
{
	  printf(DISPLAY_INFO, sys_data.sys_vtg_cur->FP1_DC1V0/100.0, sys_data.sys_vtg_cur->FP2_DC1V0/100.0,
	                       sys_data.sys_vtg_cur->FP3_DC1V0/100.0, sys_data.sys_vtg_cur->FP4_DC1V0/100.0,
	                       sys_data.sys_vtg_cur->FP5_DC1V0/100.0, sys_data.sys_vtg_cur->FP6_DC1V0/100.0,
                           sys_data.sys_vtg_cur->FP1V2/100.0    , sys_data.sys_vtg_cur->FP3V3/100.0,
                           sys_data.sys_vtg_cur->FP_MEM0_1V5/100.0,  sys_data.sys_vtg_cur->FP1V8/100.0,
	                       sys_data.sys_vtg_cur->FP_MEM0_0V75/100.0, sys_data.sys_vtg_cur->DC1V8/100.0,
	                       sys_data.sys_vtg_cur->DC1V2/100.0,        sys_data.sys_vtg_cur->MB5V/100.0,           sys_data.sys_vtg_cur->MB3V3/100.0,
	                       sys_data.sys_vtg_cur->AUX3V3/100.0,       sys_data.sys_vtg_cur->MB2V5/100.0,          sys_data.sys_vtg_cur->DC12V/100.0,
	                       sys_data.sys_vtg_cur->PLX4_1V0/100.0,     sys_data.sys_vtg_cur->PLX8_1V0/100.0,       sys_data.sys_vtg_cur->TLR_MEM0_1V5/100.0,
	                       sys_data.sys_vtg_cur->TLR_MEM0_0V75/100.0, sys_data.sys_vtg_cur->TLR_CORE_0V95/100.0, sys_data.sys_vtg_cur->ISL_VCORE_IMON/100.0,
	                       sys_data.sys_temp[9],  sys_data.sys_temp[0],  
	                       sys_data.sys_temp[1],  sys_data.sys_temp[2],
	                       sys_data.sys_temp[3],  sys_data.sys_temp[4],
	                       sys_data.sys_temp[6],  sys_data.sys_temp[5],
	                       sys_data.sys_temp[10], sys_data.sys_temp[11],
	                       sys_data.sys_temp[13], sys_data.sys_temp[12],
	                       sys_data.sys_temp[14], sys_data.sys_temp[15],
                           sys_data.sys_temp[8],  sys_data.sys_temp[7],
	                       sys_data.fpgapower[0],sys_data.fpgapower[1],sys_data.fpgapower[2],sys_data.fpgapower[3],sys_data.fpgapower[4],sys_data.fpgapower[5],
	                       sys_data.clock_cycle/8640000,sys_data.clock_cycle/100%86400/3600,sys_data.clock_cycle/100%3600/60,sys_data.clock_cycle/100%60,
	                       BDNAME, BDSER, BDVER, BUILDDATE, IPMC_VER_STR);
   return;
}


int srvBoardInit()
{
    srvAmfEarlyInit();
    bootloader_stage1();
    srvAmfPayloadOff();
    srvAmfLoadDef();
    srvAmfStart();
    
    ipmcInit();
    ShMCInit();
    fish_init();
    

}


