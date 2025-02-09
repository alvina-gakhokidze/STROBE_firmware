#include <configs/SYSTEM_config.h>

# define WEIGHTED_SUM_FILTER_DEPTH 10000 // minimum depth is 9000 values sampled at 100Hz, but 

#define WSF_PERIOD_MS 10 // 10 ms for frequency of 100Hz


// we can have a SPECIAL task that updates the ledFlyCount 
// and  THAT one is blocked with a semaphore

// what is being sampled at 100Hz?
// the capacitance value is being sampled at 100hz
// the capacitance value has to be sampled and compared to past values before a signal is sent to the LED to turn it on.

// we also have to keep in mind that there's no sense in changing period_ms or power, until we've tested for a FEW bites at least

// period of perturbation signal: 50 seconds...
// this means that for 50 seconds we want to keep a constant value before we change it again.

// so every 100hz the FPGA checks if the number of bites has changed
// and that's what our queue depends on...

// WE could check faster than that, but it doesn't really make sense, because then we'd kind of be OVER sampling.
// the value physically cannot change until the FPGA tells us, and that takes at MINIMUM 10ms

// so we know we have a volatile bool that updates (ledFlyCount)

// we can start ANOTHER timer, that constantly updates the queue with the value of that variable. Every value of the queue is 10ms apart

// don't want to use hardware timers for this i think, because it's not EXTREMELY time critical like the LED flashing is. 

// so we can use RTOS software timers
