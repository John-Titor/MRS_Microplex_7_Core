/*
 * hsd_calibration.h
 *
 * HSD calibration data is placed in EEPROM at the location that MRS
 * claim for it (0x14c8). The format is:
 * 
 * struct eeprom_hsd_cal {
 *     uint16_t		cal_1A[8];
 *     uint16_t		cal_2_5A[8];
 *     uint16_t		cal_10V[8];
 * };
 * 
 * Notes:
 *     only 4 entries in each array are populated for 7X
 *     no cal_10V entries are populated for 7H
 *     no entries at all are populated for 7L
 * 
 */

#include <AD1.h>
#include <IEE1.h>

#include <core/io.h>
#include <core/pt.h>
#include <core/timer.h>

// disable "implicit concatenation of strings"
#pragma MESSAGE DISABLE C3303

#define APPLET_INIT	hsd_calibration_init
#define APPLET_LOOP hsd_calibration_loop

#ifdef TARGET_7L
#error No current sense on Microplex 7L
#endif
#ifdef TARGET_7X
#define NUM_CHANNELS			4
#endif
#ifdef TARGET_7H
#define NUM_CHANNELS			7
#endif
#define EE_CAL_BASE_1A			(IEE1_AREA_START + 0xc8)
#define EE_CAL_BASE_2_5A		(IEE1_AREA_START + 0xd8)
#define EE_CAL_BASE_10V			(IEE1_AREA_START + 0xe8)

// these values assume 65535 FSD, i.e. GetVal16.
#define AD_1A_PLAUSIBLE_MIN     6000
#define AD_1A_PLAUSIBLE_MAX     8000
#define AD_2_5A_PLAUSIBLE_MIN  	17500
#define AD_2_5A_PLAUSIBLE_MAX   19500
#ifdef TARGET_7X
#define AD_10V_PLAUSIBLE_MIN	19000
#define AD_10V_PLAUSIBLE_MAX	21000
#endif

#define STABLE_DEVIATION        (1 << 8)
#define NUM_SAMPLES             20
#define SETTLE_TIME_MS          1000    // XXX do we want a soak time?

static void         hsd_calibration_thread(struct pt *pt);
static const uint8_t hsd_cs_channels[] = {   
        AI_CS_1,
        AI_CS_2,
        AI_CS_3,
        AI_CS_4,
#ifdef TARGET_7H
        AI_CS_5,
        AI_CS_6,
        AI_CS_7
#endif
};

#ifdef TARGET_7X
static const uint8_t hsd_vs_channels[] = {
		AI_OP_1,
		AI_OP_2,
		AI_OP_3,
		AI_OP_4
};
#define HSD_VS_CHANNEL	0x80
#endif

static uint16_t hsd_samples[NUM_SAMPLES];
static uint8_t hsd_sample_index;


void
hsd_calibration_init(void)
{
    print("HSD calibration ...");
    (void)PWM_1_Disable();
    (void)PWM_2_Disable();
    (void)PWM_3_Disable();
    (void)PWM_4_Disable();
#ifdef TARGET_7H
    (void)PWM_5_Disable();
    (void)PWM_6_Disable();
    (void)PWM_7_Disable();
#endif    
    print("    init done.");
}

void
hsd_calibration_loop(void)
{
    static struct pt	pt;
    hsd_calibration_thread(&pt);
}

static void
select_channel(uint8_t channel)
{
    (void)PWM_1_ClrValue();
    (void)PWM_2_ClrValue();
    (void)PWM_3_ClrValue();
    (void)PWM_4_ClrValue();
#ifdef TARGET_7H
    (void)PWM_5_ClrValue();
    (void)PWM_6_ClrValue();
    (void)PWM_7_ClrValue();
	DO_HSD_SEN1_ClrVal();
	DO_HSD_SEN2_ClrVal();
#endif
#ifdef TARGET_7X
    DO_HSD_SEN_ClrVal();
#endif
    
    switch (channel) {
    case 0:
        PWM_1_SetValue();
        break;
    case 1:
        PWM_2_SetValue();
        break;
    case 2:
        PWM_3_SetValue();
        break;
    case 3:
        PWM_4_SetValue();
        break;
#ifdef TARGET_7H
    case 4:
        PWM_5_SetValue();
        break;
    case 5:
        PWM_6_SetValue();
        break;
    case 6:
        PWM_7_SetValue();
        break;
#endif
    }
}

static uint16_t
sample_plausible(uint16_t plausible_min, uint16_t plausible_max)
{
    uint16_t least = 0xffff;
    uint16_t most = 0;
    uint32_t sum = 0;
    uint16_t avg;
    uint8_t i;
    uint8_t counted = 0;
    
    for (i = 0; i < NUM_SAMPLES; i++) {
        uint16_t sample = hsd_samples[i];
        
        // ignore truly bogus samples
        if ((sample < plausible_min) || (sample > plausible_max)) {
            continue;
        }
        // track the least and greatest value
        if (sample < least) {
            least = sample;
        }
        if (sample > most) {
            most = sample;
        }
        // sum values for averaging
        sum += sample;
        
        // count values summed
        counted++;
    }
    avg = (uint16_t)(sum / counted);
    
    // need to have counted at least 90% of samples
    if ((NUM_SAMPLES - counted) > (NUM_SAMPLES / 10)) {
        return 0;
    }
    
    // bail if the deviation is not acceptable
    if ((most - least) > STABLE_DEVIATION) {
        return 0;
    }
    
    // return the seemingly plausible, stable average
    return avg;
}

static void
sample_reset(void)
{
    uint8_t i;
    
    for (i = 0; i < NUM_SAMPLES; i++) {
        hsd_samples[i] = 0;
    }
    hsd_sample_index = 0;
}

static void
sample_channel(uint8_t channel)
{
	uint16_t value;
	uint8_t adc_channel;

#ifdef TARGET_7X
	if (channel & HSD_VS_CHANNEL) {
		adc_channel = hsd_vs_channels[channel - HSD_VS_CHANNEL];
	} else
#endif
	{
		adc_channel = hsd_cs_channels[channel];
	}
	
	if ((AD1_MeasureChan(TRUE, adc_channel) == ERR_OK)
			&& (AD1_GetChanValue16(adc_channel, &value) == ERR_OK)) {
		hsd_samples[hsd_sample_index++] = value;
		if (hsd_sample_index >= NUM_SAMPLES) {
			hsd_sample_index = 0;
		}
	} else {
		print("AD1 conversion error %d/%d", channel, adc_channel);
	}
}

static void
hsd_calibration_thread(struct pt *pt)
{
    static uint8_t  current_channel;
    static timer_t  sample_timer;
    static uint16_t stable_value;

    pt_begin(pt);
    timer_register(sample_timer);
    print("HSD calibration starting");
    (void)AD1_Stop();
    // Do initial calibration setup for 1A
    //
    print("Connect load to HSD_1 and calibrate for 1A"
#ifdef TARGET_7X
    		" and 10V"
#endif
    		".");
    select_channel(0);
    sample_reset();
    do {
        pt_yield(pt);
        sample_channel(0);
    } while (!sample_plausible(AD_1A_PLAUSIBLE_MIN, AD_1A_PLAUSIBLE_MAX));
    print("Disconnect load when calibration complete...");
    do {
        pt_yield(pt);
        sample_channel(0);
    } while (!sample_plausible(0, STABLE_DEVIATION));
    
    // Calibrate channels at 1A and 10V (7X only)
    for (current_channel = 0; 
    		current_channel < (sizeof(hsd_cs_channels) / sizeof(hsd_cs_channels[0]));
    		current_channel++) {
        
        print("Connect 1A load to HSD_%d...", current_channel + 1);
        select_channel(current_channel);
        sample_reset();
        do {
            pt_yield(pt);
            sample_channel(current_channel);
            stable_value = sample_plausible(AD_1A_PLAUSIBLE_MIN, AD_1A_PLAUSIBLE_MAX);
            if (stable_value == 0) {
                timer_reset(sample_timer, SETTLE_TIME_MS);
            }
        } while (!timer_expired(sample_timer));
        print("CS%d: %d", current_channel + 1, stable_value);
        (void)IEE1_SetWord(EE_CAL_BASE_1A + 2 * current_channel, stable_value);
#ifdef TARGET_7X
        do {
        	pt_yield(pt);
        	sample_channel(current_channel + HSD_VS_CHANNEL);
        	stable_value = sample_plausible(AD_10V_PLAUSIBLE_MIN, AD_10V_PLAUSIBLE_MAX);
            if (stable_value == 0) {
                timer_reset(sample_timer, SETTLE_TIME_MS);
            }
        } while (!timer_expired(sample_timer));
        print("OP%d: %d", current_channel + 1, stable_value);
        (void)IEE1_SetWord(EE_CAL_BASE_10V + 2 * current_channel, stable_value);
#endif
    }
    // Do initial calibration setup for 2.5
    //
    print("Connect load to HSD_1 and calibrate for 2.5A.");
    select_channel(0);
    sample_reset();
    do {
        pt_yield(pt);
        sample_channel(0);
        stable_value = sample_plausible(AD_2_5A_PLAUSIBLE_MIN, AD_2_5A_PLAUSIBLE_MAX);
    } while (!stable_value);
    print("CS%d: %d", current_channel + 1, stable_value);
    print("Disconnect load when calibration complete...");
    do {
        pt_yield(pt);
        sample_channel(0);
    } while (!sample_plausible(0, STABLE_DEVIATION));
    
    // Calibrate channels at 2.5A
    for (current_channel = 0; 
    		current_channel < (sizeof(hsd_cs_channels) / sizeof(hsd_cs_channels[0]));
    		current_channel++) {        
        print("Connect 2.5A load to HSD_%d...", current_channel + 1);
        select_channel(current_channel);
        sample_reset();
        do {
            pt_yield(pt);
            sample_channel(current_channel);
            stable_value = sample_plausible(AD_2_5A_PLAUSIBLE_MIN, AD_2_5A_PLAUSIBLE_MAX);
            if (stable_value == 0) {
                timer_reset(sample_timer, SETTLE_TIME_MS);
            }
        } while (!timer_expired(sample_timer));
        print("CS%d: %d", current_channel + 1, stable_value);
        (void)IEE1_SetWord(EE_CAL_BASE_2_5A + 2 * current_channel, stable_value);
    }
    
    print("");
    print("Calibration summary:");
    for (current_channel = 0; current_channel < 8; current_channel++) {
    	uint16_t cal_1A, cal_2_5A, cal_10V;
    	
    	(void)IEE1_GetWord(EE_CAL_BASE_1A + 2 * current_channel, &cal_1A);
    	(void)IEE1_GetWord(EE_CAL_BASE_2_5A + 2 * current_channel, &cal_2_5A);
    	(void)IEE1_GetWord(EE_CAL_BASE_10V + 2 * current_channel, &cal_10V);
    	print("%d: 1A=%d 2.5A=%d 10V=%d", current_channel, cal_1A, cal_2_5A, cal_10V);
    }
    select_channel(0xff);
    pt_end(pt);
}
