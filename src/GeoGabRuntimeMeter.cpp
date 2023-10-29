/**
 * @file RuntimeMeter.cpp
 * @author Gabriel Sieben (gsieben@geogab.net)
 * @brief Runtime... Testes on ESP8266
 * 
 * @version 1.0.6
 * @date 2023-10-29
 * 
 * @copyright Copyright (c) 2021
 * 
 * @licenze MIT license (see LICENSE file)
 * 
 */
#include <GeoGabRuntimeMeter.h>

/**
 * @brief Runtime Meter: Measures the time intervals between markers.
 * 
 */
RuntimeMeter::RuntimeMeter () :  mpslots (5), NoOfSlots (5), Mode (RT_MEASURE_CYCLES) {Init();} 
/**
 * @brief Runtime Meter: Measures the time intervals between markers.
 * 
 * @param size_ Set the Number of measurepoints you whant to measure. 
 */
RuntimeMeter::RuntimeMeter (uint8_t size_)  : mpslots (size_), NoOfSlots (size_), Mode (RT_MEASURE_CYCLES) {Init();} 
/**
 * @brief Runtime Meter: Measures the time intervals between markers.
 * 
 * @param size_ Set the Number of measurepoints you whant to measure. 
 * @param Mode_ Select a measure mode: In Ticks: RT_MEASURE_CYCLES, In Micros Secs: RT_MEASURE_MICROS, In Milli Secs: RT_MEASURE_MILLIS
 */
RuntimeMeter::RuntimeMeter (uint8_t size_, uint8_t Mode_)  :  mpslots (size_), NoOfSlots (size_), Mode (Mode_) {Init();} 

void RuntimeMeter::Init(){
    overall.LastPrintMillis=millis();                // Initial Time Shot
}

/**
 * @brief Adds a measuring point | On the first call it will inizialize 
 * 
 * @param String Name of the Measuring point (Optional)
 * @return uint32_t Runtime of the last run, 0=Error (except on the first measurement point)
 */
uint32_t RuntimeMeter::Add(const String &name) {

    if (loop.Finalized) {
        FlushData();                                // Flush old Data on a new round
    } 

    if (loop.SlotIndex==NoOfSlots) {
        loop.Error+=RT_ERROR_NO_SLOTS_FREE;         // Add Error Flag
        return 0;
    }

    mpslots[loop.SlotIndex].Name=name;              // Set the name

    if (!loop.PrevStamp) {   
        loop.SlotIndex++;                           // Rise Slot Index
        GetStamp(loop.PrevStamp);                   // Initial Measurement
        return 0;
    } 

    GetStamp(loop.ActStamp);                        // Store actual timestamp
    CalcRuntime(mpslots[loop.SlotIndex-1].Runtime); // Calculate the runtime
    loop.SlotIndex++;                               // Rise Slot Index
    loop.PrevStamp=loop.ActStamp;                   // Save the stamp to previous

    return mpslots[loop.SlotIndex-2].Runtime; 
}

/**
 * @brief Pauses the measurement until the next add.  
 * 
 * @return uint32_t: Runtime of the last measurement
 */
uint32_t RuntimeMeter::Pause() {
    if (!loop.PrevStamp) {                            // If someone placed pause at the beginning or one pause after another -> do nothing
        return 0;
    } 

    GetStamp(loop.ActStamp);
    CalcRuntime(mpslots[loop.SlotIndex-1].Runtime);   // Caculate runtime (writes in actual slot using acttamp and revtamp)
    loop.PrevStamp=0;                                 // Forces next add to start or ignore pause if it is called again

    return mpslots[loop.SlotIndex-1].Runtime;
}


/**
 * @brief 
 * 
 * @param kernel Optional: If 'true' (default), the kernel runtime is calculated. If 'false', then at least a delay(0) after the finalzie until loopend is mandatory.
 * @return The runtime of the loop. 0=error. 
 */
bool RuntimeMeter::Finalize(bool kernel) {
    overall.LoopCounter++;                               // Just count the laps. Overflows at some point. But this does not matter. Serves only the development.
    if(loop.SlotIndex==0) {
        // No measurement no result. What a surprise ;-)
        loop.Error+=RT_ERROR_NO_DATA_CAPTURED;
        return  loop.Error;
    }

    /* Calculate the last runtime (if there was no pause before) */
    if(!loop.PrevStamp==0) {                             // true: Es muss noch eine Runtime berechnet werden
        GetStamp(loop.ActStamp);
        CalcRuntime(mpslots[loop.SlotIndex-1].Runtime);   // Caculate runtime (writes in actual slot using acttamp and revtamp)
    }


    /* Calculate the Kernel Runtime */
    if (kernel) {
        GetStamp(loop.PrevStamp);
        delay(0);                                       // This is where the magic is. delay(0) returns immediately after the kernel tasks are done.
        GetStamp(loop.ActStamp);
        CalcRuntime(overall.kernel.Runtime);            // Caculate runtime (using loop.ActStamp, loop.PrevStamp)
    }


    /* Overall Runtime */
    for (uint8_t i=0;i<loop.SlotIndex;i++) {            // ! No rollover check: maybe in the future i will do so error handling...
        loop.Runtime+= mpslots[i].Runtime; 
    }
    if (kernel) {
        loop.Runtime+=overall.kernel.Runtime;           // Add Kernel Runtime
    }


    /* Percentage multiplied by 10*/
    for (uint8_t i=0;i<loop.SlotIndex;i++) {
        mpslots[i].Percentage=(mpslots[i].Runtime*100*10/loop.Runtime);   // percentage=part/all*100
    }
    if (kernel) {
        overall.kernel.Percentage=(overall.kernel.Runtime*100*10/loop.Runtime);     // Calculate Kernel Part
    }

    /* Loop Frequence in Herz (1/sec) multiplied by 10  */
    switch ( Mode ){
        case RT_MEASURE_CYCLES :
        loop.Frequence=F_CPU/loop.Runtime;               // CPU Clock = Ticks per seccond e.g. [80 HHz] = 80.000.000 Ticks per Second
        break;
        case RT_MEASURE_MICROS :
        loop.Frequence=1000000*10/loop.Runtime;          // 1 sec = 1.000.000 micro seconds 
        break;
        case RT_MEASURE_MILLIS:
        loop.Frequence=1000*10/loop.Runtime;             // 1 sec = 1.000 milli seconds 
        break;
        default:
        loop.Error+=RT_ERROR_UNKNOWN_MODE;
    }
    loop.Finalized=true;
    return 0;
}


/**
 * @brief Outputs the results on the serial port for development purposes.
 * 
 * @param every Specifies how long (default 1000 ms=1 second) to wait until the next output. The specification is made in milli seconds. (Same as delay())
 */
void RuntimeMeter::Print(const uint16_t &every) {

    if (millis()<overall.LastPrintMillis+every) return;       // Der Rollover wird ignoriert. It only happens after 1.193 hours anyway. That doesn't matter for a development printout. (2^32=4294967296) Milli Secs -> 2^32/1000 Secs -> 2^32/1000/60/80 Hours
    overall.LastPrintMillis=millis(); 

    if (loop.Error) {
        Serial.printf("There is a previous error with the Number: %u\r\n", loop.Error);
        return;
    } 

    if (!loop.Runtime) {
        Serial.println(F("Please perform a consolidation before printing."));
        loop.Error+=RT_ERROR_FINALIZE_BEFORE_PRINT;
        return;
    } 
    
    Serial.println(F("RUNTIME METER SUMMARY:"));
    float nanospt=(float) 1000/overall.cpuf;
    Serial.printf("CPU Frequency: \t\t%u MHz\r\n", overall.cpuf);
    Serial.printf("Nano Seconds/Tick: \t%f\r\n",nanospt);
    Serial.printf("Slots Available:\t%u\r\n", NoOfSlots);
    Serial.printf("Measurements: \t\t%u\r\n", loop.SlotIndex);
    Serial.printf("Measure Mode: \t\t%s\r\n", ModeName[Mode].c_str());
    Serial.printf("Overall Loops: \t\t%u\r\n",overall.LoopCounter);
    Serial.printf("Loop Frequency: \t%u Hz (%u KHz)\r\n", loop.Frequence/10, loop.Frequence/10000);
    Serial.println(F(" ======================================================="));
    Serial.println(F("|Measurement\t|\tRuntime\t\t|\tShare\t|"));
    Serial.println(F("|===============|=======================|===============|"));
    for (uint8_t i=0;i<loop.SlotIndex;i++) {
        if(sizeof(mpslots[i].Name)) {
            //Serial.print(sizeof(mpslots[i].Name));
            Serial.printf("|%s\t\t",mpslots[i].Name.c_str());
        } else {
            Serial.printf("|%u\t\t",i+1);
        }
        Serial.printf("|\t%d %s\t\t|\t%u %%\t|\r\n",mpslots[i].Runtime, ModeUnit[Mode].c_str() ,mpslots[i].Percentage/10);
    }
    if(overall.kernel.Runtime) Serial.printf("|Kernel\t\t|\t%d %s \t\t|\t%u %%\t|\r\n", overall.kernel.Runtime, ModeUnit[Mode].c_str() , overall.kernel.Percentage/10);
    Serial.println(F("|---------------|-----------------------|---------------|"));
    Serial.printf("|Overall\t|\t%u %s\t\t|\t100 %%\t|\r\n",loop.Runtime, ModeUnit[Mode].c_str());
    Serial.println(F(" ======================================================="));
}

/**
* @brief Internal Function used to flush all Data / used by Finalized()
* 
*/
void RuntimeMeter::FlushData() {
    loop.Error=0;        
    loop.PrevStamp=0;    
    loop.ActStamp=0;     
    loop.Runtime=0;      
    loop.Frequence=0;
    loop.SlotIndex=0;   
    loop.Finalized=false;
}

/**
 * @brief Return the Integrator Version
 */
String RuntimeMeter::Version(){
  return GEOGAB_RM_VERSION;
}

/* P R I V A T E  F U C T I O N S */
/**
 * @brief Internal Function used to calculate the Runtime.
 * 
 * @param target Write directly to the target without copying the value (Reference)
 */
void RuntimeMeter::CalcRuntime(uint32_t &target) { 
    /* Calculate the runtime */
    if (loop.PrevStamp>loop.ActStamp) {                             // True if rollover happened | Note: Comparence valid for: [CPU 160 MHz-> Timedifference Max. 13 Seconds] -> See Nerd Knwoledge in readme.
        target = loop.ActStamp + (0xFFFFFFFF - loop.PrevStamp);     // Part1: 0 - ActSamp | Part2: PrevStam - End of Variable 
    } else {
        target = loop.ActStamp - loop.PrevStamp;                    // without overfow (rollover) the calculation is trivial
    }
}

/**
 * @brief Internal Function: Used to get the time based on the user selection
 * 
 * @param var Write directly to the target without copying the value (Reference)
 */
void RuntimeMeter::GetStamp(uint32_t &var) {
    switch ( Mode ){
         case RT_MEASURE_CYCLES :
            var=ESP.getCycleCount();
            break;
         case RT_MEASURE_MICROS :
            var=micros();
            break;
         case RT_MEASURE_MILLIS:
            var=millis();
            break;
         default:
            loop.Error=RT_ERROR_UNKNOWN_MODE;
    }
}
