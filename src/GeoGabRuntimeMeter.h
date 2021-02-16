/**
 * @file RuntimeMeter.h
 * @author Gabriel Sieben (gsieben@geogab.com)
 * @brief 
 * @version 1.0.1
 * @date 2021-02-11
 * 
 * @copyright Copyright (c) 2021
 * 
 * @licenze MIT license (see LICENSE file)
 */

#ifndef GeoGabRuntimeMeter_h
#define GeoGabRuntimeMeter_h

#include <Arduino.h>

#define XSTR(s) STR(s)
#define STR(...) #__VA_ARGS__

#define GEOGAB_RM_VERSION "1.0.1"

enum  {
  RT_ERROR_NONE=0,                        // 0000 0000: Perfect :-)           
  RT_ERROR_NO_SLOTS_FREE=1,               // 0000 0001: Reverse more slots on init    
  RT_ERROR_NO_DATA_CAPTURED=2,            // 0000 0010: Set at least one capture points
  RT_ERROR_FINALIZE_BEFORE_PRINT=4,       // 0000 0100: Print does not make sence before consolidation 
  RT_ERROR_UNKNOWN_MODE=8,                // 0000 1000: Der Erfassungsmodus ist unbekannt. Bitte wähle einen folgender Werte: RT_MEASURE_TICKS, RT_MEASURE_MICROS, RT_MEASURE_MILLIS, RT_MEASURE_SECS
};

enum {
  RT_MEASURE_TICKS,
  RT_MEASURE_MICROS,
  RT_MEASURE_MILLIS
};


class RuntimeMeter {
  public:
    /* Constructors */
    RuntimeMeter ();
    RuntimeMeter (uint8_t size_);  // Pups pups pups
    RuntimeMeter (uint8_t size_, uint8_t Mode_);    // Dadada

    
    /* Prototypes */
    uint32_t Add(const String &name="");
    uint32_t Pause();  
    bool Finalize(bool kernel=true);
    void Print(const uint16_t &every=1000);   
    void FlushData();    
    String Version();

    /* Structures */
    struct measurePoint_t {
      String Name="";                   // Slot Name
      uint32_t Runtime=0;               // Runtime
      uint16_t Percentage=0;            // Percentage multiplied by 10 (Divide the number by 10 and you get the first decimal place)
    };

    struct overall_t {
      uint32_t LoopCounter=0;           // Counts the number of loops since program start
      uint32_t LastPrintMillis=0;       // Time index at last print
      measurePoint_t kernel;            // Messpunkt für den Kernel
      uint16_t cpuf=F_CPU/1000000;      // CPU Frequency
    };

    struct loop_t {
      uint8_t Error=0;                  // Number of the last error
      uint32_t PrevStamp=0;             // Previous Time Stamp
      uint32_t ActStamp=0;              // Actual Time Stamp
      uint32_t Runtime=0;               // Runtime in ticks
      uint32_t Frequence=0;             // Frequence Herz multiplied by 10 (Divide the number by 10 and you get the first decimal place) 
      uint16_t SlotIndex=0;             // Index of the actual slot
      bool Finalized=false;             // Will be set to true once the loop is finalzed
    };

    /* Variables */
    overall_t overall;
    loop_t loop;
    std::vector<measurePoint_t> mpslots;  // (Init 1) Vector of the current measured values

  private:
    /* Internal Prototypes */
    void CalcRuntime(uint32_t &target);
    void GetStamp(uint32_t &var);
    void Init();

      /* Internal Variables init be object creation */
    uint16_t NoOfSlots;   // (Init 2) Number of measuring points available (Default: 5)
    uint8_t Mode;         // (Init 3) Measuremnt Type (Default: RT_MEASURE_TICK=0)

    /* Internal Variables */
    uint32_t ActStamp=0;
    uint32_t PrevStamp=0;
    uint32_t OverallLoopRuntime=0;

    String ModeName[3] = {"CPU Ticks", "Micro Seconds", "Milli Seconds"}; 
    String ModeUnit[3] = {"", "µs", "ms"};
};

#endif
