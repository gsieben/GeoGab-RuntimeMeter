/**
 * @file RuntimeMeterExample.cpp
 * @author Gabriel Sieben (gsieben@geogab.net)
 * @brief A simple example with a serial output of the result
 * @version 0.1
 * @date 2021-02-14
 * 
 * @copyright Copyright (c) 2021
 * 
 * @licenze MIT license (see LICENSE file)
 */
#include "SerialPrintExample.h"


  //RuntimeMeter rtmeter(3,RT_MEASURE_MICROS);          // 3 Slots but measure in micro seconds instead of cpu ticks (RT_MEASURE_TICKS) which is default. This is less accurate but longer time intervals can be measured.
  RuntimeMeter rtmeter(5);

/****** Setup ******/
void setup() {
  Serial.begin(115200);
  Serial.println("\r\n### S T A R T ###");
}


/****** Main Loop ******/
void loop() {
rtmeter.Add("Code A");          // Measurement Point 1 (Start)
  uint32_t b;
    for (uint32_t a=0;a<1000;a++) {
      b=acos(a);
    } 


rtmeter.Add("Code B");          // Measurement Point 2 (caculate runtime between Point1 and Point2)
    for (uint32_t a=0;a<100;a++) {
      b=acos(a);
    } 

rtmeter.Add("Code C");          // Measurement Point 2 (caculate runtime between Point1 and Point2)
    for (uint32_t a=0;a<100;a++) {
      b=acos(a);
    } 

rtmeter.Finalize();           // Last Measure Point and finalize calculations. The final() is mandatory!!! 

  /** Note:
   * 'Finalize()' also calculates the kernel time. These include WiFi, Flash, etc. In the zone of the time measurement a delay makes little sense. From here one 
   * can use a 'delay()' if necessary. Considered however, that each 'delay()' affects the measurement of the kernel further. Because within a 'delay()' the kernel 
   * tasks are processed. 
   */

  /** Note: 
   * As you will see, the runtimes are not always 100% equal. But the precession of the values should satisfy the requirements of an estimation. The values flucuate 
   * slightly, because system accesses can have different runtimes. This cannot be influenced.   
   */
 
  /** Note:
   * From 'Finalize()' on, all determined data can be read directly from the public variables for this purpose. The next 'Add()' destroys the pre-arranged data.
   *
   * There are the following groups of variables: rtmeter.loop,rtmeter.mpslots, rtmeter.overall
   * The access is direct: rtmeter.overall.kernel.Runtime, rtmeter.mpslots[1].Name, rtmeter.mpslots[1].Runtime
   */

  rtmeter.Print(2000);        // Optionally you can output the data on the serial port. By default, the data is output every second (1000 ms).

  rtmeter.loop.Runtime=b;     // This seems to not make any sense, but it serves a purpose in this example code. The purpose of this line is only to make the optimizer think the above for loops are not meaningless. This variable will be deleted in the next step anyway. 
}

