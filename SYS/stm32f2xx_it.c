
/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx_it.h"
#include "config.h"
#include "system.h"
#include "ipmc.h"
#include "shmc.h"
extern IPMC_DATA_AREA local_data_pool;
extern void srvAmfDC12vOff(void);
/** @addtogroup Template_Project
  * @{
  */



/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}



/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
}

//pow_pb_l
void EXTI9_5_IRQHandler(void)
{
}

void EXTI0_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line0) == SET)
    {
            sys_data.pb = 1;
            EXTI_ClearITPendingBit(EXTI_Line0);

    }

}

void TIM1_CC_IRQHandler(void)
{
}

//every 10ms occur a interrupt
void TIM3_IRQHandler(void)        //rIPMC sensor scan task
{	
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    sys_data.clock_cycle ++;
	
    if(AMF_SYS_400MS == 0)
    {
        //AMF_IPMC_OOSLED_REVERSE;
        sys_data.task_timer_flg |= AMF_SYS_TIMER_FLG_400MS;
        //IWDG_ReloadCounter();
    }
    if(AMF_SYS_500MS == 0)
    {
        sys_data.task_timer_flg |= AMF_SYS_TIMER_FLG_500MS;
        //AMF_IPMC_DEBUGLED_REVERSE;
    }
    if(AMF_SYS_1S == 0)
    {
         sys_data.task_timer_flg |= AMF_SYS_TIMER_FLG_1S;
    }
    if(AMF_SYS_2S == 0)
    {
         sys_data.task_timer_flg |= AMF_SYS_TIMER_FLG_2S;
    }
    if(AMF_SYS_5S == 0)
    {
         sys_data.task_timer_flg |= AMF_SYS_TIMER_FLG_5S;
    }

}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
