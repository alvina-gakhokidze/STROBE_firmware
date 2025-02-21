#include <configs/SYSTEM_config.h>

#define MOVING_AVERAGE_FILTER_DEPTH 5000 // minimum depth is 5000 values sampled at 100Hz, but 

#define MAF_PERIOD_MS 10 // 10 ms for frequency of 100Hz


/*

FIR filter designed with
http://t-filter.appspot.com

sampling frequency: 100 Hz

* 0 Hz - 44 Hz
  gain = 0
  desired attenuation = -60 dB
  actual attenuation = -60.386564281136124 dB

* 48 Hz - 50 Hz
  gain = 1
  desired ripple = 0.05 dB
  actual ripple = 0.035223931645588485 dB

*/

#define POWER_FILTER_TAP_NUM 75

static double power_filter_taps[POWER_FILTER_TAP_NUM] = {
  -0.0004024553723618049,
  0.000998159620796862,
  -0.0008582413863015407,
  0.0014254648102468577,
  -0.0016470316450717592,
  0.0019982933952370392,
  -0.0021739290136555266,
  0.002253607496387267,
  -0.002121650708061577,
  0.0017656667960417227,
  -0.001135700922956621,
  0.00022365215684193215,
  0.0009692357414227774,
  -0.002408027160005508,
  0.004030497498903243,
  -0.005742528720384516,
  0.007422336377112879,
  -0.008921553225033942,
  0.010074546165156397,
  -0.010706604483849785,
  0.010643630369145105,
  -0.009728444243791457,
  0.00782847636528552,
  -0.004851826201764223,
  0.0007555815905286718,
  0.004445048183553621,
  -0.01067199251302729,
  0.01778293955252413,
  -0.025575423201809375,
  0.03379370896611759,
  -0.042139943714663265,
  0.050289672902958715,
  -0.0579085016943071,
  0.06467030163498588,
  -0.07027812059010304,
  0.07447848516849503,
  -0.07707910803656448,
  0.07796002703790872,
  -0.07707910803656448,
  0.07447848516849503,
  -0.07027812059010304,
  0.06467030163498588,
  -0.0579085016943071,
  0.050289672902958715,
  -0.042139943714663265,
  0.03379370896611759,
  -0.025575423201809375,
  0.01778293955252413,
  -0.01067199251302729,
  0.004445048183553621,
  0.0007555815905286718,
  -0.004851826201764223,
  0.00782847636528552,
  -0.009728444243791457,
  0.010643630369145105,
  -0.010706604483849785,
  0.010074546165156397,
  -0.008921553225033942,
  0.007422336377112879,
  -0.005742528720384516,
  0.004030497498903243,
  -0.002408027160005508,
  0.0009692357414227774,
  0.00022365215684193215,
  -0.001135700922956621,
  0.0017656667960417227,
  -0.002121650708061577,
  0.002253607496387267,
  -0.0021739290136555266,
  0.0019982933952370392,
  -0.0016470316450717592,
  0.0014254648102468577,
  -0.0008582413863015407,
  0.000998159620796862,
  -0.0004024553723618049
};



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
