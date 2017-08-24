
#ifndef __COMP_UART_H__
#define __COMP_UART_H__

#include <stm32f2xx.h>

/* Low level struct for uart */
#define USART_RX_BUFFR_CNT      2
#define USART_RXMSG_LEN_MAX     128
#define USART_TX_BUFFR_CNT      2
#define USART_TXMSG_LEN_MAX     1024/* 由于内存操作和DMA操作均远快于串口操作，因此，printf在打印一串较长的数据时
                                                          可能会出现打印未结束而下一组待打印的数据来临，因此设置发送区略大 */

#define PRINTF_IO_FLUSH_SIZE   USART_TXMSG_LEN_MAX


typedef struct{
	unsigned char *pBufPut;
	unsigned char *pBufGet;
	//unsigned char *pBufBegin;
	//unsigned char *pBufEnd;
	unsigned char usartBuf[USART_RX_BUFFR_CNT][USART_RXMSG_LEN_MAX];
}USART_RX_BUFFER_RING;

typedef struct {
    char *pRxCnsumer;
    char *pRxProductor;
    char *pTxConsumer;
    char *pTxProductor;
    __IO int  txcnt;
    __IO char uartTxBuf[USART_TX_BUFFR_CNT][USART_TXMSG_LEN_MAX];
    __IO char uartRxBuf[USART_RX_BUFFR_CNT][USART_RXMSG_LEN_MAX];
}DEV_UART;







/* application layer struct */

#define USART_HARD_LEN		5
#define USART_BODY_LEN_MAX	(USART_RXMSG_LEN_MAX - USART_HARD_LEN)
#define USART_VERIFY_LEN	        2
#define SIZEOF_SERIAL_HEAD                 7// sizeof(SERIAL_HEAD)+sizeof(SERIAL_BODY)
#define SIZEOF_SERIAL_BODY         3
#define SIZEOF_SERIAL_PKT            10


typedef  struct SERIAL_HEAD_CTX
{
	unsigned char magic1;  /* 0xaa */
	unsigned char magic2;  /* 0x55 */
	unsigned char srcSlot;  
	unsigned char dstSlot;  
	unsigned char fsn;
	unsigned char bodyLen; /* body len :not include crc checkout 2bytes*/
//	unsigned char body[0];
}SERIAL_HEAD;
typedef  union SERIAL_BODY_CTX
{
         struct {
            char reserve        : 5;
            char isheartbeat  : 1;
            char isset             : 1;
            char gcb              : 1;
            char type             : 4;
            char area             : 4;
            char id                 : 8;
            }s;
        char bodyctx[3];// total 3 control bytes
}SERIAL_BODY;

typedef struct uartMsgQueue 
{
    char *buf;
    unsigned int len;
}UART_MSG_QUEUE_t;
char send_byte(USART_TypeDef * UARTx, unsigned char ch);
char get_byte(USART_TypeDef * UARTx, unsigned char *ch);
char put(unsigned char ch);


//valiable used by fis
extern __IO char uart_cache[100];
extern __IO char uart_buffer[32];
extern char *p_uart_produce;
extern char *p_uart_custom;
extern  unsigned char cmd_counter;


#endif
/*EOF*/
