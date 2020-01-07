/* scheduler.h
 *
 *  Created on: 10 Apr 2012
 *      Author: wilkinsj
 *
 *
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

// Define the number of scheduled slots available
#define FIXED_SCHEDULED_SLOTS 10
#define FIXED_SCHEDULED_SLOTS_DIV_2 5 /* used for info display - saves a dynamic div! */

#define SCHEDULER_NEXT_FROM_NOW -1 /* start timer from 'now' */
#define SCHEDULER_NEXT_RANDOM -2 /* start time to trigger random up to 'interval' */

// Define some constants for easy referencing
#define SCHEDULER_LEDFLASH 0
#define SCHEDULER_WEBSEND 1
#define SCHEDULER_ONEWIREREQ 2
#define SCHEDULER_3SPARE 3
#define SCHEDULER_4SPARE 4

extern void scheduler_enable(uint8_t);
extern void scheduler_disable(uint8_t);
extern bool scheduler_isenabled(uint8_t);
extern void scheduler_setup();
extern void scheduler_setup(uint8_t, long, long, void (*cbfunction)(), uint8_t);
extern void scheduler_loop();
extern void scheduler_run();
extern void scheduler_stop();

/*
Bit definition constants for fScheduler_0:
 0 - is_enabled -- if 1, this event is 'enabled'; if 0, skip & ignore event
 1 - one_shot   -- if 1, set is_enabled to 0 when triggered (leading to a single shot event)
 2 - interval -- if 1, next event will be scheduled based on CURRENT ACTUAL millis not next_event + interval
 3 -
 4
 5
 6
 7
*/

#define fSCHED0_allzero 0
#define fSCHED0_enabled 1
#define fSCHED0_oneshot 2
#define fSCHED0_interval 4 /* if set, next event should be based on original next event, not actual */
#define fSCHED0_3spare 8
#define fSCHED0_4spare 16
#define fSCHED0_5spare 32
#define fSCHED0_6spare 64
#define fSCHED0_7spare 128


#endif /* SCHEDULER_H_ */
