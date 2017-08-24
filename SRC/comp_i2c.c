
#include <stm32f2xx.h>
#include <stm32f2xx_i2c.h>
#include "comp_i2c.h"
#include "config.h"
#include "system.h"
#include "errcode.h"
#include "IPMC.h"
#include "ShMC.h"

#define AMF_IPMC_I2CADDR_MAX6581  0x9E
#define AMF_IPMC_I2CADDR_INA219    0x80
extern IPMC_DATA_AREA local_data_pool;
extern const TEMP_FAN_LEVEL_t fanLevelCtrlTb[3];
extern void vTaskEnterCritical( void );
extern void vTaskExitCritical( void );

struct isl_dev{
    I2C_TypeDef* I2C;
    u8 i2caddr;

};
struct isl_dev ISL68021_DEV[] = {
    {I2C3, 0xC0},
    {I2C3, 0xFE},
    {I2C1, 0xC0},
    {I2C2, 0xFE},
    {I2C1, 0xFE},
    {I2C2, 0xC0}
};

/*******************************************************************************
* Function Name  : I2C_delay
* Description    : 延迟时间
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void I2C_delay(uint16_t cnt)
{
	while(cnt--);
}

void I2C_recfg(I2C_TypeDef* I2Cx)
{
    I2Cx->CR1 = 1 << 15;
    I2Cx->CR1 &= ~(1 << 15);
    I2Cx->CR1 |= 1 << 0;
    I2Cx->CR2 |= 30 << 0;
    I2Cx->CCR |= 150 << 0;
    I2Cx->TRISE |= 31 << 0;
    I2Cx->CR1 |= 1 << 10;

}
// @I2Cx: i2c device
// @addr: i2c address
// @buf:  stor read bytes
// @len:  if len=0, the first received byte as i2c packet lenght
int i2c_dev_read(I2C_TypeDef* I2Cx, unsigned char addr, unsigned char *buf, unsigned char len)
{
	 int timeout = 0xa000;
	 char fstaslen = 0;
	 fstaslen = len;
	 
	 while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))
	 {
	
		 // if((timeout--) <= 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
		 if((timeout--) <= 0) {I2C_recfg (I2Cx);return -2;}
	 }
	  I2Cx->CR1 |= 0x0100;
	 timeout = 0x1000;
				 
	
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))//check EV5
	 {
		 if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
	 }
	 
	 I2C_Send7bitAddress(I2Cx, addr, I2C_Direction_Receiver);
	
	 timeout = 0x1000;
	 while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))//EV6
	 {
		 if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
	 }

	 
	 do
	 {
    	 timeout = 0x1000;
    	 while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))
    	 {
    		 if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    	 }
    	 *buf = I2Cx->DR & 0xff;
         if(fstaslen == 0)
         {
             fstaslen = 1;//the first byte as len
             len = *buf + 1;// need to receive len byte(s) more
             if(len == 0)
                 return -1;
         }
    	 buf++;
	 }while(--len);
	 I2C_GenerateSTOP(I2Cx, ENABLE); 
	 return 0;
}


int i2c_dev_write(I2C_TypeDef* I2Cx, unsigned char addr, unsigned char *buf, unsigned char len)
{
    int timeout = 0xa000;

    while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))
    {

        // if((timeout--) <= 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
        if((timeout--) <= 0) {I2C_recfg (I2Cx);return -2;}
    }
    I2Cx->CR1 |= 0x0100;
    timeout = 0x1000;
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))//check EV5
    {
	    if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }
    I2C_Send7bitAddress(I2Cx, addr, I2C_Direction_Transmitter);

    timeout = 0x2000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))//EV6
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -2;}
    }
    while(len--)
    {
        I2Cx->DR = *buf++;
        timeout = 0x1000;
        while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))//EV8
        {
          if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
        }
    }
   
   I2C_GenerateSTOP(I2Cx, ENABLE);	

    return OK;
}

//MAX6581 function

//read byte debuged
char max6581ReadByte(I2C_TypeDef* I2Cx, unsigned char command, void *date)
{
    int timeout = 0xa000;
    while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))
    {

        // if((timeout--) <= 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
        if((timeout--) <= 0) {I2C_recfg (I2Cx);return -2;}
    }
    //I2C_AcknowledgeConfig(I2Cx, DISABLE);
    //I2C_GenerateSTART(I2Cx, ENABLE);//start condition
     I2Cx->CR1 |= 0x0100;
    timeout = 0x1000;
				

   while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))//check EV5
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }
    
    I2C_Send7bitAddress(I2Cx, AMF_IPMC_I2CADDR_MAX6581, I2C_Direction_Transmitter);
    
    timeout = 0x2f000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))//EV6
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }


    //I2C_SendData(I2Cx,  command && 0xFF); 
    I2Cx->DR = command;
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))//EV8
    {
      if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }

    //restart
    //I2C_GenerateSTART(I2Cx, ENABLE);
    I2Cx->CR1 |= 0x0100;//发送第二个起始信号
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))//check EV5
    {
        if((timeout--) == 0){I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }
   
    I2C_Send7bitAddress(I2Cx, AMF_IPMC_I2CADDR_MAX6581, I2C_Direction_Receiver);

    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))//EV6
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }

    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }
    //*date = I2C_ReceiveData(I2Cx);
    *(int*)date = I2Cx->DR & 0xffff;

    I2C_GenerateSTOP(I2Cx, ENABLE);	
    return 0;

}

//write byte
char max6581WriteByte(I2C_TypeDef* I2Cx, unsigned char command, unsigned char date)
{
    int timeout = 0xa000;
    while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))
    {
        // if((timeout--) <= 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
        if((timeout--) <= 0) {I2C_recfg (I2Cx);return -2;}
    }
    //I2C_AcknowledgeConfig(I2Cx, DISABLE);
    //I2C_GenerateSTART(I2Cx, ENABLE);//start condition
    I2Cx->CR1 |= 0x0100;
    timeout = 0x1000;
				

   while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))//check EV5
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }
    
    I2C_Send7bitAddress(I2Cx, AMF_IPMC_I2CADDR_MAX6581, I2C_Direction_Transmitter);
    
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))//EV6
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }
    

    //I2C_SendData(I2Cx,  command && 0xFF); 
    I2Cx->DR = command;
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))//EV8
    {
      if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }
   
    //*date = I2C_ReceiveData(I2Cx);
    I2Cx->DR = date;
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))//EV8
    {
      if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }
   
    I2C_GenerateSTOP(I2Cx, ENABLE);	

    return 0;
}

//send byte
char max6581Sendbyte(I2C_TypeDef* I2Cx, unsigned char command, unsigned char *date)
{
    int timeout = 0xa000;
    while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))
    {
        // if((timeout--) <= 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
        if((timeout--) <= 0) {I2C_recfg (I2Cx);return -2;}
    }
    //I2C_AcknowledgeConfig(I2Cx, DISABLE);
    //I2C_GenerateSTART(I2Cx, ENABLE);//start condition
    I2Cx->CR1 |= 0x0100;
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))//check EV5
     {
         if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
     }

    I2C_Send7bitAddress(I2Cx, AMF_IPMC_I2CADDR_MAX6581, I2C_Direction_Transmitter);
    
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))//EV6
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }

    I2Cx->DR = command;
        timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))//EV8
    {
      if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }

    I2C_GenerateSTOP(I2Cx, ENABLE);	

    return 0;
}

//receive byte
char max6581Recvbyte(I2C_TypeDef* I2Cx, unsigned char command, unsigned char *date)
{
    int timeout = 0xa000;
    while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))
    {
        // if((timeout--) <= 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
        if((timeout--) <= 0) {I2C_recfg (I2Cx);return -2;}
    }
    I2C_AcknowledgeConfig(I2Cx, ENABLE);
    //I2C_GenerateSTART(I2Cx, ENABLE);//start condition
    I2Cx->CR1 |= 0x0100;
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))//check EV5
     {
         if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
     }

    I2C_Send7bitAddress(I2Cx, AMF_IPMC_I2CADDR_MAX6581, I2C_Direction_Receiver);
    
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))//EV6
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }

    *date = I2Cx->DR;
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))//EV8
    {
         if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }

    I2C_GenerateSTOP(I2Cx, ENABLE);	

    return 0;
}


char max6581Init()
{
    //do nothing , use default setting.
    return 0;
}
char max6581ReadManID(I2C_TypeDef* I2Cx)
{
    unsigned char ManuID = 0;
    max6581ReadByte(I2Cx, 0x0a, &ManuID);
    return ManuID;
}
//6581温度补偿表:
//测量范围 0-127摄氏度
// 1,温度补偿


// 2,工作电压补偿


// 3,噪声补偿


//sys_data.sys_temp数组中记录从remote register读取的原始值
//
//
char max6581ReadTemp(void)
{
    char i,j;
    char validcnt;
    int temperature = 0;
    // max6581ReadManID(I2C3);
    for(i = 0; i < 8; i++)//8组温度，每组取5次，记录平均值
    {
        validcnt = 0;
        temperature = 0;
        for(j=0;j<1;j++)
        {
			if(max6581ReadByte(I2C3, i+1, &sys_data.sys_temp[i]) != 0 )
			{
				sys_data.sys_temp[i] = 0xFFFFFFFF;//如果读取i2C总线失败，则记录此处电压值为出错0xff
			}
			else
			{
				if(sys_data.sys_temp[i] > 150)
				    continue;
			    validcnt++;
			    temperature += sys_data.sys_temp[i];
			}
        }
        //calculate average
        validcnt = validcnt>0?validcnt:5;
        sys_data.sys_temp[i] = temperature/validcnt;
    }
    // max6581ReadManID(I2C1);
    for(i = 8; i < 16; i++)
    {
        validcnt = 0;
        temperature = 0;
        for(j=0;j<1;j++)
        {
			if(max6581ReadByte(I2C1, i-7, &sys_data.sys_temp[i]) != 0)
			{
				sys_data.sys_temp[i] = 0xFFFFFFFF;//如果读取i2C总线失败，则记录此处电压值为出错0xff
			}
			else
			{
				if(sys_data.sys_temp[i] > 150)//超过150度认为异常
				    continue;
			    validcnt++;
			    temperature += sys_data.sys_temp[i];
			}
        }
        //calculate average
        validcnt = validcnt>0?validcnt:5;
        sys_data.sys_temp[i] = temperature/validcnt;
    }
    return 0;
}

char tmpCmpLimit(void)
{
    char i;
    unsigned short flag = 0;
    local_data_pool.sensorAlarmTab.temp = 0;
    for(i=0;i<16;i++)
    { 
        if(local_data_pool.sensorStaTab.temp[i] > fanLevelCtrlTb[2].temperature)
        {
            uartCmdAnalize(1);
            vTaskDelay(1000);
            local_data_pool.fruActEn =0;
        }
        else
        {
            if(local_data_pool.sensorLimitTab.temp[i] > local_data_pool.sensorStaTab.temp[i] )
                flag |= (1<<i);
        }
        
        if(local_data_pool.sensorStaTab.temp[i] > fanLevelCtrlTb[0].temperature)
            local_data_pool.sensorAlarmTab.temp |= (1<<i);
    }
    if(flag == 0xFFFF)
        local_data_pool.fruActEn = 1;
    return 0;
}

// INA219 function

//ina219_read_word

unsigned  ina219ReadWord(unsigned char point, unsigned short *date)
{
    unsigned short readvalh,readvall;
    int timeout = 0xa000;
    I2C_AcknowledgeConfig(I2C2, ENABLE);
    while(I2C_GetFlagStatus(I2C2, I2C_FLAG_BUSY))
    {
        if((timeout--) <= 0) {I2C_GenerateSTOP(I2C2, ENABLE);return -1;}
    }
    I2C2->CR1 |= 0x0100;
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT))//check EV5
     {
         if((timeout--) == 0) {I2C_GenerateSTOP(I2C2, ENABLE);return -1;}
     }

    I2C_Send7bitAddress(I2C2, AMF_IPMC_I2CADDR_INA219, I2C_Direction_Transmitter);
   timeout = 0x1000;
    while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))//EV6
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2C2, ENABLE);return -1;}
    }


    
    I2C2->DR = point;
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
      if((timeout--) == 0) {I2C_GenerateSTOP(I2C2, ENABLE);return -1;}
    }



    I2C2->CR1 |= 0x0100;
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT))//check EV5
     {
         if((timeout--) == 0) {I2C_GenerateSTOP(I2C2, ENABLE);return -1;}
     }
    I2C_Send7bitAddress(I2C2, AMF_IPMC_I2CADDR_INA219, I2C_Direction_Receiver);

    timeout = 0x1000;
    while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))//EV6
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2C2, ENABLE);return -1;}
    }

	while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED))
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2C2, ENABLE);return -1;}
    }

    readvalh = I2C2->DR;

    I2C_AcknowledgeConfig(I2C2, DISABLE);//ACK 响应在接收中需打开，在接收最后一个字节后关闭
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED))
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2C2, ENABLE);return -1;}
    }
    readvall = I2C2->DR;
    I2C_GenerateSTOP(I2C2, ENABLE);	
		*date = readvalh<<8 | readvall;
		
    return 0;
}


char ina219WriteWord(unsigned char frame, unsigned short date)
{
    int timeout = 0xa000;
    I2C_AcknowledgeConfig(I2C2, ENABLE);
    while(I2C_GetFlagStatus(I2C2, I2C_FLAG_BUSY))
    {
        if((timeout--) <= 0) {I2C_GenerateSTOP(I2C2, ENABLE);return -1;}
    }
    I2C2->CR1 |= 0x0100;
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT))//check EV5
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2C2, ENABLE);return -1;}
    }
    I2C_Send7bitAddress(I2C2, AMF_IPMC_I2CADDR_INA219, I2C_Direction_Transmitter);
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))//EV6
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2C2, ENABLE);return -1;}
    }

//frame 2 Register Pointer Byte
    I2C2->DR = frame;
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
      if((timeout--) == 0) {I2C_GenerateSTOP(I2C2, ENABLE);return -1;}
    }
    //frame 3 & frame 4: date 
    I2C2->DR = date >> 8 & 0xFF;//transfer MSB 8 bits
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
      if((timeout--) == 0) {I2C_GenerateSTOP(I2C2, ENABLE);return -1;}
    }
    I2C2->DR = date & 0xFF;//transfer LSB 8 bits
    while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
      if((timeout--) == 0) {I2C_GenerateSTOP(I2C2, ENABLE);return -1;}
    }
    
    I2C_GenerateSTOP(I2C2, ENABLE);	

    return 0;
}
char ina219RefreshPointer(unsigned char frame)
{
    int timeout = 0xa000;
    I2C_AcknowledgeConfig(I2C2, ENABLE);
    while(I2C_GetFlagStatus(I2C2, I2C_FLAG_BUSY))
    {
        if((timeout--) <= 0) {I2C_GenerateSTOP(I2C2, ENABLE);return -1;}
    }
    I2C2->CR1 |= 0x0100;
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT))//check EV5
     {
         if((timeout--) == 0) {I2C_GenerateSTOP(I2C2, ENABLE);return -1;}
     }
     I2C_Send7bitAddress(I2C2, AMF_IPMC_I2CADDR_INA219, I2C_Direction_Transmitter);
         timeout = 0x1000;
    while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))//EV6
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2C2, ENABLE);return -1;}
    }

//frame 2 Register Pointer Byte
    I2C2->DR = frame;
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
      if((timeout--) == 0) {I2C_GenerateSTOP(I2C2, ENABLE);return -1;}
    }
    I2C_GenerateSTOP(I2C2, ENABLE);	
    
    return 0;
}
//for ina219 init routine, mainly calculate Register CALIBRATION.
//cal = 32768
//How to:
//Vbus_max = 16
//Vshunt_max = 0.04
//Rshunt = 0.5
//MaxPossible_I  = Vshunt_max / Rshunt = 0.08
//Max_Expcted_I = 0.08
//Minimum_LSB = Max_Expected_I / 32768 = 2.44 * 10^ -6
//Maximum_LSB = Max_Expected_I / 4096 =  19.5 * 10^ -6
//Current_LSB = 4.096 * 10^ -6
//cal = 0.04096/(Current_LSB*Rshunt) = 16786 ~ 2100


char ina219Init(void)
{
    unsigned short val;
    val = 0;//configuration register pg1=0,pg0=0, other bit keep default
	ina219WriteWord(INA219_POINTER_ADDR_CONFIG_REGISTER, 0x8000);//write config register;
    ina219WriteWord(INA219_POINTER_ADDR_CONFIG_REGISTER, 0x219F);//write config register;
	ina219ReadWord(INA219_POINTER_ADDR_CONFIG_REGISTER, &val); 
    ina219WriteWord(INA219_POINTER_ADDR_CALIBRATION, 1250);//calibration 

    return 0;
}
unsigned short ina219ReadPower()
{
    unsigned short val;
   // ina219_refresh_pointer(INA219_POINTER_ADDR_BUS_VOLTAGE);
    //I2C_delay(4);//delay some thing
    ina219ReadWord(INA219_POINTER_ADDR_POWER,&val);
	  *sys_data.power = val*100;
    return val;
}

unsigned short ina219ReadCurrent()
{
    unsigned short val;
    //ina219_refresh_pointer(INA219_POINTER_ADDR_CURRENT);
    I2C_delay(4);//delay some thing
    ina219ReadWord(INA219_POINTER_ADDR_CURRENT, &val);
    return val;
}

unsigned short ina219ReadBusVtg()
{
    unsigned short val;
   // ina219_refresh_pointer(INA219_POINTER_ADDR_CURRENT);
    I2C_delay(4);//delay some thing
    ina219ReadWord(INA219_POINTER_ADDR_BUS_VOLTAGE, &val);
    return val;
}
unsigned short ina219ReadShuntVtg()
{
    unsigned short val;
   // ina219_refresh_pointer(INA219_POINTER_ADDR_CURRENT);
    I2C_delay(4);//delay some thing
    ina219ReadWord(INA219_POINTER_ADDR_SHUNT_VOLTAGE, &val);
    return val;
}

int isl68201ReadByte(I2C_TypeDef* I2Cx, u8 i2caddr, unsigned char command, u8 *data)
{
    int timeout = 0xa000;
    while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))
    {

        // if((timeout--) <= 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
        if((timeout--) <= 0) {I2C_recfg (I2Cx);return -2;}
    }
    //I2C_AcknowledgeConfig(I2Cx, DISABLE);
    //I2C_GenerateSTART(I2Cx, ENABLE);//start condition
     I2Cx->CR1 |= 0x0100;
    timeout = 0x1000;
                

   while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))//check EV5
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }
    
    I2C_Send7bitAddress(I2Cx, i2caddr, I2C_Direction_Transmitter);
    
    timeout = 0x2f000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))//EV6
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }


    //I2C_SendData(I2Cx,  command && 0xFF); 
    I2Cx->DR = command;
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))//EV8
    {
      if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }

    //restart
    //I2C_GenerateSTART(I2Cx, ENABLE);
    I2Cx->CR1 |= 0x0100;//发送第二个起始信号
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))//check EV5
    {
        if((timeout--) == 0){I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }
   
    I2C_Send7bitAddress(I2Cx, i2caddr, I2C_Direction_Receiver);

    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))//EV6
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }

    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }
    //*date = I2C_ReceiveData(I2Cx);
    *(int*)data = I2Cx->DR & 0xffff;

    I2C_GenerateSTOP(I2Cx, ENABLE); 
    return 0;

}

int isl68201SendCmd(I2C_TypeDef* I2Cx, u8 i2caddr, unsigned char command)
{
    int timeout = 0xa000;
    while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))
    {

        // if((timeout--) <= 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -2;}
        if((timeout--) <= 0) {I2C_recfg (I2Cx);return -2;}
    }
    //I2C_AcknowledgeConfig(I2Cx, DISABLE);
    //I2C_GenerateSTART(I2Cx, ENABLE);//start condition
     I2Cx->CR1 |= 0x0100;
     timeout = 0x1000;
                

    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))//check EV5
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }
    
    I2C_Send7bitAddress(I2Cx, i2caddr, I2C_Direction_Transmitter);
    
    timeout = 0x2f000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))//EV6
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }


    //I2C_SendData(I2Cx,  command && 0xFF); 
    I2Cx->DR = command;
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))//EV8
    {
      if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }

    I2C_GenerateSTOP(I2Cx, ENABLE); 
    return 0;

}


int isl68201SendByte(I2C_TypeDef* I2Cx, u8 i2caddr, unsigned char command, u8 data)
{
    int timeout = 0xa000;
    while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))
    {

        // if((timeout--) <= 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
        if((timeout--) <= 0) {I2C_recfg (I2Cx);return -2;}
    }
    //I2C_AcknowledgeConfig(I2Cx, DISABLE);
    //I2C_GenerateSTART(I2Cx, ENABLE);//start condition
     I2Cx->CR1 |= 0x0100;
     timeout = 0x1000;
                

    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))//check EV5
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }
    
    I2C_Send7bitAddress(I2Cx, i2caddr, I2C_Direction_Transmitter);
    
    timeout = 0x2f000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))//EV6
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }


    //I2C_SendData(I2Cx,  command && 0xFF); 
    I2Cx->DR = command;
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))//EV8
    {
      if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }
    
    I2Cx->DR = data;
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))//EV8
    {
      if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }


    I2C_GenerateSTOP(I2Cx, ENABLE); 
    return 0;

}

int isl68201ReadWord(I2C_TypeDef* I2Cx, u8 i2caddr, unsigned char command, u16 *data)
{
    unsigned short readvalh,readvall;
    int timeout = 0xa000;
    I2C_AcknowledgeConfig(I2Cx, ENABLE);
    while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))
    {
        // if((timeout--) <= 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
        if((timeout--) <= 0) {I2C_recfg (I2Cx);return -2;}
    }
    I2Cx->CR1 |= 0x0100;
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))//check EV5
     {
         if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
     }

    I2C_Send7bitAddress(I2Cx, i2caddr, I2C_Direction_Transmitter);
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))//EV6
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }


    
    I2Cx->DR = command;
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
      if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }



    I2Cx->CR1 |= 0x0100;
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))//check EV5
     {
         if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
     }
    I2C_Send7bitAddress(I2Cx, i2caddr, I2C_Direction_Receiver);

    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))//EV6
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }

    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }

    readvall = I2Cx->DR;

    I2C_AcknowledgeConfig(I2Cx, DISABLE);//ACK 响应在接收中需打开，在接收最后一个字节后关闭
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }
    readvalh = I2Cx->DR;
    I2C_GenerateSTOP(I2Cx, ENABLE); 
        *data = readvalh << 8 | readvall;
        
    return 0;
}



int isl68201ReadBlock(I2C_TypeDef* I2Cx, u8 i2caddr, unsigned char command, u8 *data)
{
    unsigned short readvalh,readvall, len = 0 , i = 0;
    int timeout = 0xa000;
    while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))
    {
        // if((timeout--) <= 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
        if((timeout--) <= 0) {I2C_recfg (I2Cx);return -2;}
    }
    I2C_AcknowledgeConfig(I2Cx, ENABLE);
    I2Cx->CR1 |= 0x0100;
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))//check EV5
     {
         if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
     }

    I2C_Send7bitAddress(I2Cx, i2caddr, I2C_Direction_Transmitter);
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))//EV6
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }


    
    I2Cx->DR = command;
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
      if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }

    I2Cx->CR1 |= 0x0100;
    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))//check EV5
     {
         if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
     }
    I2C_Send7bitAddress(I2Cx, i2caddr, I2C_Direction_Receiver);

    timeout = 0x1000;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))//EV6
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }

    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))
    {
        if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
    }
//get block read len
    len = I2Cx->DR;


    while(len--)
    {
        if(len == 0)
        I2C_AcknowledgeConfig(I2Cx, DISABLE);//ACK 响应在接收中需打开，在接收最后一个字节后关闭
        timeout = 0x1000;
        while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))
        {
            if((timeout--) == 0) {I2C_GenerateSTOP(I2Cx, ENABLE);return -1;}
        }
        data[i] = I2Cx->DR;
        i++;
    }
    I2C_GenerateSTOP(I2Cx, ENABLE); 
    return 0;
}


int isl8021ClearByte()
{
    
}
int isl68201ReadDeviceId(u8 devid)
{
    u16 deviceId = 0; 
    u8 Rev = 0;
    int ret;
    // isl68201SendCmd(I2C3, 0xC0, 0x03);
    ret = isl68201ReadBlock(ISL68021_DEV[devid].I2C, ISL68021_DEV[devid].i2caddr, 0xAD, (u8*)&deviceId);
    ret = isl68201ReadBlock(ISL68021_DEV[devid].I2C, ISL68021_DEV[devid].i2caddr, 0xAE, (u8*)&deviceId);
    ret = isl68201ReadByte( ISL68021_DEV[devid].I2C, ISL68021_DEV[devid].i2caddr, 0x98, &Rev);

}


int isl68201ReadPower(u8 devid)
{
    u16 current, votage;
    float tmp;
    // isl68201SendCmd(I2C3, 0xC0, 0x03);
    isl68201ReadWord(ISL68021_DEV[devid].I2C, ISL68021_DEV[devid].i2caddr, 0x8b, &votage);
    tmp = votage/128.0;
    isl68201ReadWord(ISL68021_DEV[devid].I2C, ISL68021_DEV[devid].i2caddr, 0x8c, &current);
    tmp *= (current - 0xE800)*0.125;
    return (int)tmp;
    // printf("isl68201ReadPower[%d]power %f w\r\n", devid, tmp);
}


int isl68201Readtemp()
{
    
    u16 temperature;
    isl68201ReadWord(I2C3, 0xC0, 0x8d, &temperature);
    printf("isl68201Readtemp: \n");
}

int isl68201ReadFpgaPower()
{
    sys_data.fpgapower[0] = isl68201ReadPower(0);
    sys_data.fpgapower[1] = isl68201ReadPower(1);
    sys_data.fpgapower[2] = isl68201ReadPower(2);
    sys_data.fpgapower[3] = isl68201ReadPower(3);
    sys_data.fpgapower[4] = isl68201ReadPower(4);
    sys_data.fpgapower[5] = isl68201ReadPower(5);
    return OK;
}
//***************** IPMB communicationg ******************//
//IPMB-0 use i2c interrupt 



//IPMB-1 use GPIO 


/*EOF*/

