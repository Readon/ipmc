#include <config.h>
#include "system.h"

#include "FreeRTOS.h"
#include "task.h"
#include "flash_if.h"

xTaskHandle xHandle[xTASK_MAX];



int main()
{			 
    srvBoardInit();
    xTaskCreate( vDebugLedTask, (signed char *)        "DEBUG_LED",         configMINIMAL_STACK_SIZE, 
            NULL, ( tskIDLE_PRIORITY  + 1 ), &xHandle[xTASK_DEBUG_LED] );
    xTaskCreate( vSensorScanTask, (signed char *)        "SENSORSCAN",         configMINIMAL_STACK_SIZE, 
            NULL, ( tskIDLE_PRIORITY  + 2 ), &xHandle[xTASK_SENSOR_SCAN] );
   // xTaskCreate( vCaseManagemene, (signed char *)     "ALARM_SCAN",      configMINIMAL_STACK_SIZE, 
   //         NULL, ( tskIDLE_PRIORITY  + 3 ), &xHandle[xTASK_ALARM_SCAN] );
    xTaskCreate(vUartCmcAnalize, (signed char *)       "CMD_ANALIZY",     1024, 
           NULL, ( tskIDLE_PRIORITY  + 4 ), &xHandle[xTASK_CMD_ANALIZY] );
    xTaskCreate( vFishEcho,           (signed char *)        "FISH_ECHO",          1024, 
            NULL, ( tskIDLE_PRIORITY  + 6 ), &xHandle[xTASK_FISH_ECHO] );
    xTaskCreate(vHeartBeatDect, (signed char *)         "HEART_BEAT",        configMINIMAL_STACK_SIZE,
           NULL, (tskIDLE_PRIORITY  + 7 ), &xHandle[xTASK_HEART_BETA]);
    
    xTaskCreate( vPowerManageTask, (signed char *) "POWER_MNG",      configMINIMAL_STACK_SIZE, 
            NULL, ( tskIDLE_PRIORITY  + 8 ), &xHandle[xTASK_POWER_MNG] );
    

    vTaskStartScheduler(); // This should never return.
    while(1);
 
    return 0;
}

/*EOF*/

