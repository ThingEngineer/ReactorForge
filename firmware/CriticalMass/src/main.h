/*
 * main.h
 *
 * Created: 12/16/2017
 * Author: Josh Campbell
 */


#ifndef MAIN_H
#define MAIN_H

#if (__GNUC__ * 100 + __GNUC_MINOR__) < 303
#error "AVR-GCC 3.3 or later is required, update AVR-GCC compiler!"
#endif

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sfr_defs.h>
#include <avr/eeprom.h>
#include <string.h>
#include <stdio.h>

#include <lcd.h>
#include <ds18x20.h>
#include <onewire.h>


#define USART_BAUDRATE 57600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
#define NEWLINESTR "\r"

#define MAXSENSORS 4


/****************************************************************************
* Function prototypes
****************************************************************************/
void init(void);

void reset_ss(void);
void start_psc(void);
void stop_psc(uint8_t);
void incr_freq(uint8_t);
void decr_freq(void);
void set_frequency(void);
uint16_t get_adc(uint8_t);

static void uart_print8(uint8_t val);
static void uart_print16(uint16_t val);
static int uart_putchar(char c, FILE *stream);
static int uart_getchar(FILE *stream);

static uint8_t search_sensors(void);
static uint8_t gSensorIDs[MAXSENSORS][OW_ROMCODE_SIZE];

int read_buttons(void);

// int button0_is_pressed(void);
// int button1_is_pressed(void);
// int button2_is_pressed(void);
// int button3_is_pressed(void);
// int button4_is_pressed(void);


/****************************************************************************
* Macros
****************************************************************************/
#define bit_get(p,m) ((p) & (m))
#define bit_set(p,m) ((p) |= (m))
#define bit_clear(p,m) ((p) &= ~(m))
#define bit_flip(p,m) ((p) ^= (m))
#define bit_write(c,p,m) (c ? bit_set(p,m) : bit_clear(p,m))
#define BIT(x)	(0x01 << (x))									// Converts a bit number into a byte value.
#define LONGBIT(x) ((unsigned long)0x00000001 << (x))
#define BIT_VAL(x,y) (((x) >> (y)) & 1)

static FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);	// SDOUT buffer
static FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);		// SDIN buffer
//FILE uart_io FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);	// One buffer which works for both SDIN and SDOUT.


/****************************************************************************
* Port aliases
****************************************************************************/
// Start/Stop switch
#define SS_SW_PORT PORTD
#define SS_SW_PIN PIND
#define SS_SW_BIT 6

// Encoder output A
#define ENC_PORT PORTD
#define ENC_PIN PIND
#define ENC_BIT 1

// Indicator output
#define INDICATOR_PORT PORTC
#define INDICATOR_BIT 3

// Mains Contactor Coil output
#define CONTACTOR_PORT PORTD
#define CONTACTOR_BIT 7

// Pump switch output
#define PUMP_PORT PORTC
#define PUMP_BIT 2

// Buzzer
#define BUZZER_PORT PORTD
#define BUZZER_PIN PIND
#define BUZZER_BIT 0

#define DEBOUNCE_TIME 25			// Time to wait while "de-bouncing" button
#define LOCK_INPUT_TIME 200			// Time to wait after a button press


/****************************************************************************
* Variable and constant declarations
****************************************************************************/
typedef struct
{
	unsigned char bit0:1;
	unsigned char bit1:1;
	unsigned char bit2:1;
	unsigned char bit3:1;
	unsigned char bit4:1;
	unsigned char bit5:1;
	unsigned char bit6:1;
	unsigned char bit7:1;
}io_reg;

static volatile uint16_t mains_current;		// Mains current ADC value
//static volatile uint16_t tank_voltage;		// Tank voltage ADC value
static volatile uint16_t inverter_current;	// Inverter current ADC value
uint16_t mains_current_avg;
uint16_t inverter_current_avg;

uint16_t inverter_current_max;		// Maximum inverter current

// Buck converter variables
// uint16_t dc_ontime;				// Base PWM frequency
// uint8_t dc_deadtime;				// PWM deadtime
// uint16_t dc_max_duty_cycle;		// Maximum PWM duty cycle (max DC output)
// uint16_t dc_min_duty_cycle;		// Minimum PWM duty cycle (min DC output)
// uint16_t dc_duty_cycle_on;		// The portion of dc_ontime that should be on per cycle
// uint16_t dc_duty_cycle_off;		// The portion of dc_ontime that should be off per cycle

uint16_t setpoint;				// Represents the desired PSC value when the inverter voltage and current are at the proper phase offset with the system just above resonance
uint16_t ontime;				// Ontime value for output compare registers, correlates directly to frequency, lower ontime = higher PWM frequency
uint16_t ontime_ceiling;		// Maximum possible ontime
uint16_t ontime_floor;			// Minimum possible ontime
uint8_t deadtime;				// Deadtime for output compare registers
uint8_t frq_range;				// Minimum and maximum PWM ontime/frequency delta after a successfully lock
uint16_t scan_period;			// Number of control loop passes to execute before giving up without a successful lock
int16_t delta_max;				// Maximum phase delta that can be encountered before system halt
int16_t delta_min;				// Minimum phase delta that can be encountered before system halt
uint8_t upper_limit_hits;		// Maximum consecutive upper soft limit hits before system halt
uint8_t lower_limit_hits;		// Maximum consecutive lower soft limit hits before system halt

uint8_t control_loop;
uint8_t control_loop2;
uint16_t lock_value;			// Ontime value upon successfully lock within deadband of setpoint, the system will no longer deviate from this point +-Frq_range
uint8_t upper_limit_cnt;		// Upper limit consecutive hit counter
uint8_t lower_limit_cnt;		// Lower limit consecutive hit counter
#define lock_flag ((volatile io_reg*)_SFR_MEM_ADDR(GPIOR3))->bit0		// 0 = scanning ; 1 = locked on setpoint
#define running_flag ((volatile io_reg*)_SFR_MEM_ADDR(GPIOR3))->bit1	// Set when unit is running (not in standby)
#define first_pass ((volatile io_reg*)_SFR_MEM_ADDR(GPIOR3))->bit2		// First pass flag indicates first pass through control loop
//#define fault_flag ((volatile io_reg*)_SFR_MEM_ADDR(GPIOR3))->bit3		// Interrupt set flag on IGBT hardware fault
#define ac_peak ((volatile io_reg*)_SFR_MEM_ADDR(GPIOR3))->bit3		// AC mains peak detected

int16_t phase_current;			// PSC value representing the phase offset between inverter E & I (captured on the falling edge of the inverter current (I) waveform)
int16_t phase_previous;			// Previous PSC value (phase offset)
int16_t phase_delta;			// Change in phase offset from previous to current
int16_t phase_offset;			// Phase setpoint with deadband offset
int16_t phase_deadband;			// PSC phase offset deadband to prevent oscillation and excessive hunting near resonance
int16_t phase_adjustment;		// Phase adjustment value to detune setpoint
int16_t phase_average;
uint8_t filter_level;

int16_t error_current;			// Current error
int16_t error_previous;			// Previous error
int16_t error_delta;			// Change in error from previous to current

//uint8_t error_status;			// Error status, indicates the error type that caused a system halt
#define igbt_fault 1			// Hardware IGBT fault
#define phase_delta_max 2		// Error greater than phase delta max encountered between two consecutive control loop iterations
#define phase_delta_min 3		// Error less than phase delta min encountered between two consecutive control loop iterations
#define scan_timeout 4			// No lock within the setpoint deadband was established within the lock period
#define ceiling_reached 5		// Ontime ceiling reached (maximum PSC frequency)
#define floor_reached 6			// Ontime floor reached (minimum PSC frequency)
#define upper_soft_limit 7		// Upper soft limit was hit consecutively the number of times set in Upper_limit_hits
#define lower_soft_limit 8		// Lower soft limit was hit consecutively the number of times set in Lower_limit_hits
#define inverter_current_spike 9// Inverter current spiked above maximum allowed

#define start_code 0xFEB0		// Serial packet start code
uint16_t print_buf[10];			// UART buffer for processing data

uint8_t OW_nSensors;
int16_t deciCelsius;
uint8_t OW_error;

uint8_t lcd_key;
uint16_t adc_key_in;
uint16_t key_lockout_time;
#define btnNONE		0
#define btn1		1
#define btn2		2
#define btn3		3
#define btn4		4
#define btn5		5

int16_t enc;
int16_t enc_prev;

#define bit_temp ((volatile io_reg*)_SFR_MEM_ADDR(GPIOR3))->bit7	// Temporary bit
uint8_t byte_temp;				// Temporary byte variable
uint16_t word_temp;				// Temporary word
uint8_t i;

int16_t pid_value;				//
int16_t pid_error;				//
int16_t pid_previous_error;		//
int16_t pTerm;					//
int16_t iTerm;					//
int16_t dTerm;					//
int16_t Kp;						//
int16_t Ki;						//
int16_t Kd;						//


#endif /* MAIN_H */
