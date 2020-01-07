/*
 * scheduler.cpp
 *
 * - A very simple scheduler module
 *
 * This module provides a simplistic scheduler function for a limited number of
 * scheduled tasks.  There are a fixed (at compile time) set of scheduler
 * 'slots', each of which is defined in an element of array 'scheduler' with a
 * sched_t structure.
 *
 * The struct contains
 *   time of next event,
 *   interval between events,
 *   function to call on event trigger,
 *   flag field
 *
 *
 *  Created on: 02 Dec 2017
 *      Author: wilkinsj
 *      Based on scheduler.cpp by same, 10 Apr 2012
 */

#include <Arduino.h>
#include "scheduler.h"

// Manually set 10 scheduler slots
# define FIXED_SCHEDULED_SLOTS 10

/* The scheduler data structure has been kept in the .cpp file instead of the .h deliberataly,
 * so that modules which use the scheduler (include the .h) don't have to have - and dont need - knowledge
 * of the underlying struct
 */
struct sched_t {
	uint8_t fScheduler_0;			// flag field for this entry
	unsigned long next_event;	// millis when next event should occur (or immediately after)
	unsigned long interval;		// amount to add to trigger point(*) to repeat
	void (*callback)();			// event function to call
};


// Create a fixed-size array to hold schedule definitions (FIXED_SCHEDULED_SLOTS is defined in the .h)
sched_t scheduler[FIXED_SCHEDULED_SLOTS];
#define DEFAULT_WEBSUBMIT_INTERVAL 5000
#define DEFAULT_LEDFLASH_INTERVAL 2000
#define DEFAULT_RETRVTEMP_INTERVAL 750 /* should be 750 */
#define DEFAULT_SENDHBEAT_INTERVAL 10000 /* 10 seconds */

#define DEFAULT_10SECOND_INTERVAL 10000
#define DEFAULT_3SECOND_INTERVAL 3000
#define DEFAULT_2SECOND_INTERVAL 2000
#define DEFAULT_1SECOND_INTERVAL 1000

// Define some extern functions here that are required for the slots below
// (this is a messy way to do it... should really #include the correct .h files...)
extern void flash_led();
extern void cb_request_temp_retrieval();
extern void cb_temp_requests_ready();
extern void cb_1mwd();
#ifdef display
extern void cb_display_refresh();
#endif
extern void radio_loop();

/*
 * schedule_enable(uint8_t slot)
 * - enable slot, setting interval and whether this a one-shot or not
 *
 * note that this requires the slot is already defined, at least in terms of callback
 */
void scheduler_enable(uint8_t slot){
	// set time for next event
	scheduler[slot].next_event=scheduler[slot].interval + millis();
	// enable the scheduler entry
	scheduler[slot].fScheduler_0= ( scheduler[slot].fScheduler_0 | fSCHED0_enabled );
}

/*
 * schedule_disable(uint8_t slot)
 * - disable slot, but leaving all other settings unchanged
 *
 */
void scheduler_disable(uint8_t slot){
	// disable the scheduler entry
	scheduler[slot].fScheduler_0= ( scheduler[slot].fScheduler_0 & ~fSCHED0_enabled );
}

/*
 * scheduler_isenabled(uint8_t slot)
 * - query whether the specified slot is currently enabled
 *
 */
bool scheduler_isenabled(uint8_t i){
	if(scheduler[i].fScheduler_0 & fSCHED0_enabled){
		return true;
	}else{
		return false;
	}
}

void scheduler_setup(uint8_t scheduler_id, long interval, long next, void (callback)(), uint8_t flags){
	unsigned long millis_now=millis();

	scheduler[scheduler_id].fScheduler_0=flags;
	scheduler[scheduler_id].interval=interval;
	if(next==SCHEDULER_NEXT_FROM_NOW){
		next = millis_now+interval;
	}
//	if(next==SCHEDULER_NEXT_RANDOM){
//		millis_now+random(interval);
//	}
	scheduler[scheduler_id].next_event=next;
	scheduler[scheduler_id].callback=callback;

}
/*
 * scheduler_setup()
 * - initialise the scheduler
 *   - set each slot
 * - this should be called from setup()
 *
 * - this code, whilst repetetive and static/fixed, is designed to be simple & efficient
 *   in terms of code & memory usage.  Definately further improvements can be made. Though
 *   am wondering about adding a scheduler_create() call to init a new slot from an external
 *   module of code (making this more oo)
 */
void scheduler_setup(){
	unsigned long millis_now;
	millis_now=millis();

	// Scheduler 0 : LED blink
	//scheduler[0].fScheduler_0= ( fSCHED0_enabled | fSCHED0_interval ) ;
	//scheduler[1].fScheduler_0= ( fSCHED0_allzero);
	//scheduler[0].interval=DEFAULT_LEDFLASH_INTERVAL;
	//scheduler[0].next_event=millis_now+DEFAULT_LEDFLASH_INTERVAL;
	//scheduler[0].callback=&flash_led;

	// Scheduler 1 : Read temperatures
	scheduler[1].fScheduler_0= ( fSCHED0_enabled | fSCHED0_interval ) ;
	//scheduler[1].fScheduler_0= ( fSCHED0_allzero);
	scheduler[1].interval=DEFAULT_1SECOND_INTERVAL;
	scheduler[1].next_event=millis_now+DEFAULT_1SECOND_INTERVAL;
	scheduler[1].callback=&cb_request_temp_retrieval;

	// Scheduler 2 : Retrieve temperature reading
	scheduler[2].fScheduler_0= ( fSCHED0_oneshot ) ; // disabled, actual from set-time, one-shot
	scheduler[2].interval=DEFAULT_RETRVTEMP_INTERVAL;
	scheduler[2].next_event=0;
	scheduler[2].callback=&cb_temp_requests_ready;

    // Scheduler 3 : Send heartbeat(s)
    //scheduler[3].fScheduler_0= ( fSCHED0_enabled | fSCHED0_interval );
    scheduler[3].fScheduler_0= ( fSCHED0_allzero);
    scheduler[3].interval=DEFAULT_SENDHBEAT_INTERVAL;
    scheduler[3].next_event=0;
    //scheduler[3].callback=&send_hbeat;

    // Scheduler 4 : Scan hardware / Update environment
    //scheduler[4].fScheduler_0= ( fSCHED0_enabled | fSCHED0_interval );
    scheduler[4].fScheduler_0= ( fSCHED0_allzero);
    scheduler[4].interval=120000;
    scheduler[4].next_event=0;
    //scheduler[4].callback=&xap_send_hbeat;

    // Scheduler 5 : Spare
    scheduler[5].fScheduler_0= ( fSCHED0_allzero);

    // Scheduler 6 : Spare
    scheduler[6].fScheduler_0= ( fSCHED0_allzero);

    // Scheduler 7 : RF Send Ping
    scheduler[7].fScheduler_0= ( fSCHED0_allzero);
    //scheduler[7].fScheduler_0= ( fSCHED0_enabled | fSCHED0_interval );
    scheduler[7].interval=DEFAULT_1SECOND_INTERVAL;
    scheduler[7].next_event=millis_now+DEFAULT_3SECOND_INTERVAL;
    scheduler[7].callback=&radio_loop;

    // Scheduler 8 : Refresh OLED display
#ifdef display
    //scheduler[8].fScheduler_0= ( fSCHED0_allzero);
    scheduler[8].fScheduler_0= ( fSCHED0_enabled | fSCHED0_interval );
    scheduler[8].interval=DEFAULT_1SECOND_INTERVAL;
    scheduler[8].next_event=millis_now+DEFAULT_1SECOND_INTERVAL;
    scheduler[8].callback=&cb_display_refresh;
#else
    scheduler[8].fScheduler_0= ( fSCHED0_allzero);
#endif

	// Scheduler 9 : Spare
	scheduler[9].fScheduler_0= ( fSCHED0_enabled | fSCHED0_interval );
	//scheduler[9].fScheduler_0= ( fSCHED0_allzero);
	scheduler[9].interval=60000;
	scheduler[9].next_event=0;
	scheduler[9].callback=&cb_1mwd;
}

void scheduler_run(){
    unsigned long millis_now=millis();
    scheduler[7].fScheduler_0= ( fSCHED0_enabled | fSCHED0_interval );
    scheduler[7].next_event=millis_now+DEFAULT_3SECOND_INTERVAL;


}
void scheduler_stop(){
    scheduler[7].fScheduler_0= ( fSCHED0_allzero);
}

/*
 *
 */
void scheduler_loop(){
	unsigned long millis_now;
	millis_now=millis();

	uint8_t i=0;
	do {
		if(scheduler[i].fScheduler_0 & fSCHED0_enabled ){
			if(scheduler[i].next_event <= millis_now){
				// event it due to be triggered
				// check if event is recurring
				if(scheduler[i].fScheduler_0 & fSCHED0_oneshot){
					// is a one-shot - so clear the enable bit
					scheduler[i].fScheduler_0 = scheduler[i].fScheduler_0 & (~ fSCHED0_enabled);
				}else{
					// is recurent
					if(scheduler[i].fScheduler_0 & fSCHED0_interval){
						scheduler[i].next_event+=scheduler[i].interval;
					}else{
						scheduler[i].next_event=millis_now + scheduler[i].interval;
					}
				}

				// now actually make the call...
				scheduler[i].callback();
			}
		}
	} while (++i<FIXED_SCHEDULED_SLOTS);
}
