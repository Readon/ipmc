
#include "stdio.h"
#include <stm32f2xx.h>
#include "system.h"
#include "config.h"
#include "comp_gpio.h"

/*gpio config table*/
const struct gpio_attribute gpio_a[16]=
{{ENABLE,gpio_mode_AN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_0*/
{ENABLE,gpio_mode_AN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_1*/
{ENABLE,gpio_mode_AN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_2*/
{ENABLE,gpio_mode_AN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_3*/
{ENABLE,gpio_mode_AN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_4*/
{ENABLE,gpio_mode_AN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_5*/
{ENABLE,gpio_mode_AN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_6*/
{ENABLE,gpio_mode_AN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_7*/
{ENABLE,gpio_mode_AF,gpio_otyper_open_drain,gpio_ospeedr_50M,gpio_pupdr_none,gpio_af4_i2c1_i2c3},/*pin_8*/
{ENABLE,gpio_mode_IN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_9*/
{ENABLE,gpio_mode_IN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_10*/
{DISABLE,gpio_mode_AF, gpio_otyper_push_pull, gpio_ospeedr_50M, gpio_pupdr_pull_up, gpio_af1_tim1_tim2},/*pin_11*/
{1,gpio_mode_AF, gpio_otyper_push_pull, gpio_ospeedr_50M, gpio_pupdr_pull_up, gpio_af1_tim1_tim2},/*pin_12*/
{0,0,0,0,0,0},/*pin_13!!!!*/
{0,0,0,0,0,0},/*pin_14!!!!*/
{0,0,0,0,0,0},/*pin_15!!!!*/

};


const struct gpio_attribute gpio_b[16]=
{{ENABLE,gpio_mode_AN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_0*/
{ENABLE,gpio_mode_AN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_1*/
{0,0,0,0,0,0},/*pin_2*/
{0,0,0,0,0,0},/*pin_3*/
{0,0,0,0,0,0},/*pin_4*/
{0,0,0,0,0,0},/*pin_5*/
{ENABLE, gpio_mode_AF,gpio_otyper_open_drain,gpio_ospeedr_50M,gpio_pupdr_none,gpio_af4_i2c1_i2c3},/*pin_6*/
{ENABLE, gpio_mode_AF,gpio_otyper_open_drain,gpio_ospeedr_50M,gpio_pupdr_none,gpio_af4_i2c1_i2c3},/*pin_7*/
{ENABLE, gpio_mode_AF, gpio_otyper_push_pull, gpio_ospeedr_50M, gpio_pupdr_pull_up, gpio_af2_tim3_tim5},/*pin_8*/
{0,0,0,0,0,0},/*pin_9*/
{ENABLE, gpio_mode_OP, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_10*/
{ENABLE, gpio_mode_AF, gpio_otyper_push_pull, gpio_ospeedr_50M, gpio_pupdr_pull_up, gpio_af1_tim1_tim2},/*pin_11*/
{ENABLE, gpio_mode_OP, gpio_otyper_push_pull,  gpio_ospeedr_2M,  gpio_pupdr_none, gpio_af0_system},/*pin_12*/
{DISABLE,gpio_mode_OP, gpio_otyper_push_pull, gpio_ospeedr_2M,   gpio_pupdr_none, gpio_af0_system},/*pin_13*/
{ENABLE, gpio_mode_IN, gpio_otyper_push_pull, gpio_ospeedr_2M,   gpio_pupdr_none, gpio_af0_system},/*pin_14*/
{ENABLE, gpio_mode_IN, gpio_otyper_push_pull, gpio_ospeedr_2M,   gpio_pupdr_none, gpio_af0_system},/*pin_15*/

};


const struct gpio_attribute gpio_c[16]=
{{ENABLE,gpio_mode_AN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_0*/
{ENABLE,gpio_mode_AN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_1*/
{ENABLE,gpio_mode_AN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_2*/
{ENABLE,gpio_mode_AN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_3*/
{ENABLE,gpio_mode_AN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_4*/
{ENABLE,gpio_mode_AN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_5*/
{ENABLE,gpio_mode_AF, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af3_tim8_tim11},/*pin_6*/
{ENABLE,gpio_mode_IN, 0,0,0,0},/*pin_7*/
{ENABLE,gpio_mode_IN, 0,0,0,0},/*pin_8*/
{ENABLE, gpio_mode_AF,gpio_otyper_open_drain,gpio_ospeedr_50M,0,gpio_af4_i2c1_i2c3},/*pin_9*/
{ENABLE,gpio_mode_AF,0,0,gpio_pupdr_none,gpio_af8_usart4_usart6},/*pin_10*/
{ENABLE,gpio_mode_AF,0,0,gpio_pupdr_none,gpio_af8_usart4_usart6},/*pin_11*/
{DISABLE,gpio_mode_AF,0,0,gpio_pupdr_none,gpio_af8_usart4_usart6},/*pin_12*/
{DISABLE,gpio_mode_OP,0,0,0,0},/*pin_13*/
{0,0,0,0,0,0},/*pin_14*/
{0,0,0,0,0,0},/*pin_15*/

};

const struct gpio_attribute gpio_d[16]=
{{ENABLE,gpio_mode_OP, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_0*/
{ENABLE,gpio_mode_IN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_1*/
{ENABLE,gpio_mode_AF, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af2_tim3_tim5},/*pin_2*/
{0,0,0,0,0,0},/*pin_3*/
{0,0,0,0,0,0},/*pin_4*/
{ENABLE,gpio_mode_AF,0,0,gpio_pupdr_none,gpio_af7_usart1_usart3},/*pin_5*/
{ENABLE,gpio_mode_AF,0,0,gpio_pupdr_none,gpio_af7_usart1_usart3},/*pin_6*/
{ENABLE,gpio_mode_OP, gpio_otyper_push_pull, gpio_ospeedr_50M, gpio_pupdr_pull_up, gpio_af0_system},/*pin_7*/
{ENABLE,gpio_mode_IN,0,0,0,0},/*pin_8*/
{ENABLE,gpio_mode_IN,0,0,0,0},/*pin_9*/
{ENABLE,gpio_mode_OP,0,0,0,0},/*pin_10*/
{ENABLE,gpio_mode_OP,0,0,0,0},/*pin_11*/
{ENABLE,gpio_mode_AF,gpio_otyper_push_pull,gpio_ospeedr_50M,gpio_pupdr_pull_up,gpio_af2_tim3_tim5},/*pin_12*/
{ENABLE,gpio_mode_OP,0,0,0,0},/*pin_13*/
{DISABLE,gpio_mode_OP,0,0,0,0},/*pin_14*/
{DISABLE,gpio_mode_OP,0,0,0,0},/*pin_15*/

};


const struct gpio_attribute gpio_e[16]=
{{ENABLE,gpio_mode_IN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_pull_up, gpio_af0_system},/*pin_0*/
{ENABLE,gpio_mode_OP, gpio_otyper_push_pull, gpio_ospeedr_50M, gpio_pupdr_pull_up, gpio_af0_system},/*pin_1*/
{0,0,0,0,0,0},/*pin_2*/
{ENABLE,gpio_mode_OP, gpio_otyper_push_pull, gpio_ospeedr_50M, gpio_pupdr_pull_up, gpio_af0_system},/*pin_3*/
{0,0,0,0,0,0},/*pin_4*/
{ENABLE,gpio_mode_OP,0,0,gpio_pupdr_pull_up,0},/*pin_5*/
{ENABLE,gpio_mode_IN,0,0,gpio_pupdr_pull_up,0},/*pin_6*/
{ENABLE,gpio_mode_IN,0,0,0,0},/*pin_7*/
{ENABLE,gpio_mode_IN,0,0,0,0},/*pin_8*/
{ENABLE,gpio_mode_IN,0,0,0,0},/*pin_9*/
{ENABLE,gpio_mode_IN,0,0,0,0},/*pin_10*/
{ENABLE,gpio_mode_IN,0,0,0,0},/*pin_11*/
{ENABLE,gpio_mode_IN,0,0,0,0},/*pin_12*/
{ENABLE,gpio_mode_IN,0,0,0,0},/*pin_13*/
{0,gpio_mode_AF, gpio_otyper_push_pull, gpio_ospeedr_50M, gpio_pupdr_pull_up, gpio_af1_tim1_tim2},/*pin_14*/
{DISABLE,gpio_mode_OP,0,0,0,0},/*pin_15*/

};

const struct gpio_attribute gpio_f[16]=
{{ENABLE, gpio_mode_AF,gpio_otyper_open_drain,gpio_ospeedr_50M,0,gpio_af4_i2c1_i2c3},/*pin_0*/
{ENABLE, gpio_mode_AF,gpio_otyper_open_drain,gpio_ospeedr_50M,0,gpio_af4_i2c1_i2c3},/*pin_1*/
{0,0,0,0,0,0},/*pin_2*/
{ENABLE,gpio_mode_AN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_3*/
{ENABLE,gpio_mode_AN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_4*/
{ENABLE,gpio_mode_AN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_5*/
{ENABLE,gpio_mode_AN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_6*/
{ENABLE,gpio_mode_AN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_7*/
{ENABLE,gpio_mode_AN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_8*/
{ENABLE,gpio_mode_AN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_9*/
{ENABLE,gpio_mode_AN, gpio_otyper_push_pull, gpio_ospeedr_2M, gpio_pupdr_none, gpio_af0_system},/*pin_10*/
{ENABLE,gpio_mode_IN,0,0,0,0},/*pin_11*/
{ENABLE,gpio_mode_OP,0,0,0,0},/*pin_12*/
{ENABLE,gpio_mode_IN,0,0,0,0},/*pin_13*/
{ENABLE,gpio_mode_OP,0,0,0,0},/*pin_14*/
{ENABLE,gpio_mode_OP,0,0,0,0},/*pin_15*/

};

const struct gpio_attribute gpio_g[16]=
{{ENABLE,gpio_mode_IN,0,0,0,0},/*pin_0*/
{ENABLE,gpio_mode_IN,0,0,0,0},/*pin_1*/
{ENABLE,gpio_mode_IN,0,0,0,0},/*pin_2*/
{ENABLE,gpio_mode_IN,0,0,0,0},/*pin_3*/
{0,0,0,0,0,0},/*pin_4*/
{0,0,0,0,0,0},/*pin_5*/
{ENABLE,gpio_mode_IN,0,0,gpio_pupdr_pull_up,0},/*pin_6*/
{ENABLE,gpio_mode_OP, gpio_otyper_push_pull, gpio_ospeedr_50M, gpio_pupdr_pull_up, gpio_af0_system},/*pin_7*/
{0,0,0,0,0,0},/*pin_8*/
{0,0,0,0,0,0},/*pin_9*/
{ENABLE,gpio_mode_OP, gpio_otyper_push_pull, gpio_ospeedr_50M, gpio_pupdr_pull_up, gpio_af0_system},/*pin_10*/
{ENABLE,gpio_mode_OP, gpio_otyper_push_pull, gpio_ospeedr_50M, gpio_pupdr_pull_up, gpio_af0_system},/*pin_11*/
{ENABLE,gpio_mode_OP,0,0,0,0},/*pin_12*/
{ENABLE,gpio_mode_OP,0,0,0,0},/*pin_13*/
{ENABLE,gpio_mode_IN,0,0,0,0},/*pin_14*/
{ENABLE,gpio_mode_OP,0,0,0,0},/*pin_15*/

};

const struct gpio_attribute gpio_h[]={
{0,0,0,0,0,0},
{0,0,0,0,0,0}
};
/*end of gpio config table*/




//FIXEDME:
// 0503: In gpio_config_group, we config gpio pin mode, ospeedr, ptyper, pupdr, af

int gpioCfg(const struct gpio_attribute *gpio_grp, GPIO_TypeDef *GPIOx, unsigned char group_num)
{
    char i = 0;
    unsigned int gpio_mode;
    unsigned int gpio_otyper;
    unsigned int gpio_ospeedr;
    unsigned int gpio_pupdr;
    unsigned int gpio_af[2];

    struct gpio_attribute *gpio_group = NULL;
    gpio_group = (struct gpio_attribute *)gpio_grp;
    if(gpio_group == NULL)
        return -1;
    if(group_num > MAX_PIN_GROUP)
        return -1;
    for(i=0; i<16; i++)
    {
        if(gpio_group[i].setbit)
        {
            sys_data.gpio_mode[group_num]    |= gpio_group[i].mode<<(2*i);
            sys_data.gpio_ospeedr[group_num] |= gpio_group[i].ospeedr<<(2*i);
            sys_data.gpio_otyper[group_num]   |= gpio_group[i].otyper<<i;
            sys_data.gpio_pupdr[group_num]    |= gpio_group[i].pupdr<<(2*i);
            if(i < 8)
                sys_data.gpio_af[0][group_num] |= gpio_group[i].af<<(4*i);
            else
                sys_data.gpio_af[1][group_num] |= gpio_group[i].af<<(4*(i-8));
        }
    }
    GPIOx->OSPEEDR    = sys_data.gpio_ospeedr[group_num];
    GPIOx->OTYPER     = sys_data.gpio_otyper[group_num];
    GPIOx->PUPDR      = sys_data.gpio_pupdr[group_num];
    GPIOx->AFR[0]     = sys_data.gpio_af[0][group_num];//set to 1
    GPIOx->AFR[1]     = sys_data.gpio_af[1][group_num];
    if(group_num == 0)
    {
        GPIOx->MODER     |= sys_data.gpio_mode[group_num];//be very carefully here. PA15, PA14 & PA13 are used for debuging, DO NOT change default pin moder.
    }
    else
    {
        GPIOx->MODER     = sys_data.gpio_mode[group_num];//set to 0
    }
    return 0;
}



/*EOF*/
