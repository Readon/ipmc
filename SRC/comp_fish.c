
#include <stdio.h>
#include "config.h"
#include "system.h"
#include "version.h"
#include "comp_uart.h"
#include "comp_fish.h"
#include "ipmc.h"


#define CMD_HISTORY_ARRAY  100
#define CMD_HISTORY_TAIL_BUFFER 28
#define CMD_HISTORY_ARRAY_TOP   (CMD_HISTORY_ARRAY+CMD_HISTORY_TAIL_BUFFER)
//extern valiable
extern IPMC_DATA_AREA local_data_pool;
extern FISH_CMD_t fish_command_list[FISH_MAX_CMD_SUPPORT];
unsigned char now_allocked;//已经申请掉的code command号码
char history[CMD_HISTORY_ARRAY+CMD_HISTORY_TAIL_BUFFER];//为了支持上、下键，搜索历史命令。历史命令用命令码来记录
//在history数组中，为了使编程复杂度。我们使用512字节来当做history存储的环形缓冲，用550-512字节当做"末尾缓冲区"
//所谓"末尾缓冲区"的意思是当512字节环形缓冲区在存储最后一个字串不够用时，550-512这部分将被使用，随后，
//负责存储的指针立马回到环形缓冲区头部。如果最后一个字串较长，以致超过了550-512+n,n为512最后几个字符。那么，多余的字符
//将被cut掉。history总是打印最近的512存储区中包含的字串。
char *phistory = NULL;

unsigned int str2inteter(char *buf, int len)
{
    unsigned int result = 0;
    int i;
    for(i = 0; i < len; i++)
    {
        if(*buf >= '0' && *buf <= '9')
        {
            result |= ((*buf-'0')&0xf) << (len-i-1)*4;
        }
        else if(*buf >= 'a' && *buf <= 'f')
            result |= ((*buf-'a'+10)&0xF) << (len-i-1)*4;
        else
        {
            result = 0;
            break;
        }
        buf++;
    }
    return result;
}

unsigned int parameterlen(char *buf)
{
    int len;
    len = 0;
    while((*buf != ' ') && (*buf != 0x00))
    {
        buf++;
        len++;
    }
    return len;
}


unsigned char fishAllocCmdCode()
{
    return now_allocked;//新申请的command依次放在fish_command_list数组中，用now_allocked来记录当前已被申请的号码
}

FUNC help_main(void *arg)
{
    unsigned char i;
    printf("support command:\r\n");
    i=0;
    while(fish_command_list[i].name != NULL)
    {
        printf("%s\t%s\r\n",fish_command_list[i].name, fish_command_list[i].description);
        i++;
    }
    return 0;
    
}


//Fish version description
FUNC version_main(void *arg)
{
    printf("IPMC version %s\r\n", IPMC_VER_STR);
    return 0;
}

FUNC board_main(void *arg)
{
    arg =arg;//to avoid compile warnning
    srvAmfDisplay();
    sys_data.display_all_flg = 1;
    return 0;
}

//support:
//-a, address
FUNC memory_main(void *arg)
{
    int i;
    int *addr = NULL;
    addr = (int *)(*((int *)arg+1));
    
 //   printf("param1 %c num1 0x%8x;param2 %c num2 0x%8x;param3 %c num3 0x%8x", ((int*)arg)[0],((int*)arg)[1], ((int*)arg)[2],((int*)arg)[3], ((int*)arg)[4],((int*)arg)[5]);
    if((*((int *)arg+1)) < 0x20000000 || (*((int *)arg+1)) > 0x20020000-64)
    {
        printf("read addr 0x%08x error\r\n", (int)addr);
        return 0;
    }
    switch(*((int *)arg))
    {
        case 'a':
            printf("\r\n%08x: ",(int)addr);
            for(i = 0; i < 16; i++)
            {
                if(i%4 == 0 && i!= 0)
                    printf("\r\n%08x: ", (int)(i+addr));
                printf("%08x ",addr[i]);
            }
        break;
        default:printf("Unkonw option -%c\r\n",*((int *)arg));break;
    }
    return 0; 
}

//update firm by serial port.
FUNC olupd_main(void *arg)
{
    updFirmware();
    return 0;
}

FUNC watch_main(void *arg)
{
    int i;
    unsigned char *watch = (unsigned char *)&local_data_pool; 
    
    printf("\r\n%04x: ",0);
    for(i = 0;i<sizeof(local_data_pool);i++)
    {
        if(i%16 == 0 && i!= 0)
            printf("\r\n%04x: ", i);
        printf("%02x ",watch[i]);
    }
    return 0;
}

//found history command
FUNC history_main()
{
    int cnt;
    char *ptr = NULL;
    ptr = phistory;
    //ptr += 1;
    do{
        if(*ptr == '\0')
        {
            ptr++;
            if(ptr >= &history[CMD_HISTORY_ARRAY])
                ptr = history;
            continue;
         }

        printf("%s\r\n", ptr);
        ptr += strlen(ptr);
        if(ptr >= &history[CMD_HISTORY_ARRAY])
            ptr = history;
    }while(ptr != phistory);
    return 0;
}


//====================================================
char fish_command_register(FISH_CMD_t *pcmd)
{
    if(pcmd == NULL)
        return 0;
    fish_command_list[pcmd->code].code = pcmd->code;
    fish_command_list[pcmd->code].name = pcmd->name;
    fish_command_list[pcmd->code].exec = pcmd->exec;
    fish_command_list[pcmd->code].description = pcmd->description;
    now_allocked ++;
    return 0;
}
//Fish component
void fish_init(void)
{

    FISH_CMD_t cmd;
    p_uart_produce = uart_cache;
    p_uart_custom = uart_cache;
    cmd_counter = 0;
    now_allocked = 0;
    phistory = history;
    memset(uart_cache, 0x0, sizeof(uart_cache));
    memset(uart_buffer, 0x0, sizeof(uart_buffer));
    memset(history, 0x0, sizeof(history));
    memset(fish_command_list, 0x0, sizeof(fish_command_list));
    
    //!!some system command must register at startup time.
    // register command "help"
    cmd.code          = 0;
    cmd.name          = "help";
    cmd.exec          = help_main;
    cmd.description   = "print this message";
    fish_command_register(&cmd);
    //!register command "board"
    //alloc a command code
     
    cmd.code          = fishAllocCmdCode();
    cmd.name          = "board";
    cmd.exec          = board_main;
    cmd.description   = "List board state";
    fish_command_register(&cmd);

    cmd.code          = fishAllocCmdCode();
    cmd.name          = "memory";
    cmd.exec          = memory_main;
    cmd.description   = "watch a specific memory address";
    fish_command_register(&cmd);

    cmd.code          = fishAllocCmdCode();
    cmd.name          = "history";
    cmd.exec          = history_main;
    cmd.description   = "print all history cmd";
    fish_command_register(&cmd);

    cmd.code          = fishAllocCmdCode();
    cmd.name          = "olupd";
    cmd.exec          = olupd_main;
    cmd.description   = "online update STM32 fireware";
    fish_command_register(&cmd);

    cmd.code          = fishAllocCmdCode();
    cmd.name          = "watch";
    cmd.exec          = watch_main;
    cmd.description   = "watch local data pool";
    fish_command_register(&cmd);

    
}



unsigned char fish_find_possible_command()
{
    unsigned char i, j, print_all_cmd;
    unsigned char possible_cmd[64];//possible_cmd数组中记录了,tab键补全可能的命令。其中第一个字节为可能命令的总数。用于打印这些命令时用作引导。因此，无需初始化该数组
    possible_cmd[0] = 0;
    j=1;print_all_cmd = 0;
    for(i = 0; i < FISH_MAX_CMD_SUPPORT; i++)
    {
        if(cmd_counter == 0)
        {
            //mark all
            print_all_cmd = 1;
            break;
        }
         //比较某个命令的钱cmd_counter个字符，如果符合，则将该命令置为可能被选命令
         if(memcmp(uart_buffer, fish_command_list[i].name, cmd_counter) == 0)//cmd_counter记录了当前uart_buffer中某个命令的部分字节
         {
              possible_cmd[0] += 1;
              possible_cmd[j]  =  i;
         }
    }
    //做完这些后，打印他们
    printf("\r\nDo you want to find these command?\r\n");
    if(print_all_cmd)
    {
        for(i = 0; i < now_allocked; i++)
            printf("%s\r\n", fish_command_list[i].name);
    }
    else
    {
        for(i = 1; i <= possible_cmd[0]; i++)//i从1开始，因为0被用作计数了
        {
            j = possible_cmd[i];
            printf("%s\r\n", fish_command_list[j].name);
        }
    }
    return 0;
}

unsigned char fish_print_possible_command()
{
    return 0;
}

//规定，命令行参数格式为:
//cmd  -subcmd1   parameter1   -subcmd2   parameter2  ...
//cmd, subcmd, parameter之间必须用空格隔开
//subcmd 必须以 - 开头
//每一个subcmd后必须跟一个参数 parameter，如果没有parameter，则填 0
//parameter 必须为16进制形式，0xFFFFFFFF  最大32bits。
char fish_main()
{
    unsigned char i,j;
    unsigned char cmdlen,argnum;
    int tmp;
    int parameter;
    unsigned int arg[6];//fixedme: we only support on sub command right now. but 10 is for future use.
    //check if p_uart_produce not equal to p_uart_custom
    while(p_uart_custom != p_uart_produce)//something new
    {
        if(*p_uart_custom == FISH_KEYBOARD_RETURN)//we got an return, find & execute command
        {
            
            put('\r');put('\n');
            putstr(IPMC_DEVSCRIPTION);
            if(cmd_counter)//if we really got a command
            {
            //search command in fish_command_list & execute  it
            for(i=0; i<FISH_MAX_CMD_SUPPORT; i++)
            {
                 if(memcmp(uart_buffer, fish_command_list[i].name, strlen(fish_command_list[i].name)) == 0)//+1 mainly because of comparing \0
                 {
                      argnum = 0;
                      //check if it is the command we support
                      cmdlen = strlen(fish_command_list[i].name);
                      if(uart_buffer[cmdlen] == ' ')//if separate is space
                      {
                          for(j = cmdlen;j < cmd_counter;j++)// find the remain parameter
                          {
                              if(uart_buffer[j] == ' ')
                                  ;//skip
                              if(uart_buffer[j] == '-')
                              {
                                  arg[argnum++] = uart_buffer[++j];
                              }
                              if(uart_buffer[j]=='0'&&uart_buffer[j+1] == 'x')// 16
                              {
                                  j += 2;
                                  arg[argnum] = str2inteter(uart_buffer+j, parameterlen(uart_buffer+j));
                                  argnum++;
                              }
                          }
                      }
                      //do the execute
                      put('\r');
                      fish_command_list[i].exec(arg);
                      put('\r');put('\n');
                      putstr(IPMC_DEVSCRIPTION);
                      break;
                 }
            }
            if((phistory+cmd_counter) < &history[CMD_HISTORY_ARRAY-1])
            {
                memcpy(phistory, uart_buffer, cmd_counter);
                phistory +=cmd_counter;
                *phistory = '\0';
                phistory++;
                //when complete, set uart_buffer to zero
                memset(uart_buffer, 0x0, sizeof(uart_buffer));
                cmd_counter = 0;
            }
            else
            {
                tmp = MIN((&history[CMD_HISTORY_ARRAY_TOP-2]-phistory), cmd_counter);//why CMD_HISTORY_ARRAY_TOP-2 ??  last for '\0'
                //printf("history DBG_INFO: it is going to run out of history array, MIN size 0x%d, cmd_counter %d\r\n", tmp, cmd_counter);
                memcpy(phistory, uart_buffer, tmp);
                phistory +=tmp;
                *phistory = '\0';
                phistory = history;
                memset(uart_buffer, 0x0, cmd_counter);
                cmd_counter = 0;
            }
            }
          }//end if(*p_uart_custom == FISH_KEYBOARD_RETURN)
          else if(*p_uart_custom == FISH_KEYBOARD_TABLE)
          {//print the possible command
               fish_find_possible_command();
               *p_uart_custom = 0;
             ///  p_uart_custom = p_uart_produce ;
               putstr(IPMC_DEVSCRIPTION);

               printf("%s", uart_buffer);
             // fish_print_possible_command();
              return 0;
                
          }
          else if(*p_uart_custom == FISH_KEYBOARD_BACKSPACE)//
          {
              if(cmd_counter > 0)
              {
                  cmd_counter--;
                  uart_buffer[cmd_counter]=0; 
                  put('\b');
                  put(' ');
                  put('\b');
                  
              }
          }
          else if(*p_uart_custom == FISH_KEYBOARD_UP)//found history command
          {
          }
          else if(*p_uart_custom == FISH_KEYBOARD_DOWN)
          {
          }
          else//other char, just echo it
          {
              //if((*p_uart_custom >='a')&&(*p_uart_custom <='z'))//we only support character a~z
             // {
                   put(*p_uart_custom);
                   uart_buffer[cmd_counter] = *p_uart_custom;//IN ORDER TO remember the command
                   cmd_counter ++;
                   cmd_counter %= sizeof(uart_buffer);
           //   }
          }
          
        if(p_uart_custom == &uart_cache[99])
            p_uart_custom = uart_cache;
        else
            p_uart_custom++;
        
    }
    return 0;
}

