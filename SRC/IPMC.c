
#include"system.h"
#include"IPMC.h"
#include"ShMC.h"
#include"errcode.h"


IPMC_DATA_AREA local_data_pool;
static unsigned long active_cycle = 0, inactive_cycle = 0;

unsigned long get_current_cycle()
{
    return sys_data.clock_cycle;
}

int ipmcGetCaseType()
{
    int ret;
    char cmd[3]={0xAA,0xAA,0x20};
    char buf[10]={0,0,0,0,0,0,0,0,0,0};
    //enable i2c bus
    GPIOF->BSRRL |= 1UL <<  14;
    GPIOF->BSRRL |= 1UL <<  12;
    ret = i2c_dev_write(I2C3, ICMM_IN_THE_TOP_RIGHT_FAN_TRAY, cmd, 3);
    ret = i2c_dev_read(I2C3, ICMM_IN_THE_TOP_RIGHT_FAN_TRAY, buf, 4);
    if(ret < 0)
    {
        ret = i2c_dev_write(I2C1, ICMM_IN_THE_TOP_RIGHT_FAN_TRAY, cmd, 3);
        ret = i2c_dev_read(I2C1, ICMM_IN_THE_TOP_RIGHT_FAN_TRAY, buf, 4);
    }
    if(ret < 0)
        return 0;
  //  switch((buf[1] >>2) & 0x1F)
    // 01 1111= 0x7C
  //  {
  //      case 1:// 3U2slot
        
  //  }
    //Disable i2c bus
    GPIOF->BSRRH |= 1UL <<  14;
    GPIOF->BSRRH |= 1UL <<  12;
    return ((buf[1] >>2) & 0x1F);

}


int GX36_heart_beat()
{
    return local_data_pool.sensorStaTab.cpuHealth;
}
int ipmcSetupSlot(void)
{
    unsigned char hardaddr;
    hardaddr = ((GPIOE->IDR) >> 7) & 0xFF;
    //read gpio PE[14:7], it is hard address
    local_data_pool.fruAddr.hardAddr = hardaddr&0x1 | (hardaddr&0x2)<<3 |(hardaddr&0x4)>>1 | (hardaddr&0x8)<<2 \
                                        |(hardaddr&0x10)>>2 |(hardaddr&0x20)<<1 |(hardaddr&0x40)>>3 |(hardaddr&0x80);
    local_data_pool.fruAddr.hardAddr &= 0x7F;
    local_data_pool.fruAddr.i2cAddr = local_data_pool.fruAddr.hardAddr << 1;
    local_data_pool.fruAddr.logicSlotId = local_data_pool.fruAddr.hardAddr - 0x40;
    return 0;
}

int ipmcGetMySlot(void)
{
    return 1;
}
 
int ipmbFruStatMachine()
{
    char curFruSta;
    char curFruActEn;
    
    curFruSta = local_data_pool.sensorStaTab.fruSta;
    curFruActEn = local_data_pool.fruActEn;
    if(curFruSta == FRU_STATE_S1)/*s1->s2的变迁通过两种方式实现。1，插入单板。2，pow键长按。不能通过软件的方式变迁。*/
    {
        local_data_pool.sensorStaTab.cpuHealth = 0;
        local_data_pool.sensorAlarmTab.other |= 0x02;
        local_data_pool.alarmFlag = 1;
        local_data_pool.fruActEn = 0;
        srvAmfPayloadOff();
        srvAmfDC12vOff();
        srvAmfDC12vIpmcUpCtrOff();
    }
    if((curFruActEn == 0) && (curFruSta == FRU_STATE_S2))/*去激活*/
    {
           local_data_pool.sensorStaTab.cpuHealth = 0;
           local_data_pool.sensorAlarmTab.other |= 0x02;
           local_data_pool.alarmFlag = 1;
           srvAmfDC12vOn();
           srvAmfPayloadOff();
           
    }
    if((curFruActEn == 1) && (curFruSta == FRU_STATE_S2))/*请求激活*/
    {
        srvAmfDC12vIpmcUpCtrOn();
        srvAmfDC12vOn();
        srvAmfPayloadOn();
        active_cycle = get_current_cycle();
        local_data_pool.sensorStaTab.fruSta = FRU_STATE_S3;//s3, activing
    }
     if((curFruActEn == 0) && (curFruSta == FRU_STATE_S3))/*去激活请求*/
    {
        srvAmfPayloadOff();
        inactive_cycle = get_current_cycle();
        local_data_pool.sensorStaTab.fruSta = FRU_STATE_S5;
    }
    if((curFruActEn == 1) && (curFruSta == FRU_STATE_S3))/*激活中*/
    {
       //check heart beat
       if(GX36_heart_beat())
       {
           local_data_pool.sensorStaTab.cpuHealth = 1;
           local_data_pool.sensorAlarmTab.other &= (~0x02);

           local_data_pool.sensorStaTab.fruSta = FRU_STATE_S4;
       }
       if(get_current_cycle() - active_cycle > 6000)
       {
           local_data_pool.sensorStaTab.cpuHealth = 0;
           local_data_pool.sensorAlarmTab.other |= 0x02;
           local_data_pool.alarmFlag = 1;
       }
    }

    if((curFruActEn == 0) && (curFruSta == FRU_STATE_S4))/*请求去激活*/
    {
        srvAmfPayloadOff();
       inactive_cycle = get_current_cycle();
       //local_data_pool.fruActEn = 0;//this must already been 1
       local_data_pool.sensorStaTab.fruSta = FRU_STATE_S5;
    }
    if((curFruActEn == 1) && (curFruSta == FRU_STATE_S4))/*已激活*/
    {
        local_data_pool.sensorStaTab.cpuHealth = 1;
        local_data_pool.sensorAlarmTab.other &= (~0x02);
    }
    if((curFruActEn == 0) && (curFruSta == FRU_STATE_S5))/*去激活中*/
    {
        //这里默认去激活的过程是稳定的

        local_data_pool.sensorStaTab.cpuHealth = 0;
        local_data_pool.sensorAlarmTab.other |= 0x02;
        local_data_pool.alarmFlag = 1;
        
        local_data_pool.sensorStaTab.fruSta = FRU_STATE_S2;
    }
    if((curFruActEn == 1) && (local_data_pool.sensorStaTab.fruSta == FRU_STATE_S5))/*请求激活*/
    {
        local_data_pool.sensorStaTab.cpuHealth = 0;
        local_data_pool.sensorAlarmTab.other |= 0x02;
        local_data_pool.alarmFlag = 1;

        local_data_pool.sensorStaTab.fruSta = FRU_STATE_S3;

        active_cycle = sys_data.clock_cycle;
    }
    return 0;
}

/**
 fruActEn,只能由ShMC修改，所有单板请求激活、去激活都应上报ShMC
 fruSta由ipmcPowerManagement函数管理
状态变迁:
1, 修改CPU健康状态，告警状态
2，修改电源状态
3, 如果是激活中、去激活中，检查是否超时
4, 如果是请求激活状态，记录当前的cycle

**/
int ipmcPowerManagement()
{
    //power button
    #if RAM_DEBUG
    if(sys_data.forcecomeup == 0)
    {
        sys_data.forcecomeup = 1;
        HDS_PW1_ON;
        HDS_PW2_ON;
        srvAmfPayloadOn();
        sys_data.mstate = 1;
    }
    #endif
    if(sys_data.pb)
    {
        if((GPIOE->IDR | 0xFFFE) == 0xFFFE)
        {
            sys_data.pb ++;
        }
        else
        {
            sys_data.pb --;
        }
        if(sys_data.pb >= 600)
        {
            sys_data.pb = 0;
            if(sys_data.mstate == 0)//if power off
            { 
                HDS_PW1_ON;
                HDS_PW2_ON;
                srvAmfPayloadOn();
            }
            else
            {
                HDS_PW1_OFF;
                HDS_PW2_OFF;
                srvAmfPayloadOff();
            }
            sys_data.mstate ^= 1;
        }
    }
    return 0;

}
int ipmcInit()
{
    ipmcSetupSlot();
    AMF_SYS_LED_HS_OFF;
    AMF_SYS_LED_OOS_ON;
    return 0;
}

unsigned char ipmcGetShmcSlot()
{
    return 1;    /* treat slot 1 as shmc slot 09-02*/
}

unsigned char ipmcCheckAlarmTlb()
{
    unsigned char buf[32];
    buf[0] = 0xaa;//magic0
    buf[1] = 0x55;//magic1
    buf[2] = 0x01;//srcslot
    buf[3] = 0x01;//dstslot
    buf[4] = 4;   //valid date length
    
    buf[5] |= (1<<5);

    if(local_data_pool.sensorAlarmTab.vtg != 0)
    {
        buf[4] = 9;
        buf[6] = (IPMC_ALARM_DATA_AREA<<4) | IPMC_SENSOR_TYPE_VTG_CUR;
        buf[7] = 0xFF;
        *((int *)(buf+8)) = local_data_pool.sensorAlarmTab.vtg;
        local_data_pool.sensorAlarmTab.vtg = 0;
        if(GX36_heart_beat())
        send_buf(USART2, buf, 14);
    }
    if(local_data_pool.sensorAlarmTab.temp)
    {
        buf[4] = 6;
        buf[6] = (IPMC_ALARM_DATA_AREA<<4) | IPMC_SENSOR_TYPE_TEMPERATURE;
        buf[7] = 0xFF;
        buf[8] = local_data_pool.sensorAlarmTab.temp;
        if(GX36_heart_beat())
        send_buf(USART2, buf, 11);
        local_data_pool.sensorAlarmTab.temp = 0;
    }
  //  if(local_data_pool.sensorAlarmTab.other)
 //   {
 //       buf[4] = 6;
 //       buf[6] = (2<<4) | 2;
 //       buf[7] = 0xFF;
 //       buf[8] = local_data_pool.sensorAlarmTab.other;
 //       send_buf(USART2, buf, 11);
 //       local_data_pool.sensorAlarmTab.other = 0;
//
 //   }
    return OK;
}



 

/*EOF*/


