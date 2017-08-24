//BSP support for AMF board.
//This startup.c file mainly contain two stages:
// 1, config hardware
// 2, loader user application
#include "stm32f2xx.h"

#include "config.h"
#include "system.h"
#include "comp_gpio.h"
#include "comp_uart.h"
#include "string.h"
#include "stm32f2xx_adc.h"
#include "stm32f2xx_rcc.h"
#include "stm32f2xx_dma.h"
#include "stm32f2xx_tim.h"

#define ADC_CDR_ADDRESS    ((uint32_t)0x40012308)
#define IPMC_I2C_ADDRESS  0x30  
#define SYSTEM_TICK_TIMER TIM3
#define SYSTEM_OC_TIMER TIM4
#define SYSTEM_CPLD1K_TIMER TIM2

extern __IO char uart_cache[100];
extern DEV_UART devuart4;
extern void SystemInit(void);
extern void usartBufferInit(void);

int cfgSysClk()
{
// use default config for system clock.
// default setting:
// PLLM:25, PLLN 240, PLLP:2, PLLQ:5 HSE:25
// Fvoc = HSE * (PLLN/PLLM)
// Fsys  =Fvoc / PLLP
//    /* HCLK = SYSCLK / 1*/
/* PCLK1 = HCLK / 4*/
/* PCLK2 = HCLK / 2*/
//    SystemInit();
    return 0;
}
//HOWTO:
//in order to enable an periph clock, we need to enable 3 rcc config register
// 1, enable periph clock
// 2, enable GPIOx clock
// we do all this in config_sys_rcc function
int cfgSysRcc()
{
    //ENable all gpio clock

    RCC->AHB1ENR |= 0xff;//refer to reference manual, RCC_AHB1ENR low 9ibts for gpio0-8
    //ENable periph clock
    //ADC clock
    RCC->APB2ENR |= (7UL <<   8);//Enable ADC1, ADC2, ADC3
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC3 , ENABLE);  
    //serial
    RCC->APB1ENR  |= (1UL << 19);
    //dma clock
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
    //timer1
    RCC->APB2ENR |= (1UL << 0);
    //timer4
    RCC->APB1ENR |= (1UL << 2);
    //timer2
    RCC->APB1ENR |= (1UL << 0);
    //timer3
    RCC->APB1ENR |= (1UL << 1);
    //timer8
    RCC->APB2ENR |= (1UL << 1);
    //i2c2 clock
    RCC->APB1ENR |= (1UL << 22);
    //i2c1 clock
    RCC->APB1ENR |= (1UL << 21);
    RCC->APB1ENR |= (1UL << 23);
    //uart4 clock
    RCC->APB1ENR  |= (1UL << 19);

    //usart2 clock
    RCC->APB1ENR  |= (1UL << 17);

    //syscfg
    RCC->APB2ENR  |= (1UL << 14);
    return 0;
}


int cfgSysNvic()
{

// NVIC_EnableIRQ(TIM2_IRQn);
// NVIC_EnableIRQ(UART4_IRQn);

    NVIC_InitTypeDef  NVIC_InitStructure;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;	
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;	   
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
    NVIC_Init(&NVIC_InitStructure);			

    NVIC_InitStructure.NVIC_IRQChannel = ADC_IRQn;	
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	   
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
    NVIC_Init(&NVIC_InitStructure);			

    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream4_IRQn;	
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;	   
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
    NVIC_Init(&NVIC_InitStructure);	
    
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;	
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;	   
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
    NVIC_Init(&NVIC_InitStructure);	


    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0xF;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0xE;	   
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
    NVIC_Init(&NVIC_InitStructure);	
    return 0;
}


int cfgPeripGpio()
{
    gpioCfg(gpio_a, GPIOA, 0);
    gpioCfg(gpio_b, GPIOB, 1);
    gpioCfg(gpio_c, GPIOC, 2);
    gpioCfg(gpio_d, GPIOD, 3);
    gpioCfg(gpio_e, GPIOE, 4);
    gpioCfg(gpio_f, GPIOF, 5);
    gpioCfg(gpio_g, GPIOG, 6);
    gpioCfg(gpio_h, GPIOH, 7);
   
    return 0;
}

// HOWTO:
// In order to enable a periph, we need to
// 1, enable periph(do this in config_sys_rcc); 
// 2, config pin in or out(done by config_periph_gpio);
// 3, config different IO feature, which is done by each config_periph_xxx.


/* UART config routine baudrate, date bit, stop bit, parity */
int cfgPeriphUart()
{
/* Configure UART4: 115200 baud @ 42MHz, 8 bits, 1 stop bit, no parity      */
//uart 在apb1总线上，PCLK1 = HCLK / 4 = 30M
//波特率计算方法
// baud_rate=PCLK2/(16*USARTDIV),
// PCLK在本系统中为:30M
// USARTDIV = 30M/(115200 *16)=16.27604166666667
// BRR[15:4]=16
//BRR[3:0]=16*0.276041 = 5
    UART4->BRR = (16 << 4) | 5;
    UART4->CR3 = 0x0000;
    UART4->CR2 = 0x0000;
    UART4->CR1 = 0x200C;
    UART4->CR1 |= (1 << 5);//enable rx interrupt
    //Enable uart4 Rx DMA transfer data.
    USART_DMACmd(UART4, USART_DMAReq_Tx , ENABLE);


    UART5->BRR = (16 << 4) | 5;
    UART5->CR3 = 0x0000;
    UART5->CR2 = 0x0000;
    UART5->CR1 = 0x200C;

    //usart2 
    //IPMC connect with GX36
    USART2->BRR = (195 << 4) | 5;
    USART2->CR3 = 0x0000;
    USART2->CR2 = 0x0000;
    USART2->CR1 = 0x200C;
    USART2->CR1 |= (1 << 5);//enable rx interrupt

    return 0;
}

int cfgPeriphSpi()
{
    return 0;
}


//routine:
// 1, ENable adc clock(done by config_sys_rcc)
// 2, ENable corresponding gpio pin(done by gpio_config)
// 3, 
int cfgPeriphAd()
{
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_InitTypeDef ADC_InitStructure;
#ifdef DEBUG
    RCC->APB2ENR |= (1UL <<   8);         /* Enable ADC1 clock                  */
    RCC->AHB1ENR |= (1UL <<   2);         /* Enable GPIOC clock                 */
    GPIOC->MODER |= (3UL << 2*2); 
    ADC1->SQR1 = 0;
    ADC1->SQR2 = 0;
    ADC1->SQR3 = (12 << 0);//channel 12, 1st conversion
    ADC1->SMPR1  =  ( 7UL <<  6);
    ADC1->SMPR2  =   0; 
    ADC1->CR1    |=  ( 1UL <<  8);

   ADC1->CR1   |=  ( 1UL <<  5);         /* enable EOC interrupt               */
//   NVIC_EnableIRQ(ADC_IRQn); 
   ADC1->CR2 |= 0x1;//set ADON = 1
#endif

    //ADC common init
    //ADC_CommonInitStructure.ADC_Mode = ADC_DualMode_RegSimult;
    ADC_CommonInitStructure.ADC_Mode                     =  ADC_TripleMode_RegSimult;
    ADC_CommonInitStructure.ADC_Prescaler               = ADC_Prescaler_Div8;
    ADC_CommonInitStructure.ADC_DMAAccessMode     = ADC_DMAAccessMode_1;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);
    //set ADC channel 1,2,3
     
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 8;
    ADC_Init(ADC1, &ADC_InitStructure);
    ADC_Init(ADC2, &ADC_InitStructure);
    ADC_Init(ADC3, &ADC_InitStructure);

  
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_3Cycles);//
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_3Cycles);//
    ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_3Cycles);//
    ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4, ADC_SampleTime_3Cycles);//
    ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 5, ADC_SampleTime_3Cycles);//
    ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 6, ADC_SampleTime_3Cycles);//
    ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 7, ADC_SampleTime_3Cycles);//
    ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 8, ADC_SampleTime_3Cycles);//
    ADC_Cmd(ADC1, ENABLE);
    
    ADC_RegularChannelConfig(ADC2, ADC_Channel_8,  1, ADC_SampleTime_3Cycles);//
    ADC_RegularChannelConfig(ADC2, ADC_Channel_9,  2, ADC_SampleTime_3Cycles);//
    ADC_RegularChannelConfig(ADC2, ADC_Channel_10, 3, ADC_SampleTime_3Cycles);//
    ADC_RegularChannelConfig(ADC2, ADC_Channel_11, 4, ADC_SampleTime_3Cycles);//
    ADC_RegularChannelConfig(ADC2, ADC_Channel_12, 5, ADC_SampleTime_3Cycles);//
    ADC_RegularChannelConfig(ADC2, ADC_Channel_13, 6, ADC_SampleTime_3Cycles);//
    ADC_RegularChannelConfig(ADC2, ADC_Channel_14, 7, ADC_SampleTime_3Cycles);//
    ADC_RegularChannelConfig(ADC2, ADC_Channel_15, 8, ADC_SampleTime_3Cycles);//
    ADC_Cmd(ADC2, ENABLE);
   
    ADC_RegularChannelConfig(ADC3, ADC_Channel_9,  1, ADC_SampleTime_3Cycles);//
    ADC_RegularChannelConfig(ADC3, ADC_Channel_14, 2, ADC_SampleTime_3Cycles);//
    ADC_RegularChannelConfig(ADC3, ADC_Channel_15, 3, ADC_SampleTime_3Cycles);//
    ADC_RegularChannelConfig(ADC3, ADC_Channel_4,  4, ADC_SampleTime_3Cycles);//
    ADC_RegularChannelConfig(ADC3, ADC_Channel_5,  5, ADC_SampleTime_3Cycles);//
    ADC_RegularChannelConfig(ADC3, ADC_Channel_6,  6, ADC_SampleTime_3Cycles);//
    ADC_RegularChannelConfig(ADC3, ADC_Channel_7,  7, ADC_SampleTime_3Cycles);//
    ADC_RegularChannelConfig(ADC3, ADC_Channel_8,  8, ADC_SampleTime_3Cycles);//
    ADC_Cmd(ADC3, ENABLE);
  
    ADC_MultiModeDMARequestAfterLastTransferCmd(ENABLE);
    ADC_SoftwareStartConv(ADC1);
    
    return 0;
}


int cfgPeriphDMA()
{
    DMA_InitTypeDef DMA_InitStructure;
    /* Config Dma2 stream0 channel_0 for ADC data transfer. */
    DMA_InitStructure.DMA_Channel = DMA_Channel_0; 
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&sys_data.aADCTripleConvertedValue;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC_CDR_ADDRESS;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_InitStructure.DMA_BufferSize = 24;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;         
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA2_Stream0, &DMA_InitStructure);
  
    /* DMA2_Stream0 enable */ 
    DMA_Cmd(DMA2_Stream0, ENABLE);

    DMA_DeInit(DMA1_Stream2);
    /*Config Dma1 stream2 channel 4 for UART4 RX data transfer; channel 1 for UART4 RX data transfer*/
    DMA_InitStructure.DMA_Channel = DMA_Channel_4; 
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)uart_cache;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(0x40004c04);//UART4 Rx data register
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_InitStructure.DMA_BufferSize = 16;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;//DO NOT USE FIFO MODE!!!         
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    //DMA_Init(DMA1_Stream2, &DMA_InitStructure);/* Do Not config as DMA receive. */


    /*Config Dma1 stream1 channel 4 for UART4 TX data transfer; channel 1 for UART4 TX data transfer*/
    DMA_InitStructure.DMA_Channel = DMA_Channel_4;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)devuart4.pTxConsumer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_BufferSize = PRINTF_IO_FLUSH_SIZE;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_Init(DMA1_Stream4, &DMA_InitStructure);
    /* Clear all interrupt flags. */
    DMA_ClearFlag(DMA1_Stream4, DMA_FLAG_TCIF4 | DMA_FLAG_HTIF4 | DMA_FLAG_TEIF4 | DMA_FLAG_DMEIF4 | DMA_FLAG_FEIF4);
    DMA_ITConfig(DMA1_Stream4, DMA_IT_TC, ENABLE);

    //DMA_Cmd(DMA1_Stream4, ENABLE);/* Tx */
    DMA_Cmd(DMA1_Stream2, ENABLE);/* Rx */
    return 0;
}

int cfgPeriphI2c()
{
//  I2C_InitTypeDef   I2C_InitStructure;
//  I2C_DeInit(I2C2);
    RCC->APB1RSTR |= 1<<22;//复位i2c2
    RCC->APB1RSTR &= ~(1 << 22);


    I2C2->CR1 = 1 << 15;//复位
    I2C2->CR1 &= ~(1 << 15);
    I2C2->CR1 |= 1 << 0;//启用I2C模块，要在使能ACK之前配置
    I2C2->CR2 |= 30 << 0;//I2C2输入时钟频率设置为30MHz
    //I2C2->CCR |= 1 << 15;//设置成快速模式
    //I2C2->CCR |= 1 << 14;//占空比设置为16/9
    I2C2->CCR |= 150 << 0;//
    I2C2->TRISE |= 31 << 0;//设置主模式时的最大上升时间（标准模式为1000ns，快速为300ns，超快为120ns）
    I2C2->CR1 |= 1 << 10;//应答使能（ACK）
    //I2C2->CR1 |= 1 << 6;//广播
    // I2C2->OAR1 |= 0 << 15;//7位地址模式
    // I2C2->OAR1 |= 1 << 14;//必须由软件保持为1
    // I2C2->OAR1 |= 0 << 0;//设置接口地址
    // I2C2->CR2 |= 1 << 9;//使能事件中断

//I2C1
//config speed 100k
//	pclk1 = 30M
//I2Cx->CR2 |= tmpreg => (pclk1/1M)
//if pclk1/(100K<<1) >=4
//I2Cx->CCR |= tmpreg => pclk1/(100K<<1)
//else
//I2Cx->CCR |= 4 
//I2Cx->TRISE = freqrange + 1; freqrange=>(pclk1/1M)

    RCC->APB1RSTR |= 1<<21;
    RCC->APB1RSTR &= ~(1 << 21);
    I2C1->CR1 = 1 << 15;
    I2C1->CR1 &= ~(1 << 15);
    I2C1->CR1 |= 1 << 0;
    I2C1->CR2 |= 30 << 0;
    I2C1->CCR |= 150 << 0;
    I2C1->TRISE |= 31 << 0;
    I2C1->CR1 |= 1 << 10;

//I2C3
    RCC->APB1RSTR |= 1<<23;
    RCC->APB1RSTR &= ~(1 << 23);
    I2C3->CR1 = 1 << 15;
    I2C3->CR1 &= ~(1 << 15);
    I2C3->CR1 |= 1 << 0;
    I2C3->CR2 |= 30 << 0;
    I2C3->CCR |= 150 << 0;
    I2C3->TRISE |= 31 << 0;
    I2C3->CR1 |= 1 << 10;

    return 0;
}


int cfgPeriphGPtimer()//general purpose
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    //TIM1 和TIM8为高级定时器，时钟源为APB, 主频为12M
    //要得到一个1024的PWM,TIM_Prescaler = 11，那么分频后的时钟为 120M/(11+1)=10M, 10M/1024 = 9764即为TIM_Period的值。
//     TIM_TimeBaseStructure.TIM_Period = 9764;//
//     TIM_TimeBaseStructure.TIM_Prescaler = 11;//
//     TIM_TimeBaseStructure.TIM_ClockDivision = 0;
//     TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//     
//     TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
//     
//     /* PWM1 Mode configuration: Channel4 */
//     TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
//     TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//     TIM_OCInitStructure.TIM_Pulse = 4882;//50%
//     TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
//     
//     TIM_OC4Init(TIM1, &TIM_OCInitStructure);
//     
//     TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);
//     TIM_ARRPreloadConfig(TIM1, ENABLE);
// 
//     /* TIM1 enable counter */
//     TIM_Cmd(TIM1, ENABLE);
//     TIM_CtrlPWMOutputs(TIM1, ENABLE);

    //普通定时器计算方法：
    //系统时钟/2/(TIM_Prescaler+1) = 分频时钟,120/2=60/(5+1)=10M
    //分频时钟/TIM_Period = 输出频率（即每个中断发生时间间隔）
    //PWM

    //      ETR
     TIM_TimeBaseStructure.TIM_Prescaler = 0x00; 
     TIM_TimeBaseStructure.TIM_Period = 0xFFFF; 
     TIM_TimeBaseStructure.TIM_ClockDivision = 0x0; 
     TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
     TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);  // Time base configuration 
     
     TIM_ETRClockMode2Config(TIM1, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0);   
     TIM_SetCounter(TIM1, 0);    
     TIM_Cmd(TIM1, ENABLE); 
    return 0;
}

int cfgPeriphWd()//watch dog timer
{
   IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);//启动寄存器读写
   IWDG_SetPrescaler(IWDG_Prescaler_32);//32K时钟32分频,，时钟源为32K
   IWDG_SetReload(1000);                 //计数器数值
   IWDG_ReloadCounter();             //重启计数器
  // IWDG_Enable();
   return 0;
}

int cfgPeriphExit()
{
    EXTI_InitTypeDef   EXTI_InitStructure;
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, 0);//PE0
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;  
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
}

void cfgPeriphExit5Disable()
{
    EXTI_InitTypeDef   EXTI_InitStructure;
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, 0);//PE0
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
    EXTI_InitStructure.EXTI_LineCmd = DISABLE;
    EXTI_Init(&EXTI_InitStructure);
}
void cfgSysTickTimer(TIM_TypeDef* tim)
{
  
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  TIM_TimeBaseStructure.TIM_Period = 10000;//100 HZ
  TIM_TimeBaseStructure.TIM_Prescaler = 59;//1M
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(tim, &TIM_TimeBaseStructure);
  TIM_Cmd(tim, ENABLE);
  TIM_ITConfig(tim, TIM_IT_Update, ENABLE);
}

void cfgCpld1KhzTimer(TIM_TypeDef* tim)
{
    
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    TIM_TimeBaseStructure.TIM_Period = 9764;
    TIM_TimeBaseStructure.TIM_Prescaler = 5;//10M
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(tim, &TIM_TimeBaseStructure);
    
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 4882;//50%
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    
    TIM_OC4Init(tim, &TIM_OCInitStructure);
    
    TIM_OC4PreloadConfig(tim, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(tim, ENABLE);

    TIM_Cmd(tim, ENABLE);
    TIM_CtrlPWMOutputs(tim, ENABLE);

}
void cfgOcTimer(TIM_TypeDef* tim)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    TIM_TimeBaseStructure.TIM_Period = 40;//
    TIM_TimeBaseStructure.TIM_Prescaler = 59;//
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    
    TIM_TimeBaseInit(tim, &TIM_TimeBaseStructure);
    
    /* PWM1 Mode configuration: Channel2 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 20;//50%
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    
    TIM_OC3Init(tim, &TIM_OCInitStructure);
    
    TIM_OC3PreloadConfig(tim, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(tim, ENABLE);

    /* TIM4 enable counter */
    TIM_Cmd(tim, ENABLE);
    TIM_CtrlPWMOutputs(tim, ENABLE);

}
void cfgSysTickTimerEnable(unsigned char en)
{
    if(en)
        TIM_ITConfig(SYSTEM_TICK_TIMER, TIM_IT_Update, ENABLE);
    else
        TIM_ITConfig(SYSTEM_TICK_TIMER, TIM_IT_Update, DISABLE);
}
int cfgSysIo()
{//this might contain all useable periph in the board
    
    cfgPeriphUart();
    cfgPeriphSpi();
    cfgPeriphDMA();
    cfgPeriphAd();
    cfgPeriphI2c();
    cfgPeriphGPtimer();
    cfgSysTickTimer(SYSTEM_TICK_TIMER);
    cfgOcTimer(SYSTEM_OC_TIMER);
    cfgCpld1KhzTimer(SYSTEM_CPLD1K_TIMER);
    cfgPeripGpio();
    cfgPeriphWd();
    cfgPeriphExit();

    return 0;
}

int cfgSysInit()
{
    memset((unsigned char*)&sys_data, 0, sizeof(struct system_data));
    return 0;
}


void bootloader_stage1()
{
   //config_sys_init();
 
    cfgSysClk();
    cfgSysNvic();
    cfgSysRcc();
    cfgSysIo();

}








/*EOF*/

