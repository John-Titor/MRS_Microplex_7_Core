/* ###################################################################
**     Filename    : main.c
**     Project     : 7x_experiment
**     Processor   : MC9S08DZ60CLH
**     Version     : Driver 01.12
**     Compiler    : CodeWarrior HCS08 C Compiler
**     Date/Time   : 2021-04-22, 15:59, # CodeGen: 0
**     Abstract    :
**         Main module.
**         This module contains user's application code.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file main.c
** @version 01.12
** @brief
**         Main module.
**         This module contains user's application code.
*/         
/*!
**  @addtogroup main_module main module documentation
**  @{
*/         
/* MODULE main */


/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "Events.h"
#include "DI_CAN_ERR.h"
#include "CAN_EN.h"
#include "CAN_STB_N.h"
#include "CAN_WAKE.h"
#include "DO_1.h"
#include "DO_30V_10V_1.h"
#include "DO_30V_10V_2.h"
#include "DO_30V_10V_3.h"
#include "DO_2.h"
#include "AD1.h"
#include "DO_POWER.h"
#include "CAN1.h"
#include "IEE1.h"
#include "PWM_5.h"
#include "PWM_6.h"
#include "PWM_7.h"
#include "PWM_1.h"
#include "PWM_3.h"
#include "PWM_4.h"
#include "PWM_2.h"
#include "TickTimer.h"
#include "WDog1.h"
/* Include shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"

/* User includes (#include below this line is not maintained by Processor Expert) */
#include <core/can.h>
#include <core/io.h>
#include <core/lib.h>
#include <core/pt.h>

#include <can_devices/blink_keypad.h>

static struct pt pt_can_listener;

extern void app_init(void);
extern void app_loop(void);

void main(void)
{
    /* Write your local variable definition here */

    /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
    PE_low_level_init();
    /*** End of Processor Expert internal initialization.                    ***/

    // Start with a freshly-reset watchdog.
    (void)WDog1_Clear();

    // Fix CAN config - PE_low_level_init doesn't know about the EEPROM.
    can_reinit();

    // Print / trace work now.
    print("start");
    can_trace(0xff);

    // Disable PWM mode on 7X non-PWM outputs
#ifdef TARGET_7X
    // XXX TODO flip PWM_5 over to frequency-capture mode somehow.
    (void)PWM_6_Disable();
    AI_3_PU_ClrVal();
    (void)PWM_7_Disable();
    DO_20MA_1_ClrVal();
#endif // TARGET_7X
    
    // Start the ADC in continuous mode.
    (void)AD1_Start();
    
#ifdef CONFIG_WITH_BLINK_KEYPAD
    bk_init();
#endif
    
    // Init the application.
    app_init();
  
    // Main loop; never exits
    for (;;) {
        (void)WDog1_Clear();                            // must be reset every 1s

        // Run the CAN listener thread and any message-reception callouts
        can_listen(&pt_can_listener);
      
#ifdef CONFIG_WITH_BLINK_KEYPAD
        bk_loop();
#endif
        
        // Run any registered threads
        pt_list_run();
        
        // Run the application.
        app_loop();
    }

  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.3 [05.09]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
