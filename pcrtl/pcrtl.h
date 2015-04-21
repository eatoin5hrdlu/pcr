/*
 * Peltier junction PCR controller definitions
 */

#define YELLOW 8
#define BLOCK 0
#define TUBE  1

// PELTIER CONTROL PINS
#define P1    4
#define P2    3

#define HEAT      0x10
#define COOL      0x20
#define HEATING   (state & HEAT)
#define COOLING   (state & COOL)
#define OFF       ((state & (HEAT|COOL))==0)

EXT boolean  running;
EXT boolean  target_reached;
EXT boolean  once;       /* Flag to avoid duplicate message */
EXT double target;    /* Current target temperature */
EXT double last_temp; /* Keep track of previous temperature */
EXT int duration;     /* Hold time after target is reached */
EXT unsigned long last_state_change; /* Time of last change */
EXT unsigned long elapsed;           /* Time since target temp was reached */
EXT byte schedule[100];
EXT int state;

/* reporting.c */
//void show_state(unsigned long elapsed);
//void report(void);
//void report_target_reached(unsigned long elapsed);
//void report_off(void);


