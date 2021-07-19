/*
 * interrupt_load_test.h
 *
 * Measure the cost of ADC interrupt processing (or anything 
 * else apart from the 1ms timer tick for that matter.)
 */

#ifndef INTERRUPT_LOAD_TEST_H_
#define INTERRUPT_LOAD_TEST_H_

#include <WDOG1.h>
#include <AD1.h>

#include <core/timer.h>

#define APPLET_INIT		interrupt_load_init
#define APPLET_LOOP		interrupt_load_main

static void
interrupt_load_init(void)
{
	static timer_t		second_timer;
	volatile uint32_t	count;
	
	timer_register(second_timer);
	
	// count throughput with interrupts on
	count = 0;
	timer_reset(second_timer, 5000);
	while (!timer_expired(second_timer)) {
		(void)WDog1_Clear();
		count++;
	}
	print("interrupts on: %ld", count);
	
	// drain CAN output
	timer_reset(second_timer, 100);
	while (!timer_expired(second_timer)) {}

	// disable all of the interesting interrupts here
	(void)AD1_Stop();

	// count throughput with interrupts off
	count = 0;
	timer_reset(second_timer, 5000);
	while (!timer_expired(second_timer)) {
		(void)WDog1_Clear();
		count++;
	}
	print("interrupts off: %ld", count);
}

static void
interrupt_load_main(void)
{
}

#endif /* INTERRUPT_LOAD_TEST_H_ */
