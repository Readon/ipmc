/*config file*/

//CPU型号配置，调试信息配置，编译器配置，操作系统运行组件配置，

#ifndef __CONFIG_H__
#define __CONFIG_H__

//#define DEBUG 
//#define RELEASE
#define DEBUG_LED_GPIO  GPIOD
#define DEBUG_LED_PIN    gpio_pin_13

#define IPMC_DEVSCRIPTION "IPMC> "

#define IPMC_TEMP_ALARM   65
#define IPMC_TEMP_EMERGENCY 85

#define MAX_PIN_GROUP 8

#define AMF_IPMC_DEBUGLED_REVERSE do{GPIOE->ODR ^= 0x20;}while(0)// 0000 0010 0000
#define AMF_IPMC_OOSLED_REVERSE do{GPIOG->ODR ^= 0x80;}while(0)
#define AMF_IPMC_HSLED_REVERSE do{GPIOE->ODR ^= 0x2;}while(0)


//clock configure
//#define QIUOS_TIME_SLOT  5ms

//#define QIUOS_DEBUG

#define SYS_TERMINAL_UART UART4

#endif
/*EOF*/

