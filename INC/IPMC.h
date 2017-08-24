


#ifndef __IPMC_H__
#define __IPMC_H__

#include"comp_adc.h"


#define FRU_SHMC0_HARDADDR 0x42

#define IPMC_STATUS_DATA_AREA 0
#define IPMC_ALARM_DATA_AREA  1
#define IPMC_LIMIT_DATA_AREA   2
#define IPMC_SHMC                3
#define IPMC_AREA_ALL           255

#define IPMC_SENSOR_TYPE_RESERVE        0
#define IPMC_SENSOR_TYPE_TEMPERATURE   1
#define IPMC_SENSOR_TYPE_VTG_CUR       2
#define IPMC_SENSOR_TYPE_FRU_STAT       4
#define IPMC_SENSOR_TYPE_POWER          5
#define IPMC_SENSOR_TYPE_CPU_HEALTH     6
#define IPMC_SENSOR_TYPE_HOTSWAP        7
#define IPMC_SENSOR_TYPE_IPMC_VER       0xb

#define SHMC_SHELFADDR                   1
#define SHMC_CASEADDR                    2
#define SHMC_LOGICSLOTID                   3
#define SHMC_CASE_VOLTAGE                4
#define SHMC_CASE_TEMPERATURE           5
#define SHMC_CASE_FAN                    6
#define SHMC_CASE_TYPE                    7



#define SIZEOF_SERIAL_HEAD    sizeof(SERIAL_HEAD)
#define SIZEOF_SERIAL_BODY    sizeof(SERIAL_BODY)
#define SIZEOF_SERIAL_PKT       (sizeof(SERIAL_HEAD) + sizeof(SERIAL_BODY))
#define NUM_OF_TEMP_SENSOR   16
#define NUM_OF_VTG_SENSOR    24
//
typedef struct IPMC_sensor_status_t{

	signed int temp[NUM_OF_TEMP_SENSOR];       /*  status area: indicate status value  */
	VOLTAGE_MONITOR_t vtg;     /*   indicate voltage or current value */
	int power;      /**/
	char hotSwap; 
	char cpuHealth;
	char fruSta;
}IPMC_SENSOR_STA;

typedef struct IPMC_sensor_alarm_t{
	unsigned short temp;  /* bit 7-0: each bit indicate one temp alarm status: bit7 temp[7], bit6 temp[6] …
	                      0 no alarm 1- alarm */
	char other;          /*bit 7-3 reserved  bit 2   hotSwap;bit 1   cpuHealth;bit 0   fruSta;    */
	int  vtg;  /* bit 32-0: each bit indicate one voltage or current alarm status: 
	                        bit23 power[23], bit22 power[22] …bit0 power[0],other: reserve
	                        0 no alarm 1- alarm   */

}IPMC_SENSOR_ALARM;

typedef struct IPMC_sensor_limit_t{
	signed int temp[NUM_OF_TEMP_SENSOR];      
	VOLTAGE_MONITOR_t vtg;        //limit值为真实值 x 100
}IPMC_SENSOR_LMT;

typedef struct FRU_addr_t{
	    char shelfAddr;      /* system shelf address */
	    char caseAddr;      /* system case address */
	    char phySlotId;      /* physical slot ID in system */
		char logicSlotId;     /* logic slot ID in system*/
		char hardAddr;       /* hard address in system*/
	    char i2cAddr;       /* I2C address */
	    char caseType; 
} FRU_ADDR_AREA;

typedef struct IPMC_data_t{
	IPMC_SENSOR_STA sensorStaTab;     /* sensor status table */
	IPMC_SENSOR_ALARM sensorAlarmTab;   /* sensor alarm table */
	IPMC_SENSOR_LMT sensorLimitTab;   /* sensor limit table */
	FRU_ADDR_AREA fruAddr;           /* FRU address */
	char fruActEn;                       /* ShMC send CMD of active or inavtive of FRU*/
	char alarmFlag;       /* indicate weather exist alarm in seneor alarm table
	                              as long as any of member of sensorAlarmTab exist one '1', alaramFlag=1;
	                                all of member of sensorAlarmTab are '0', alarmFlag=0*/
	unsigned char fruMSRShMCIdentify;          /*
	                                           1,master ShMC;0 IPMC
	                                           */
	unsigned char gx36HeartBeat;
	
} IPMC_DATA_AREA;





extern IPMC_DATA_AREA local_data_pool;

#endif
/*EOF*/

