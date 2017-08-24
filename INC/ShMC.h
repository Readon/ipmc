

#ifndef __SHMC__H__
#define __SHMC__H__


#define FRU_IDENITIFY_IPMC 0
#define FRU_IDENITIFY_SHMC 1

#define FRU_STATE_S0 0
#define FRU_STATE_S1 1
#define FRU_STATE_S2 2
#define FRU_STATE_S3 3
#define FRU_STATE_S4 4
#define FRU_STATE_S5 5
#define FRU_STATE_S6 6
#define FRU_STATE_S7 7


#define ICMM_IN_THE_TOP_LEFT_FAN_TRAY                  0x42
#define ICMM_IN_THE_TOP_RIGHT_FAN_TRAY                 0x40
#define ICCM_FAN_TRAY_GROUP   2


#define GET_HOTSWAP_STATE                          0x01
#define GET_ALARM                                  0x02
#define GET_FAN_SENSOR_COUNT                       0x03
#define GET_FAN_LEVEL                              0x04
#define GET_FAN_SPEED                              0x05
#define GET_FAN_SPEED_HIGH_THRESHOLD               0x06
#define GET_FAN_SPEED_LOW_THRESHOLD                0x07
#define GET_FAN_SPEED_IGH_HYSTERESIS               0x08
#define GET_FAN_SPEED_LOW_HYSTERESIS               0x09
#define GET_TEMPERATURE_SENSOR_COUNT               0x0A
#define GET_TEMPERATURE                            0x0B
#define GET_TEMPERATURE_HIGH_THRESHOLD             0x0C
#define GET_TEMPERATURE_LOW_THRESHOLD              0x0D
#define GET_TEMPERATURE_HIGH_HYSTERESIS            0x0E
#define GET_TEMPERATURE_LOW_HYSTERESIS             0x0F
#define SET_FAN_LEVEL                              0x10
#define ICMM_ACTIVEATE                                 0x11
#define ICMM_DEACTIVATE                                0x12
#define ICMM_REBOOT                                    0x13
#define SET_PARAMETERS_TO_FLASH                    0x14
#define GET_PSU_CFG_AND_STATUS                     0x15
#define GET_48DC_VOLTAGE_SENSOR_COUNT              0x16
#define GET_48DC_VOLTAGE                           0x17
#define GET_48DC_VOLTAGE_HIGH_THRESHOLD            0x18
#define GET_48DC_VOLTAGE_LOW_THRESHOLD             0x19
#define GET_48DC_VOLTAGE_HIGH_HYSTERESIS           0x1A
#define GET_48DC_VOLTAGE_LOW_HYSTERESIS            0x1B
#define GET_AC_DC_CONVERTERS_1                     0x1C
#define GET_AC_DC_CONVERTERS_2                     0x1D
#define GET_AC_DC_CONVERTERS_3                     0x1E
#define GET_AC_DC_CONVERTERS_4                     0x1F
#define GET_CHASSIS_TYPE                           0x20
#define GET_CHASSIS_ID                             0x21
#define SET_CHASSIS_ID                             0x22

#define MAX_ICMM_CMD_LEN 12

typedef struct iccm_cmd
{
    unsigned char dstaddr;
    unsigned char cmd[MAX_ICMM_CMD_LEN];
}ICCM_CMD_t;

typedef struct temp_fan_level 
{
    unsigned int fanLevel;
    unsigned int temperature;
}TEMP_FAN_LEVEL_t;

typedef struct fan_addr
{
    unsigned char    addr[ICCM_FAN_TRAY_GROUP];
}FAN_TRAY_ADDR_t;
typedef struct fan_manage
{
    TEMP_FAN_LEVEL_t fanLev[3];
    FAN_TRAY_ADDR_t    fanAddr;
}ICCM_FAN_t;

typedef struct ipmb_0
{
    I2C_TypeDef *cur_ipmb_bus;
    I2C_TypeDef *ipmb0[2];
    char useless;// mark for useless bus, bit0 for ipmb-a, bit1 for ipmb-b, other reserve.
}IPMB_0_BUS_t;

char ICMMFanLevelSet(int level);


#endif
/*EOF*/

