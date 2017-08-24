
//This is comp_uart.c
//This file mainly offer component of the uart.


#include <stdio.h>
#include "stm32f2xx.h"
#include "system.h"
#include "comp_fish.h"
#include "comp_uart.h"
#include "IPMC.h"
#include "ShMC.h"
#include "errcode.h"
#include "version.h"


#define IPMC_HEAD0_MG0   0
#define IPMC_HEAD1_MG1   1
#define IPMC_HEAD2_SRCSLOT   2
#define IPMC_HEAD3_DSTSLOT   3
#define IPMC_HEAD4_VLD_DATA_LEN   4





unsigned char pendsv_flag;
USART_RX_BUFFER_RING RxBufRing;
__IO char uart_cache[100];//存放uart收到的数据
__IO char uart_buffer[32];//在命令解析的过程中，存放一个命令

DEV_UART devuart4;
//DEV_UART uart2;


unsigned char cmd_counter = 0;//uart_buffer中存放命令的字符数
char *p_uart_produce = NULL;
char *p_uart_custom = NULL;
static unsigned long uart_recv_cycle_last = 0;
FISH_CMD_t fish_command_list[FISH_MAX_CMD_SUPPORT];

unsigned char SizeOfRxBody = 0;		   
unsigned char  TxCounter=0;					  //发送计数器
//unsigned char  TxBuffer[64];
__IO unsigned char  RxCounter=0; 				  //接收计数器
unsigned char  UsartRxFullFlag = 0;

extern int ipmcGetMySlot(void);
__asm trig_pendSV_fish();
UART_MSG_QUEUE_t msgQueue;

/*CRC16-CCITT---x16 + x12 + x5 + 1 */
const unsigned short crc_Table[16]={ 
0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
};

char flush_IO()
{
    //flush IO
    DEV_UART *dev = &devuart4;
    if(dev->txcnt == 0)
        return 0;
    dev->pTxConsumer = dev->pTxProductor;
    DMA_MemoryTargetConfig(DMA1_Stream4, (u32)dev->pTxConsumer, DMA_Memory_0);
    DMA_SetCurrDataCounter(DMA1_Stream4, dev->txcnt);
    dev->txcnt = 0;
    DMA_Cmd(DMA1_Stream4, ENABLE);/* Tx */
    dev->pTxProductor = (dev->pTxProductor==dev->uartTxBuf[0])?dev->uartTxBuf[1] : dev->uartTxBuf[0];
    return 0;
}
char push_char(DEV_UART *dev, char c)
{
    *(dev->pTxProductor+dev->txcnt) = c;
    dev->txcnt++;
    if(dev->txcnt >= PRINTF_IO_FLUSH_SIZE)//raise a DMA transfer
    {
        flush_IO();
    }
    return 0;
}
char push_string(DEV_UART *dev, char *s)
{
    int i = 0, len = strlen(s);
    for(i = 0; i < len; i++)
        push_char(dev, *(s+i));
    return len;
}
//==================uart2 basic stuff=======================

char send_byte(USART_TypeDef * UARTx, unsigned char ch)
{
    while (!(UARTx->SR & 0x0080))
        ;
    UARTx->DR = (ch & 0xFF);
    return 0;
}

char send_buf(USART_TypeDef * UARTx, unsigned char *buf, int len)
{
	    int i = 0;
	    if(len <=0 )
			return -1;
	    for(i = 0; i < len; i++)
    	    send_byte(UARTx,*(buf+i));
			return i;
}
char recv_buf(USART_TypeDef * UARTx, unsigned char *buf, int len)
{
	    int i = 0;
	    if(len <=0 )
			return -1;
	    for(i = 0; i < len; i++)
    	    get_byte(UARTx,(buf+i));
		return i;
}

char put(unsigned char ch)
{
    push_char(&devuart4, ch);
    return 0;
}
char get_byte(USART_TypeDef * UARTx, unsigned char *ch)
{
    while(USART_GetITStatus(UART4, USART_IT_RXNE) == RESET)
        ;
    *ch = UARTx->DR;
    return 0;
}


struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;


/*int fputc(int c, FILE *f) {
  return (send_byte(UART4,c));
}*/

int fputc(int c, FILE *f) {
  return (push_char(&devuart4, c));
}

char putstr(char *str)
{  
    int i;
    for(i = 0; i < strlen(str); i++)
    push_char(&devuart4, *(str+i) );
    return 0;
}

//===============uart2 advanced stuff,protocal================

/******************************************************
*Crc16----CRC16-CCITT校验算法
*
*@unsigned char *ptr---计算数据首指针
*@unsigned short len---计算数据长度
*
*return: crc16 value
*/
unsigned short uartCrc16(unsigned char *ptr, unsigned short len) 
{
	unsigned short crc=0;
	unsigned short da;
	while(len--)
	{
	   da=crc>>12; 
	   crc<<=4; 
	   crc^=crc_Table[da^(*ptr>>4)];                               
	   da=crc>>12; 
	   crc<<=4; 
	   crc^=crc_Table[da^(*ptr&0x0f)]; 
	   ptr++;
	}
	return(crc);
}

/******************************************************
*
* @pkt : address of packet being checked
* @pktlen : packet length , but doesn't contain the last two crc16 check bytes.
* return OK , ERROR
*/

char uartPktVld(unsigned char *pkt, unsigned char pktlen)
{
    unsigned short *crc;
    unsigned short expCrc;
    if(pkt == NULL)
        return ERR_NULL;
    crc = (unsigned short *)(pkt + pktlen);
    //calculate except crcval
    expCrc = uartCrc16(pkt, pktlen);
    if(expCrc != *crc)
        return ERROR;
    return OK;
}


void usartBufferInit(void)
{
	//RxBufRing.pBufBegin = RxBufRing.usartBuf[0];
	//RxBufRing.pBufEnd = &RxBufRing.usartBuf[USART_RX_BUFFR_CNT-1][USART_MSG_LEN_MAX-1];
	RxBufRing.pBufPut =  RxBufRing.usartBuf[0];
	RxBufRing.pBufGet = RxBufRing.pBufPut;
	memset(RxBufRing.usartBuf[0], 0, USART_RX_BUFFR_CNT*USART_RXMSG_LEN_MAX);
	msgQueue.buf = NULL;
	msgQueue.len = 0;
	memset(&devuart4, 0, sizeof(DEV_UART));
	devuart4.pTxProductor = devuart4.uartTxBuf[0];
	devuart4.pTxConsumer = devuart4.uartTxBuf[0];
}

//push msg to uart2 queue
char pushMsgToQueue(unsigned char *buf, int len)
{
    //enter critical
   // msgQueue.len += len;
   // while(msgQueue.buf)
    //exit critical
    return 0;
}


//echo

//uart4 handler
//the handler, receive date from "terminal" only.
void UART4_IRQHandler()
{
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)	
    {
        USART_ClearITPendingBit(UART4, USART_IT_RXNE);
        *p_uart_produce = USART_ReceiveData(UART4);
        if(p_uart_produce == &uart_cache[99])
            p_uart_produce = uart_cache;
        else
            p_uart_produce ++;
    }
    
}


//@ alarm: 0-analize, 1-alarm message send
//
int uartCmdAnalize(unsigned isalarm)
{
	//cmd_analize mainly analize array RxBufRing.pBufGet
    unsigned char *pget = RxBufRing.pBufGet;
    __IO unsigned char buf[256];
    
    unsigned int *data = (unsigned int *)(buf+SIZEOF_SERIAL_PKT);
    int len;
    unsigned char area, type, id, srcslot, dstslot, setoper;
    unsigned short fsn;
    unsigned short *crc16 = NULL;
    unsigned char isheartbeat;
    SERIAL_HEAD *phead = (SERIAL_HEAD *)pget;
    SERIAL_BODY *pbody = (SERIAL_BODY *)(pget +SIZEOF_SERIAL_HEAD);
    SERIAL_HEAD *rplhead = (SERIAL_HEAD *)buf;
    SERIAL_BODY  *rplbody = (SERIAL_BODY *)(buf +SIZEOF_SERIAL_HEAD);
    
    memset(buf, 0, 256);
    
     if(isalarm)
     {
         
         rplhead->magic1 = 0xaa;
         rplhead->magic2 = 0x55;
         rplhead->srcSlot = 1;//reserved
         rplhead->dstSlot = ipmcGetMySlot();
         rplhead->fsn       = fsn;
         rplhead->bodyLen = 0;
         
         rplbody->s.isheartbeat = 0;
         rplbody->s.isset           = 0;
         rplbody->s.gcb            = 0;
         rplbody->s.type           = IPMC_SENSOR_TYPE_TEMPERATURE;
         rplbody->s.area           = IPMC_ALARM_DATA_AREA;
         rplbody->s.id               =0xF;
         *data = local_data_pool.sensorAlarmTab.temp;
         crc16 = (unsigned short *)(buf+len-2);
         *crc16 = uartCrc16(buf, len-2);
         if(GX36_heart_beat())
             send_buf(USART2, buf, 0);//send_buf(USART2, buf, len+2);
         return 0;

     }
     area            = pbody->s.area;
     type           = pbody->s.type;
     id               = pbody->s.id;
     isheartbeat = pbody->s.isheartbeat;
     setoper       = pbody->s.isset;
     srcslot        = phead->srcSlot;
     dstslot        = phead->dstSlot;
     fsn             = phead->fsn;
     if(uartPktVld(pget, phead->bodyLen + sizeof(SERIAL_HEAD)) != OK)
		 return ERR_PKT;

    if(srcslot != ipmcGetMySlot())
        return ERR_NODEV;
    if(isheartbeat)//heartbeat 包
    {
        local_data_pool.gx36HeartBeat ++;
        //local_data_pool.sensorStaTab.cpuHealth = 1;
        return OK;
    }
    if(setoper)
	{
		//====set state====//
		if(dstslot!= ipmcGetShmcSlot())
		{
		    return ERR_NO_ACCESS;
		}
		if(area == IPMC_LIMIT_DATA_AREA)//set limit
		{
		    switch(type)
		    {
		        case IPMC_SENSOR_TYPE_TEMPERATURE:
		        if(id < NUM_OF_TEMP_SENSOR)
		        {
    		            local_data_pool.sensorLimitTab.temp[id] = *(int *)(pget+SIZEOF_SERIAL_PKT);
    		        }
    		        if(id == 0xff)
                        {
                            memcpy(local_data_pool.sensorLimitTab.temp, pget+SIZEOF_SERIAL_PKT, sizeof(local_data_pool.sensorLimitTab.temp));
                        }
		        break;

		        case IPMC_SENSOR_TYPE_VTG_CUR:
		        if(id < NUM_OF_VTG_SENSOR)
		        {
		            //*((int *)&local_data_pool.sensorLimitTab.vtg.fpga_core_vtg[0] + id) = *((int *)(pget+SIZEOF_SERIAL_PKT));
                        }
		        if(id == 0xff)
		        {
		            //memcpy(local_data_pool.sensorLimitTab.vtg.fpga_core_vtg, pget+SIZEOF_SERIAL_PKT, sizeof(local_data_pool.sensorLimitTab.vtg));
		        }
		        break;
		        
		        default:break;
		    }
		}
		if(area == IPMC_SHMC)
		{
		    switch(type)
		    {
		        case SHMC_CASE_FAN:
		            shmcCaseSet(SET_FAN_LEVEL, (int *)(pget+SIZEOF_SERIAL_PKT));
		            //ICMMFanLevelSet(*(int *)(pget+SIZEOF_SERIAL_PKT));    
		        break;
		        default:break;
		    }
		}
	}
	else//get stage
	{
	
        rplhead->magic1 = 0xaa;
        rplhead->magic2 = 0x55;
        rplhead->srcSlot = dstslot;//reserved
        rplhead->dstSlot = srcslot;
        rplhead->fsn       = fsn;
        rplhead->bodyLen = 0;
        
        rplbody->s.isheartbeat = 0;
        rplbody->s.isset           = 0;
        rplbody->s.gcb            = 0;
        rplbody->s.type           = type;
        rplbody->s.area           = area;
        rplbody->s.id               =id;
        
        if(area == IPMC_STATUS_DATA_AREA)//sensor state
        {
            switch(type)
            {
                case IPMC_SENSOR_TYPE_TEMPERATURE://temperature
                if(id < NUM_OF_TEMP_SENSOR)//look up single state
                {
                    //*data = sys_data.sys_temp[id];
                    memcpy(data, sys_data.sys_temp+id, sizeof(int));
                    len = SIZEOF_SERIAL_PKT + sizeof(int) + 2;//8+1+2
                }
                else if(id < 0xFF)
                {
                    *data = 0xFF;
                    len = SIZEOF_SERIAL_PKT + sizeof(int) + 2;
                }
                else//id == 0xff
                {
                    memcpy(data, local_data_pool.sensorStaTab.temp, sizeof(local_data_pool.sensorStaTab.temp));
                    len = SIZEOF_SERIAL_PKT+sizeof(local_data_pool.sensorStaTab.temp)+2;
                }
                break;
                
                case IPMC_SENSOR_TYPE_VTG_CUR://voltage
                    if(id < NUM_OF_VTG_SENSOR)
                    {
                        //*data = *(sys_data.sys_vtg_cur->fpga_core_vtg+id);
                        len = SIZEOF_SERIAL_PKT+sizeof(int)+2;
                    }
                    else if(id<0xFF)
                    {
                        *data = 0;
                        len = SIZEOF_SERIAL_PKT+sizeof(int)+2;
                    }
                    else
                    {
                        //memcpy(data, local_data_pool.sensorStaTab.vtg.fpga_core_vtg, sizeof(local_data_pool.sensorStaTab.vtg));
                        len = SIZEOF_SERIAL_PKT + sizeof(local_data_pool.sensorStaTab.vtg);
                    }
                break;
                
                case IPMC_SENSOR_TYPE_FRU_STAT://fru state
                    *data  = local_data_pool.sensorStaTab.fruSta;
                    len = SIZEOF_SERIAL_PKT+sizeof(int)+2;
                break;
                case IPMC_SENSOR_TYPE_POWER://board power
                    *data = *sys_data.power;
                    len = SIZEOF_SERIAL_PKT+sizeof(int)+2;
                break;
                case IPMC_SENSOR_TYPE_CPU_HEALTH://CPU health
                    *data  = local_data_pool.sensorStaTab.cpuHealth; 
                    len = SIZEOF_SERIAL_PKT+sizeof(int)+2;
                break;
                case IPMC_SENSOR_TYPE_HOTSWAP://hotswap
                    *data  = local_data_pool.sensorStaTab.hotSwap;
                    len = SIZEOF_SERIAL_PKT+sizeof(int)+2;
                    break;
                case IPMC_SENSOR_TYPE_IPMC_VER:
                    *data = IPMC_VER_INT;
                    len = SIZEOF_SERIAL_PKT+sizeof(int)+2;
                break;
                default:break;
            }
        }
        else if(area == IPMC_ALARM_DATA_AREA)//alarm
        {
            switch(type)
            {
                case IPMC_SENSOR_TYPE_TEMPERATURE://temperature
                    *data = local_data_pool.sensorAlarmTab.temp ;
                    len = SIZEOF_SERIAL_PKT+sizeof(int)+2;
                break;
                case IPMC_SENSOR_TYPE_VTG_CUR://voltage
                    *data = local_data_pool.sensorAlarmTab.vtg ;
                    len = SIZEOF_SERIAL_PKT+sizeof(int)+2;
                break;
                case IPMC_SENSOR_TYPE_POWER:
                    *data = (local_data_pool.sensorAlarmTab.other>>3) & 0x1;
                    len = SIZEOF_SERIAL_PKT+sizeof(int)+2;
                break;
                case IPMC_SENSOR_TYPE_FRU_STAT://fru state
                    *data = local_data_pool.sensorAlarmTab.other & 0x1;
                    len = SIZEOF_SERIAL_PKT+sizeof(int)+2;
                break;
                case IPMC_SENSOR_TYPE_CPU_HEALTH://CPU health
                    *data = (local_data_pool.sensorAlarmTab.other >> 1) & 0x1; 
                    len = SIZEOF_SERIAL_PKT+sizeof(int)+2;
                break;
                case IPMC_SENSOR_TYPE_HOTSWAP://hotswap
                    *data  = (local_data_pool.sensorAlarmTab.other >> 2) & 0x1;
                    len = SIZEOF_SERIAL_PKT+sizeof(int)+2;
                break;

                default:break;
            }
        }
        else if(area == IPMC_LIMIT_DATA_AREA)//limit
        {
            switch(type)
            {
                case IPMC_SENSOR_TYPE_TEMPERATURE://temperature
                    if(id < NUM_OF_TEMP_SENSOR)
                    {
                        *data =  local_data_pool.sensorLimitTab.temp[id];
                        len = SIZEOF_SERIAL_PKT+sizeof(int)+2;
                    }
                    else if(id < 0xFF)
                    {
                        *data =  0x00;
                        len = SIZEOF_SERIAL_PKT+sizeof(int)+2;
                    }
                    else
                    {
                        memcpy(data, local_data_pool.sensorLimitTab.temp, sizeof(local_data_pool.sensorLimitTab.temp));
                        len = SIZEOF_SERIAL_PKT+sizeof(local_data_pool.sensorLimitTab.temp)+2;
                    }
                break;
                
                case IPMC_SENSOR_TYPE_VTG_CUR://voltage
                    if(id < NUM_OF_VTG_SENSOR)
                    {
                        //*data = * (local_data_pool.sensorLimitTab.vtg.fpga_core_vtg +id);
                        len = SIZEOF_SERIAL_PKT+sizeof(int)+2;
                    }
                    else if(id < 0xFF)
                    {
                        *data =  0x00;
                        len = SIZEOF_SERIAL_PKT+sizeof(int)+2;
                    }
                    else
                    {
                        //memcpy(data, local_data_pool.sensorLimitTab.vtg.fpga_core_vtg, sizeof(local_data_pool.sensorLimitTab.vtg));
                        len = SIZEOF_SERIAL_PKT+sizeof(local_data_pool.sensorLimitTab.vtg)+2;
                    }
                    
                break;
                
                default:break;
            }

        }
        else if(area == IPMC_SHMC)
        {
            switch(type)
            {
                case SHMC_SHELFADDR:
                    *data = local_data_pool.fruAddr.shelfAddr;
                break;
                case SHMC_CASEADDR:
                    *data = local_data_pool.fruAddr.caseAddr;
                break;
                case SHMC_LOGICSLOTID:
                    *data = local_data_pool.fruAddr.logicSlotId;
                break;
                case SHMC_CASE_VOLTAGE:
                    shmcIccmGet(GET_48DC_VOLTAGE, data);
                break;
                case SHMC_CASE_TEMPERATURE:
                    shmcIccmGet(GET_TEMPERATURE, data);
                break;
                case SHMC_CASE_FAN:
                    shmcIccmGet(GET_FAN_LEVEL, data);//HIGH 16 bits for fan speed, the low 16bits for fan level.
                break;
                case SHMC_CASE_TYPE:
                    shmcIccmGet(GET_CHASSIS_TYPE, data);//HIGH 16 bits for fan speed, the low 16bits for fan level.
                break;
                default:break;
            }
            len = SIZEOF_SERIAL_PKT+sizeof(int)+2;
        }
#if 0
        else if(area == 0xF)//all sensor state
        {
            switch(pget[sizeof(SERIAL_HEAD) + 1]&0x0F)
            {
                case IPMC_SENSOR_TYPE_TEMPERATURE:
                break;
                case IPMC_SENSOR_TYPE_VTG_CUR:
                break;
                case 0xF:
                    //copy all local data pool to send
                    buf[4] = 3+sizeof(local_data_pool);//valid date length
                    memcpy(buf+sizeof(SERIAL_HEAD) + sizeof(SERIAL_BODY), (unsigned char *)&local_data_pool, sizeof(local_data_pool));
                    //send_buf(USART2, buf, 8+sizeof(local_data_pool));
                    len =buf[4]+5;
                    
                break;
                default:
                break;
            }
        }
#endif			

        else
        {
            len = SIZEOF_SERIAL_HEAD;
        }
            ;
        rplhead->bodyLen = len - SIZEOF_SERIAL_HEAD;
        crc16 = (unsigned short *)(buf+len-2);// crc16 value palace the last 2 bytes of a packet.
        *crc16 = uartCrc16(buf, len-2);
	if(GX36_heart_beat())
            send_buf(USART2, buf, len);//send_buf(USART2, buf, len+2);
    
	}

	return OK;
}

void USART2_IRQHandler()
{
    unsigned char *pRxBufHead = RxBufRing.pBufPut;
    SERIAL_HEAD *phead = (SERIAL_HEAD *)pRxBufHead;
    if(sys_data.clock_cycle - uart_recv_cycle_last > 10)               //>100ms 表示上次收包未收完整
        RxCounter = 0;
    uart_recv_cycle_last = sys_data.clock_cycle;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)       //检测是否是接收中断
    {	
        //USART_ClearITPendingBit(USART2, USART_IT_RXNE);             //清除中断标志    
        //pRxBufHead[RxCounter++]= USART_ReceiveData(USART2);
        pRxBufHead[RxCounter++] = USART2->DR & (uint16_t)0x01FF;
        if(phead->magic1 !=0xaa)
        {
            RxCounter = 0;
            return;
        }
        if(RxCounter == SIZEOF_SERIAL_HEAD)
        {
            SizeOfRxBody = phead->bodyLen;
        }
        if(RxCounter == SizeOfRxBody+SIZEOF_SERIAL_HEAD +2)	 //如果接收数据量等于接收数据长度+6字节头+2字节checksum
        {
            RxBufRing.pBufGet = RxBufRing.pBufPut;
            sys_data.uart2msg   %=   USART_RX_BUFFR_CNT;
            sys_data.uart2msg++;
            RxBufRing.pBufPut = RxBufRing.usartBuf[sys_data.uart2msg];
            RxCounter=0;
        }
    }
  //  if(USART_GetITStatus(USART2, USART_IT_TXNE) != RESET)
  //  {
        //if(uart2MsgQueue[0].data != NULL || uart2MsgQueue[1].data != NULL)
  //  }
}

void DMA1_Stream4_IRQHandler()
{
    DMA_Cmd(DMA1_Stream4, DISABLE);
    DMA_ClearFlag(DMA1_Stream4, DMA_FLAG_TCIF4 | DMA_FLAG_HTIF4 | DMA_FLAG_TEIF4 | DMA_FLAG_DMEIF4 | DMA_FLAG_FEIF4);
}
