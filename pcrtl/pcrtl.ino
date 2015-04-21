/*
 * Peltier junction PCR thermocycler     Peter Reintjes
 * 
 * Two digital outputs:	      Two analog inputs:
 *  P1  P2   State         Fixed 'BLOCK' temp sensor
 *   1   1    OFF          Movable 'TUBE' temp sensor
 *   1   0   COOL (+12V)
 *   0   0   HEAT (-12V)
 */

#define DEBUG 1
#ifdef DEBUG
int count;
#endif

#define EXT 
#include "pcrtl.h"
#include <EEPROM.h>

// Specify default temperature sensor
// #define SENSOR TUBE
#define SENSOR BLOCK

/*
 * restore() reloads the current schedule from EEPROM
 * Format:
 * Repeats   N   Temp1  Duration1  Temp2 Duration2 ... TempN DurationN
 *
 * Example Schedule:
 * 1   1  25  10          // Room temperature for ten seconds
 * 1   1  92  30          // 92 degrees for 30 seconds (e.g. Hot Start)
 * 32  3  39  30    57  90     78  40
 * ^ Thirty-two cycles
 *     ^ Three temperature/time pairs
 *          39C for 30 seconds    (Annealing)
 *                     57C for 90 seconds  (Extension)
 *                               78C for 40 seconds (Melting)
 * 1  1  25  10  // Finally, cool down to room temperature
 * 0  // End
 */

void restore(void)
{
int addr = 0;
	byte b = EEPROM.read(addr);
	while(b) {
		schedule[addr++] = b;
		b = EEPROM.read(addr);
	}
	schedule[addr] = b;
#ifdef DEBUG
	Serial.print(addr);
	Serial.println(" items read from EEPROM to schedule");
#endif
}

int readNumber(void)
{
	String is = "";
	while (Serial.available() == 0) delay(100);
	while (Serial.available() > 0)
	{
		int c  = Serial.read();
		if (isdigit(c))	is += (char)c;
		else 		break;
		if (Serial.available() == 0) delay(100);
	}
	if ( is.length() > 0 )
		return(atoi(&is[0]));
	else
		Serial.print("failed to read a number from input");
	return 0;
}

void loadSchedule(void)
{
int  addr = 0;
byte reps = readNumber();
	while(reps) {
		byte datanum = readNumber();
		EEPROM.write(addr++,reps);
		EEPROM.write(addr++,datanum);
		for (int i=0; i < datanum; i++) {
			EEPROM.write(addr++, readNumber());
			EEPROM.write(addr++, readNumber());
		}
		delay(1000);
		reps = readNumber();
	}
	EEPROM.write(addr,reps);
}

void respondToRequest(void)
{
	String is = "";
	while (Serial.available() > 0)  // Read a line of input
	{
		int c  = Serial.read();
		if ( c < 32 ) break;
		is += (char)c;
		if (Serial.available() == 0) // It is possible we're too fast
			delay(100);
	}
	if ( is.length() > 0 )  {   // process the command
		int value = 0;
		if (is.length() > 2)
			value = atoi(&is[2]);
		process(is[0], value);
	}
}

void wrtnum(byte n) {Serial.print(" ");Serial.print(n);}
/*
 * 'd' print the schedule
 * 's' load a complete schedule
 */

void process(char c1, int value)
{
int addr = 0;
int repeat;
int tmp;
byte bval;
	switch(c1) {
		case 'd':
			addr = 0;
			repeat = EEPROM.read(addr++);
			while(repeat) {
				int pairs = EEPROM.read(addr++);
				wrtnum(repeat);
				wrtnum(pairs);
				for (int i=0; i<pairs; i++) {
			  	  wrtnum((int)EEPROM.read(addr++));
			  	  wrtnum((int)EEPROM.read(addr++));
				}
				Serial.println("");
				repeat = EEPROM.read(addr++);
			}
			wrtnum(repeat);
			Serial.println("");
		  break;
		case 's':
			loadSchedule();
		  break;
		case 'l':
			restore();
		  break;
		default :
		  Serial.print("Ignoring line begining with [");
		  Serial.write(c1);
		  Serial.println("]");
	}
}

double temperature(int analog)
{
  double accum = 0;
  for(int i=0; i<10; i++) accum = accum + analogRead(analog);
  return (accum/93.1);
}

int next;    // Schedule index pointer
int repeat;  // Current cycle repeat counter
int datanum; // Number of temperature/duration pairs in current cycle
int cycle;   // Counter for point in cycle

void nextStep(void) {
	if (repeat == 0) {
		repeat = schedule[next++];
		if (repeat == 0) {
			Serial.println("Reached the End of Schedule");
			running = false;
			return;
		}
		datanum = schedule[next++];
		cycle = next;
	} else {
		if (cycle-next < 2*datanum) {
			target = (double) schedule[cycle++];
			Serial.print("Target: "); Serial.print(target);
			target_reached = false;
			duration = schedule[cycle++];
			Serial.print("   Duration: "); Serial.println(duration);
			last_state_change = millis();
		} else {
			repeat--;
			cycle = next;
		}
	}
	return;
}
/*
 * update(sensor):
 *    Read the current temperature and update elapsed time
 *    Heat or cool until target temperature is reached
 *    When target temperature reached:
 *    	Peltier unit off
 *	Start elapsed timer
 *    If elapsed > duration, begin the next step
 */
void update(int sensor)
{
  double t = temperature(sensor);
  unsigned long elapsed = millis() - last_state_change;

#ifdef DEBUG
  if (count % 3000 == 0) { toggle(); show_state(elapsed); }
#endif

	// At the target temperature for specified number of seconds
        // nextStep(): resets target_reached=false and elapsed=0

	if (target_reached && elapsed/1000 > duration) {
		nextStep();
		report();
	}

//The following line is for testing without an active peltier unit
//( After a while, pretend the temperature has been reached )
//	else if (elapsed/1000 > 2*duration) target_reached = true;
//

	if (  ( HEATING && t > (target+1.0) ) // <1C from target
	    ||( COOLING && t < (target+1.0) ) )
	{
		if (!target_reached) {
			target_reached = true; // Begin timing
			report_target_reached(elapsed);
			elapsed = millis() - last_state_change;
			last_state_change = millis();
		}
		poweroff();
	}
	else if (t > target )
	{
		state = COOL;
		digitalWrite(P1,1);digitalWrite(P2,0);
	}
	else if (t < target) // Too cold
	{
		state = HEAT;
		digitalWrite(P1,0);digitalWrite(P2,0);
	}
	else
		poweroff();
}

void poweroff(void)
{
  state = 0;
  digitalWrite(P1,1);
  digitalWrite(P2,1);
}

int alt_sensor(int sensor)
{
	if (sensor == BLOCK) return TUBE;
	if (sensor == TUBE) return BLOCK;
	return sensor;
}

boolean tgl = true;
void toggle(void)
{
	tgl = !tgl;
	digitalWrite(YELLOW,tgl);
}

void setup()
{
  once = true;
  Serial.begin(9600);

  pinMode(P1, OUTPUT);
  pinMode(P2, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  poweroff();            // Peltier junction off

  // 1.1v internal reference:  10mv/deg C -> 1.00V = 100C
  analogReference(INTERNAL);

  next = 0;
  last_state_change = millis();

#ifdef DEBUG
  count = 0;
#endif

  restore();             // Load the schedule from EEPROM
  last_temp      = 25.1; // Assume starting at room temperature
  running        = true;
  target_reached = true;
  repeat = 0;
  duration = 1;
  elapsed = 2000;
  Serial.println("setup");
}


void loop()
{
  last_temp = temperature(SENSOR);
  respondToRequest();              // Handle queries
#ifdef DEBUG
  if (count++ % 10000 == 0)
	{
	 double t2 = temperature(alt_sensor(SENSOR));
	 Serial.print(millis());
	 Serial.print("        ");
	 Serial.print(last_temp);
	 Serial.print("  (");
	 Serial.print(t2);
	 Serial.println(")");
	}
#endif
  if ( running )
	update(SENSOR);
  else
  {
	poweroff();
        if (once)
	{
		once = false;
		Serial.println("End of schedule reached, peltier off.");
	}
  }
}


