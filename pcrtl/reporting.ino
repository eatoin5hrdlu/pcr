/*
 * Messages and debugger output for PCR thermocycler
 */

#define EXT extern
#include "pcrtl.h"

void show_state(unsigned long elapsed)
{
	Serial.print(elapsed/1000);
	Serial.print(" seconds of"); wrtnum(duration);
	if (HEATING)       Serial.print(": Heating is on      ");
	else if (COOLING)  Serial.print(": Cooling is on      ");
	else               Serial.print(": Peltier unit is off ");
	wrtnum(last_temp);
	Serial.print(" -> ");
	wrtnum(target);
	Serial.println("\n");
}

void report(void) {
#ifdef DEBUG
      Serial.print("Position in Schedule: ");
      Serial.print(next);
      Serial.print("Point in Cycle: ");
      Serial.print(cycle);
      Serial.print(" Temp(");
      Serial.print(target);
      Serial.print(")  Duration(");
      Serial.print(duration);
      Serial.println(") seconds");
#endif
}

void report_target_reached(unsigned long elapsed) {
unsigned int seconds = elapsed/1000;
unsigned int minutes = seconds/60;
	seconds = seconds % 60;
#ifdef DEBUG
	Serial.print("                   ");
	Serial.print(minutes);
	Serial.print(":");
	if (seconds < 10) Serial.print("0");
	Serial.print(seconds);
	Serial.println(" to reach temperature");
#endif
}

void report_off(void) {
  if (once)
    {
	once = false;
	Serial.println("End of schedule reached, peltier off.");
    }
}
