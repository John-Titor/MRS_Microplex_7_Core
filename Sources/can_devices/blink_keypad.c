/**
 * blink_keypad.c
 */

#include <WDog1.h>

#include <config.h>

#include <core/lib.h>
#include <core/pt.h>
#include <core/timer.h>
#include <can_devices/blink_keypad.h>

#ifdef BK_FIXED_KEYPAD_ID
# define BK_DEFAULT_KEYPAD_ID   BK_FIXED_KEYPAD_ID
#else
# define BK_DEFAULT_KEYPAD_ID   0xff
#endif

static struct {
    uint8_t     counter;
    uint8_t     reported_state;
} key_state[BK_MAX_KEYS];

static struct {
    uint8_t     color_a:4;
    uint8_t     color_b:4;
    uint8_t     pattern;
} led_state[BK_MAX_KEYS];

static uint8_t          num_keys;
static uint8_t          keypad_id = BK_DEFAULT_KEYPAD_ID;          
static uint8_t          backlight_intensity;
static uint8_t          key_intensity;
static uint8_t          update_flags;

#define UPDATE_KEYS         0x1
#define UPDATE_INTENSITY    0x2

static void             bk_thread(struct pt *pt);
static void             bk_tick(void);
static timer_call_t     bk_tick_call = {
        bk_tick,
        BK_TICK_PERIOD_MS,
        BK_TICK_PERIOD_MS
};

static timer_t          bk_idle_timer;

void
bk_init(void)
{
    timer_call_register(bk_tick_call);
    timer_register(bk_idle_timer);
}

void
bk_loop(void)
{
    static struct pt bk_pt;
    
    bk_thread(&bk_pt);
}

uint8_t
bk_num_keys(void)
{
	return num_keys;
}

bool
bk_can_receive(can_buf_t *buf)
{
    uint8_t     i;
    
    if (keypad_id == 0xff) {
        // Process a keypad boot message and learn the keypad ID.
        if ((buf->id >= 0x700)
            && (buf->id <= 0x77f)
            && (buf->dlc == 1)
            && (buf->data[0] == 0)) {

            keypad_id = buf->id & 0x7f;
            return TRUE;
        }
        // without a keypad ID we can't do anything else...
        return FALSE;
    }
    
    if (num_keys == 0) {
        // Process a register continuation that we expect to be from a
        // Model ID read. Be fairly conservative.
        if ((buf->id == 0x580 + keypad_id)
            && (buf->dlc == 8)
            && (buf->data[0] == 0)
            && (buf->data[1] == 'P')
            && (buf->data[2] == 'K')
            && (buf->data[3] == 'P')) {

            uint8_t nk = buf->data[5] - '0';
            if (buf->data[4] == '2') {
                nk <<= 1;
            }
            if (nk <= BK_MAX_KEYS) {
                num_keys = nk;
                return TRUE;
            }
        }
        // without a key count we can't do anything else
        return FALSE;
    }
    
    // process a key-state message
    if ((buf->id == (0x180 + keypad_id))
            && (buf->dlc == 5)
            && (buf->data[2] == 0)
            && (buf->data[3] == 0)) {

        // for each key...
        for (i = 0; i < num_keys; i ++) {
            // if it is currently pressed...
            if (buf->data[i / 8] & (1 << (i % 8))) {
                // and it wasn't pressed...
                if (key_state[i].counter == 0) {
                    // start the counter
                    key_state[i].counter = 1;
                }
            }
            // if it was not pressed
            else {
                // reset the counter
                key_state[i].counter = 0;
            }
        }
        timer_reset(bk_idle_timer, BK_IDLE_TIMEOUT_MS);
        return TRUE;
    }
    return FALSE;
}

uint8_t
bk_get_event(void)
{
    uint8_t i;
    uint8_t current_state = BK_EVENT_NONE;
    
    // scan key state & look for un-reported state changes
    for (i = 0; i < num_keys; i++) {
        
    	// get the most recent event that occurred for the key
    	current_state = bk_get_key_event(i);

        // always report state transitions in order, don't skip any
    	if (current_state > key_state[i].reported_state) {
        	current_state = key_state[i].reported_state + 0x10;
        }
        if (current_state != key_state[i].reported_state) {
        	// report a change of state
            break;
        }
    }
    if (i < num_keys) {
        key_state[i].reported_state = current_state;
        return current_state | i;
    }
    return BK_EVENT_NONE;
}

uint8_t
bk_get_key_event(uint8_t key)
{
    if (key_state[key].counter >= BK_LONG_PRESS_3_TICKS) {
        return BK_EVENT_LONG_PRESS_3;
    } else if (key_state[key].counter >= BK_LONG_PRESS_2_TICKS) {
    	return BK_EVENT_LONG_PRESS_2;
    } else if (key_state[key].counter >= BK_LONG_PRESS_1_TICKS) {
    	return BK_EVENT_LONG_PRESS_1;
    } else if (key_state[key].counter >= BK_SHORT_PRESS_TICKS) {
    	return BK_EVENT_SHORT_PRESS;
    }
    return BK_EVENT_RELEASE;
}

void
bk_set_key_led(uint8_t key, uint8_t colors, uint8_t pattern)
{
    led_state[key].color_a = colors & BK_COLOR_MASK;
    led_state[key].color_b = (colors >> 4) & BK_COLOR_MASK;
    led_state[key].pattern = pattern;
    update_flags |= UPDATE_KEYS;
}

void
bk_set_key_intensity(uint8_t intensity) 
{
    key_intensity = intensity & BK_MAX_INTENSITY;    
    update_flags |= UPDATE_INTENSITY;
}

void
bk_set_backlight_intensity(uint8_t intensity)
{
    backlight_intensity = intensity & BK_MAX_INTENSITY;
    update_flags |= UPDATE_INTENSITY;
}

static void
bk_send_led_update(uint8_t phase)
{
    const uint8_t phase_mask = 1 << phase;
    const uint8_t bit_offset = (num_keys == 12) ? 12 : (num_keys == 10) ? 16 : 8;
    uint8_t i;
    uint8_t data[8] = {0};

    for (i = 0; i < num_keys; i++) {
        uint8_t color, offset;

        if (led_state[i].pattern & phase_mask) {
            color = led_state[i].color_b;
        } else {
            color = led_state[i].color_a;
        }
        offset = i;
        if (color & BK_RED) {
            data[offset / 8] |= 1 << (offset % 8);
        }
        offset += bit_offset;
        if (color & BK_GREEN) {
            data[offset / 8] |= 1 << (offset % 8);
        }
        offset += bit_offset;
        if (color & BK_BLUE) {
            data[offset / 8] |= 1 << (offset % 8);
        }
    }
    can_tx_async(0x200 + keypad_id, sizeof(data), data);    
}

static void
bk_send_intensity_update()
{
    uint8_t data[8] = {0};
    
    data[0] = key_intensity;
    can_tx_async(0x400 + keypad_id, sizeof(data), data);
    data[0] = backlight_intensity;
    can_tx_async(0x500 + keypad_id, sizeof(data), data);
}

static void
bk_thread(struct pt *pt)
{
    static timer_t      blink_timer;
    static uint8_t      blink_phase;

    pt_begin(pt);
    timer_register(blink_timer);

    for (;;) {
        // We start here with no idea about keypad ID (unless hardcoded) or size.
        
        do {
            // Give the keypad time to start talking to us after whatever we
            // just did to it.
            //
            pt_delay(pt, blink_timer, BK_UPDATE_PERIOD_MS * 2);
            
            // First, try to find a keypad.
            //
            // ID may be hardcoded, or discovered by receiving a boot-up message. 
            // We can't do anything else until we know what it is.
            //
            if (keypad_id == 0xff) {
                
                // Send reset-all and hope that shakes a boot-up message
                // out of the keypad. If it has been turned off, we have
                // to have it hardcoded.
                //
                static const uint8_t bk_reset_all[] = {0x81, 0x00};
                can_tx_ordered(0x00, sizeof(bk_reset_all), bk_reset_all);
                continue;
            }

            // Work out how many keys it has.
            //
            // The only way to do this seems to be to read the Model ID
            // and parse out the x/y dimensions from the text. Insane.
            //
            if (num_keys == 0) {
                // We only read enough of the Model ID to get the name out.
                static const uint8_t bk_get_model_id[8] = { 0x40, 0x0b, 0x10 };
                static const uint8_t bk_get_next_register[8] = { 0x60 };

                can_tx_ordered(0x600 + keypad_id, sizeof(bk_get_model_id), bk_get_model_id);
                pt_yield(pt);
                can_tx_ordered(0x600 + keypad_id, sizeof(bk_get_next_register), bk_get_next_register);
                continue;
            }

        } while ((num_keys == 0) || (keypad_id == 0xff));

        // we've found a keypad and we know how big it is, use it
        for (;;) {
            
            // Wait until it's time to send an update; either because it's time
            // for a new animation iteration or because an LED state change was
            // requested.
            pt_wait(pt, timer_expired(blink_timer) || (update_flags & UPDATE_KEYS));
            if (timer_expired(blink_timer)) {
                timer_reset(blink_timer, BK_BLINK_PERIOD_MS);
                blink_phase = (blink_phase + 1) & 0x7;
            }
            bk_send_led_update(blink_phase);
            
            if (update_flags & UPDATE_INTENSITY) {
                bk_send_intensity_update();
            }
            update_flags = 0;
            
            // if the keypad disappears, release every key
            if (timer_expired(bk_idle_timer)) {
                uint8_t i;
                
                for (i = 0; i < num_keys; i++) {
                    key_state[i].counter = 0;
                }
            }
        }
    }
    pt_end(pt);
}

static void
bk_tick(void)
{
    int i;
    
    for (i = 0; i < num_keys; i++) {
        if ((key_state[i].counter > 0) && (key_state[i].counter < 255)) {
            key_state[i].counter++;
        }
    }
}
