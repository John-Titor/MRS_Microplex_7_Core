/*
 * Application top-level logic.
 */

#include <CAN_EN.h>
#include <CAN_STB_N.h>
#include <DO_HSD_2.h>
#include <DO_POWER.h>
#include <WDog1.h>

#include "can.h"
#include "lib.h"
#include "pt.h"
#include "timer.h"

struct pt pt_can_listener;

void
app_main()
{
	// Start with a freshly-reset watchdog.
    (void)WDog1_Clear();
    DO_HSD_2_SetVal();

    // Say awake even if KL15 is not present.
    DO_POWER_SetVal();

    // Ensure the CAN transceiver is enabled.
    CAN_STB_N_SetVal();
    CAN_EN_SetVal();

    // Fix CAN config and hook up printf.
    // XXX TODO can / should we just use the bootloader CAN config as-is?
    can_reinit();
    print("start");

    // main loop
    for (;;) {
        (void)WDog1_Clear();                            // must be reset every 1s

        // Run the CAN listener thread and any message-reception callouts
        can_listen(&pt_can_listener);
    }
}

/*
 * Callbacks.
 */
void
app_can_receive(can_buf_t *buf)
{
}

void
app_can_idle(bool is_idle)
{
}
