#include <stm32f2xx_i2c.h>


#include"ShMC.h"
#include"IPMC.h"
#include"system.h"

#define IPMB_A I2C3
#define IPMB_B I2C1

#define SHMC_DEBUG 
#define NORMAL 0
#define ALARM  1
#define EMERG  2
#define FIRST_RECVBYTE_AS_LENGTH   0

extern IPMC_DATA_AREA local_data_pool;
IPMB_0_BUS_t ipmbBus;
FAN_TRAY_ADDR_t fanTrayAddr;


char IPMBEnable();
char IPMBDisable();
int shmcIccmGet(unsigned char cmd, unsigned int *data);


//3 group for :normal, alarm, emergency

const TEMP_FAN_LEVEL_t fanLevelCtrlTb[3]=
{
#ifdef SHMC_DEBUG
    {40,6000},
#elif
    {55,6000},
#endif
    {80,8000},
    {100,9000}
};

ICCM_CMD_t casecmd;


u8 const crc8_tab[]={
0x00, 0x07,  0xE, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31,
0x24, 0x23, 0x2A, 0x2D, 0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D, 0xE0, 0xE7, 0xEE, 0xE9,
0xFC, 0xFB, 0xF2, 0xF5, 0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1,
0xB4, 0xB3, 0xBA, 0xBD, 0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA, 0xB7, 0xB0, 0xB9, 0xBE,
0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16,
0x03, 0x04, 0x0D, 0x0A, 0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A, 0x89, 0x8E, 0x87, 0x80,
0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8,
0xDD, 0xDA, 0xD3, 0xD4, 0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44, 0x19, 0x1E, 0x17, 0x10,
0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F,
0x6A, 0x6D, 0x64, 0x63, 0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13, 0xAE, 0xA9, 0xA0, 0xA7,
0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8, 0xEF,
0xFA, 0xFD, 0xF4, 0xF3
};

static unsigned char CRC8_Table(unsigned char *p, char counter)
{
  unsigned char crc8 = 0;
  for( ; counter > 0; counter --)
  {
    crc8 = crc8_tab[crc8 ^ *p];
    p++;
  }
  return (crc8);
}


char ShMCInit()
{
    //enable IPMB-0
    unsigned int caseType;
    
    ipmbBus.ipmb0[0] = IPMB_A;
    ipmbBus.ipmb0[1] = IPMB_B;
    ipmbBus.cur_ipmb_bus = ipmbBus.ipmb0[0];
    ipmbBus.useless = 0;
    fanTrayAddr.addr[0] = ICMM_IN_THE_TOP_LEFT_FAN_TRAY;
    fanTrayAddr.addr[1] = ICMM_IN_THE_TOP_RIGHT_FAN_TRAY;

    IPMBEnable();
    shmcIccmGet(GET_CHASSIS_TYPE, &caseType);
    switch((caseType>>2 & 0x1F))
    {
        case 1: //_ 3U2slot
        case 2:
        case 5:
        if(local_data_pool.fruAddr.hardAddr == 0x42)// logic 1 as shmc
            local_data_pool.fruMSRShMCIdentify = FRU_IDENITIFY_SHMC;
        break;
        case 3:
        case 4:
        case 6:
            if(local_data_pool.fruAddr.hardAddr == 0x44)// logic 9 as shmc
                local_data_pool.fruMSRShMCIdentify = FRU_IDENITIFY_SHMC;
        break;
        default:break;
    }
    if(local_data_pool.fruMSRShMCIdentify == FRU_IDENITIFY_SHMC)
        AMF_SYS_LED_SHMC_ON;
    //IPMBDisable();
    
    memset(casecmd, 0x0, sizeof(casecmd));
   // ICMMFanLevelSet(fanLevelCtrlTb[NORMAL].fanLevel);
    return 0;
}

int ShMCSwitchIPMB0Bus()
{
    int ret = -1;
    char ipmbidx = 0;
    cfgPeriphI2c();//try to reconfig the i2c bus
    // make current ipmb as useless
    if(ipmbBus.cur_ipmb_bus == ipmbBus.ipmb0[0])
        ipmbidx = 0;
    else
        ipmbidx = 1;
    ipmbBus.useless |= 1<<ipmbidx;
    switch(ipmbBus.useless)
    {
        case 0x00:
        ipmbBus.cur_ipmb_bus = ipmbBus.ipmb0[0];
        I2C_ClearFlag(ipmbBus.ipmb0[1], I2C_FLAG_AF);
        ret = 0;
        break;
        case 0x01:
        ipmbBus.cur_ipmb_bus = ipmbBus.ipmb0[1];
        I2C_ClearFlag(ipmbBus.ipmb0[0], I2C_FLAG_AF);
        ret = 0;
        break;
        case 0x03:
        I2C_ClearFlag(ipmbBus.ipmb0[0], I2C_FLAG_AF);
        I2C_ClearFlag(ipmbBus.ipmb0[1], I2C_FLAG_AF);
        ipmbBus.useless = 0;
        break;
        default:break;
    }
    return ret;
}
//==========board management===========



//==========case management===========

char IPMBEnable()
{
    int i;
    GPIOF->BSRRL |= 1UL <<  14;
    GPIOF->BSRRL |= 1UL <<  12;
    i = 0x1F000;
    while(i--)
    ;
    return 0;
}
char IPMBDisable()
{
    GPIOF->BSRRH |= 1UL <<  14;
    GPIOF->BSRRH |= 1UL <<  12;
    return 0;
}



//@cmd : command of ICCM
//@data: the place to stor read data, high 16bits for fan tray left, low 16bits for fan tray right.
//return:0-OK  other-error
int shmcIccmGet(unsigned char cmd, unsigned int *data)
{
    int ret = 0;
    char i;
    ICCM_CMD_t *cmdl = &casecmd;
    char buf[2][12];
    cmdl->cmd[0] = 0xAA;
    cmdl->cmd[1] = 0xAA;
    cmdl->cmd[2] = cmd;
    for(i = 0; i < ICCM_FAN_TRAY_GROUP; i++)
    {
        ret = i2c_dev_write(ipmbBus.cur_ipmb_bus, fanTrayAddr.addr[i], cmdl->cmd, 3);
        ret = i2c_dev_read(ipmbBus.cur_ipmb_bus, fanTrayAddr.addr[i], buf[i], FIRST_RECVBYTE_AS_LENGTH);
        if(ret < 0)
        {
            //switch to the other ipmb bus.
            ret = ShMCSwitchIPMB0Bus();
            if(ret < 0)
                return ret;
            ret = i2c_dev_write(ipmbBus.cur_ipmb_bus, fanTrayAddr.addr[i], cmdl->cmd, 3);
            ret = i2c_dev_read(ipmbBus.cur_ipmb_bus, fanTrayAddr.addr[i], buf[i], FIRST_RECVBYTE_AS_LENGTH);
        }
    }
    if(ret < 0)
        goto err;
    switch(cmd)
    {
        case GET_HOTSWAP_STATE:
        case GET_ALARM:
        case GET_TEMPERATURE_SENSOR_COUNT:
        case GET_TEMPERATURE:// imp
        case GET_48DC_VOLTAGE_SENSOR_COUNT:
        case GET_48DC_VOLTAGE://imp
            *data = buf[0][1]<<16 | buf[1][1];
        break;
        case GET_FAN_LEVEL://imp
        *data = buf[0][2]<<16 | buf[1][2];
        break;
        case GET_CHASSIS_ID:
        break;
        case GET_FAN_SENSOR_COUNT:
        break;
        case GET_FAN_SPEED:
        break;
        case GET_FAN_SPEED_HIGH_THRESHOLD:
        break;
        case GET_FAN_SPEED_LOW_THRESHOLD:
        break;
        case GET_FAN_SPEED_IGH_HYSTERESIS:
        break;
        case GET_FAN_SPEED_LOW_HYSTERESIS:
        break;
        case GET_TEMPERATURE_HIGH_THRESHOLD:
        break;
        case GET_TEMPERATURE_LOW_THRESHOLD:
        break;
        case GET_TEMPERATURE_HIGH_HYSTERESIS:
        break;
        case GET_TEMPERATURE_LOW_HYSTERESIS:
        break;
        case GET_PSU_CFG_AND_STATUS:
        break;
        case GET_48DC_VOLTAGE_HIGH_THRESHOLD:
        break;
        case GET_48DC_VOLTAGE_LOW_THRESHOLD:
        break;
        case GET_48DC_VOLTAGE_HIGH_HYSTERESIS:
        break;
        case GET_48DC_VOLTAGE_LOW_HYSTERESIS:
        break;
        case GET_AC_DC_CONVERTERS_1:
        break;
        case GET_AC_DC_CONVERTERS_2:
        break;
        case GET_AC_DC_CONVERTERS_3:
        break;
        case GET_AC_DC_CONVERTERS_4:
        break;
        case GET_CHASSIS_TYPE:
            *data = buf[0][1];
        break;
        default:break;
    }
err:
    return ret;

}

int shmcCaseSet(unsigned char cmd, unsigned int *data)
{
    int ret;
    int len, i;
    int fanLevel = 0;
    ICCM_CMD_t *cmdl = &casecmd;
    cmdl->cmd[0] = 0xAA;
    cmdl->cmd[1] = 0xAA;
    cmdl->cmd[2] = cmd;
    switch(cmd)
    {
        case SET_FAN_LEVEL:
            cmdl->cmd[3] = 0x3;
            cmdl->cmd[4] = 0x0;
            cmdl->cmd[5] = *data;
            len = 6;
            for(i = 0; i < ICCM_FAN_TRAY_GROUP; i++)
            {
                cmdl->dstaddr = fanTrayAddr.addr[i];
                cmdl->cmd[len] = CRC8_Table((unsigned char *)cmdl, len+1);
                ret = i2c_dev_write(ipmbBus.cur_ipmb_bus, fanTrayAddr.addr[i], cmdl->cmd, len+1);
                if(ret < 0)
                {
                    ret = ShMCSwitchIPMB0Bus();
                    if(ret < 0)
                        return ret;
                    ret = i2c_dev_write(ipmbBus.cur_ipmb_bus, fanTrayAddr.addr[i], cmdl->cmd, len+1);
                }
            }
            if(ret < 0)
                goto err;
            shmcIccmGet(GET_FAN_LEVEL, &fanLevel);
            if((fanLevel&0xFF) != *data)
                return -1;
            
        break;
        case ICMM_ACTIVEATE:
        case ICMM_DEACTIVATE:
        case ICMM_REBOOT:
        break;
        case SET_PARAMETERS_TO_FLASH:
            cmdl->cmd[3] = 0x8;
            cmdl->cmd[4] = 0x0;
            cmdl->cmd[5] = 40;
            cmdl->cmd[6] = 0x40;
            cmdl->cmd[7] = 0x42;
           /// cmdl->cmd[8] = ;
           // cmdl->cmd[9] = ;
           // cmdl->cmd[10] = ;
            cmdl->cmd[11] = CRC8_Table((unsigned char *)cmdl, len+1);;

        break;
        case SET_CHASSIS_ID:
        break;

        default:break;
    }
    
err:
    return ret;
}

char ICMMFanSensorCnt()
{
    ICCM_CMD_t *cmdl = &casecmd;
    char buf[10]={0,0,0,0};
    cmdl->cmd[0] = 0xAA;
    cmdl->cmd[1] = 0xAA;
    cmdl->cmd[2] = 0x03;
    
    i2c_dev_write(IPMB_A, ICMM_IN_THE_TOP_RIGHT_FAN_TRAY, cmdl->cmd, 3);
    i2c_dev_read(IPMB_A, ICMM_IN_THE_TOP_RIGHT_FAN_TRAY, buf, 4);
    return 0;

}


char ICMMHotSwapState()
{}

char ICMMAlarm()
{}

char ICMMFanLevel(unsigned int *lv)
{
    ICCM_CMD_t *cmdl = &casecmd;
    char buf[5]={0,0,0,0};
    cmdl->cmd[0] = 0xAA;
    cmdl->cmd[1] = 0xAA;
    cmdl->cmd[2] = 0x04;
    i2c_dev_write(IPMB_A, ICMM_IN_THE_TOP_RIGHT_FAN_TRAY, cmdl->cmd, 3);
    i2c_dev_read(IPMB_A, ICMM_IN_THE_TOP_RIGHT_FAN_TRAY, buf, 5);
    *lv = buf[2];
    return 0;
}

char ICMMFanSpeed(unsigned int *spd)
{
    ICCM_CMD_t *cmdl = &casecmd;
    char buf[4]={0,0,0,0};
    cmdl->cmd[0] = 0xAA;
    cmdl->cmd[1] = 0xAA;
    cmdl->cmd[2] = 0x05;
    i2c_dev_write(IPMB_A, ICMM_IN_THE_TOP_RIGHT_FAN_TRAY, cmdl->cmd, 3);
    i2c_dev_read(IPMB_A, ICMM_IN_THE_TOP_RIGHT_FAN_TRAY, buf, 4);
    ICMMFanLevel(spd);
    *spd |= (buf[1]*buf[3])<<16;
    return 0;
}

char ICMMTemperature(unsigned int *temp)
{
    ICCM_CMD_t *cmdl = &casecmd;
    char buf[10]={0,0,0,0};
    cmdl->cmd[0] = 0xAA;
    cmdl->cmd[1] = 0xAA;
    cmdl->cmd[2] = 0x0B;
    
    i2c_dev_write(IPMB_A, ICMM_IN_THE_TOP_RIGHT_FAN_TRAY, cmdl->cmd, 3);
    i2c_dev_read(IPMB_A, ICMM_IN_THE_TOP_RIGHT_FAN_TRAY, buf, 3);
    *temp = buf[1];
    return 0;
}

char ICMMFanLevelSet(int level)
{
    ICCM_CMD_t *cmdl = &casecmd;
    char ret = 0;
    cmdl->dstaddr = ICMM_IN_THE_TOP_RIGHT_FAN_TRAY;
    cmdl->cmd[0] = 0xAA;
    cmdl->cmd[1] = 0xAA;
    cmdl->cmd[2] = 0x10;
    cmdl->cmd[3] = 0x03;
    cmdl->cmd[4] = 0x00;
    cmdl->cmd[5] = level;
    cmdl->cmd[6] = CRC8_Table((unsigned char *)cmdl, 7);

    ret = i2c_dev_write(IPMB_A, ICMM_IN_THE_TOP_RIGHT_FAN_TRAY, cmdl->cmd, 7);
    cmdl->dstaddr = ICMM_IN_THE_TOP_LEFT_FAN_TRAY;
    cmdl->cmd[6] = CRC8_Table((unsigned char *)cmdl, 7);
    ret = i2c_dev_write(IPMB_A, ICMM_IN_THE_TOP_LEFT_FAN_TRAY, cmdl->cmd, 7);
    return ret;
}

//0x17
char ICMM48DCVoltage(unsigned int *vtg)
{
    ICCM_CMD_t *cmdl = &casecmd;
    char buf[4]={0,0,0,0};
    cmdl->cmd[0] = 0xAA;
    cmdl->cmd[1] = 0xAA;
    cmdl->cmd[2] = 0x17;
    i2c_dev_write(IPMB_A, ICMM_IN_THE_TOP_RIGHT_FAN_TRAY, cmdl->cmd, 3);
    i2c_dev_read(IPMB_A, ICMM_IN_THE_TOP_RIGHT_FAN_TRAY, buf, 4);
    *vtg = buf[1]+buf[2]<<8;
}

char ICMMCaseType()
{
    char cmd[3]={0xAA,0xAA,0x20};
    char buf[10]={0,0,0,0,0,0,0,0,0,0};
    i2c_dev_write(IPMB_A, ICMM_IN_THE_TOP_RIGHT_FAN_TRAY, cmd, 3);
    i2c_dev_read(IPMB_A, ICMM_IN_THE_TOP_RIGHT_FAN_TRAY, buf, 4);
    return 0;

}


//it seems caseID is useless.
char ICMMCaseID()
{
    char cmd[3]={0xAA,0xAA,0x21};
    char buf[10]={0,0,0,0,0,0,0,0,0,0};
    i2c_dev_write(IPMB_A, ICMM_IN_THE_TOP_RIGHT_FAN_TRAY, cmd, 3);
    i2c_dev_read(IPMB_A, ICMM_IN_THE_TOP_RIGHT_FAN_TRAY, buf, 4);
    return 0;
}

char ICCMParameterGet()
{
    ICCM_CMD_t *cmdl = &casecmd;
    char buf[8]={0,0,0,0,0,0,0,0}; 
    cmdl->dstaddr = ICMM_IN_THE_TOP_RIGHT_FAN_TRAY;
    cmdl->cmd[0] = 0xAA;
    cmdl->cmd[1] = 0xAA;
    cmdl->cmd[2] = 0x15;
   // cmdl->cmd[3] = CRC8_Table((unsigned char *)cmdl, 4);
    i2c_dev_write(IPMB_A, ICMM_IN_THE_TOP_RIGHT_FAN_TRAY, cmdl->cmd, 3);
    i2c_dev_read(IPMB_A, ICMM_IN_THE_TOP_RIGHT_FAN_TRAY, buf, 8);
    return 0;
}

char ICCMParameterSet()
{
    return 0;
}

//every one second exec
char shmcFanProc()
{
    static unsigned short srvFlg = 0;
    static int srvCnt = 0;
    static char dftSpd = 0;
    char i = 0;
    if(local_data_pool.sensorAlarmTab.temp != 0)
    {
        if(srvFlg != local_data_pool.sensorAlarmTab.temp)
        {
            //check what the alarm level is
            while(!((local_data_pool.sensorAlarmTab.temp >> i) & 0x1) && i<16)
                i++;
            if(local_data_pool.sensorStaTab.temp[i] > fanLevelCtrlTb[EMERG].temperature)
            {
                if(shmcCaseSet(SET_FAN_LEVEL, (int *)&fanLevelCtrlTb[EMERG].fanLevel) == 0)
                {
                    srvFlg = local_data_pool.sensorAlarmTab.temp;
                    srvCnt = 1;//start timer
                }
            }
            else
            {
                if(shmcCaseSet(SET_FAN_LEVEL, (int *)&fanLevelCtrlTb[ALARM].fanLevel) == 0)
                {
                    srvFlg = local_data_pool.sensorAlarmTab.temp;
                    srvCnt = 1;
                }
            }
        }
         
    }
    srvCnt = srvCnt>0?(++srvCnt):0;
    if(srvCnt > 300)//5 minutes
    {
        if(shmcCaseSet(SET_FAN_LEVEL, (int *)&fanLevelCtrlTb[NORMAL].fanLevel)==0)
        {
            srvCnt = 0;
            srvFlg = 0;
        }
    }
    if(srvCnt == 0 && dftSpd == 0)
    {
        if(shmcCaseSet(SET_FAN_LEVEL, (int *)&fanLevelCtrlTb[NORMAL].fanLevel) == 0)
        {
            dftSpd = 1;
        }
    }
    return 0;
}


//==========expertation management===========




/*EOF*/

