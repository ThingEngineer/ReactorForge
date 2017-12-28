/*
 * File:      main.c
 * Title      ReactorForge - CriticalMass
 * Hardware:  ReactorCore AT90PWM316
 * Author:    Josh Campbell
 * Created:   8/24/2013
 * Modified:  12/22/2017
 * Version:   0.0.a1
 * Link:      https://github.com/ThingEngineer/ReactorForge
 * Website:   https://reactorforge.com
 * This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.
 * License:		https://reactorforge.com/licence/
 *
 * DESCRIPTION
 *        The ReactorForge project is an open source hardware platform for
 *        high power induction heating, designed for heavy use and reliable
 *        performance in real-world applications.
 *
 * Coding Standards:
 *        WIP - These standards are not yet fully implemented.
 *        http://ieng9.ucsd.edu/~cs30x/indhill-cstyle.html
 *        https://users.ece.cmu.edu/~eno/coding/CCodingStandard.html
 *

 *
 * THIS CODE AND ASSOCIATED DOCUMENTATION (THE "SOFTWARE") IS PROVIDED "AS IS",
 * WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include <main.h>


/****************************************************************************
* Setup
****************************************************************************/
void setup() {
  //Set port data direction registers and pull ups
	DDRB = 0b00000000;
	DDRD = 0b10000000;
	DDRC = 0b00001100;
	DDRE = 0b00000000;

	// Initial output port states
	bit_clear(INDICATOR_PORT, BIT(INDICATOR_BIT));
	bit_clear(CONTACTOR_PORT, BIT(CONTACTOR_BIT));
	bit_clear(PUMP_PORT, BIT(PUMP_BIT));
	bit_clear(BUZZER_PORT, BIT(BUZZER_BIT));

	// Set pull ups
 	bit_set(SS_SW_PORT, BIT(SS_SW_BIT));
	bit_set(ENC_PORT, BIT(ENC_BIT));
	bit_set(PORTC, BIT(0));		// Rotary encoder
	bit_set(PORTD, BIT(2));		// IGBT Fault
	//bit_set(PORTC, BIT(6));	// Multiplexed buttons

	// USART Setup
	//uart_init( UART_BAUD_SELECT(UART_BAUD_RATE) );
	bit_set(UCSRB, BIT(RXEN) | BIT(TXEN));			// Enable receiver and transmitter
	bit_set(UCSRC, BIT(UCSZ0) | BIT(UCSZ1));		// Use 8-bit character sizes
	UBRRH = (BAUD_PRESCALE >> 8);					// Load upper 8-bits of the baud rate value into the high byte of the UBRR register
	UBRRL = BAUD_PRESCALE;							// Load lower 8-bits of the baud rate value into the low byte of the UBRR register
	stdout = &uart_output;							// Redirect STDOUT to UART
	stdin  = &uart_input;							// Redirect STDIN to UART

	// Power Stage Controller Setup
	bit_set(PSOC0,  BIT(POEN0A));					// PSC 0 Synchronization and Output Configuration - Enable Part A - Buzzer output
	//bit_set(PSOC1,  BIT());						// PSC 1 Synchronization and Output Configuration
	bit_set(PSOC2, BIT(POEN2B) | BIT(POEN2A));		// PSC 2 Synchronization and Output Configuration - Enable Part A & B - Inverter output

	bit_set(PCNF0, BIT(POP0));						// PSC 0 Configuration Register - 1 ramp mode, Active high
	bit_set(PCNF1, BIT(PCLKSEL1));					// PSC 1 Configuration Register - 1 ramp mode, fast clock
	bit_set(PCNF2, BIT(PCLKSEL2));					// PSC 2 Configuration Register - 1 ramp mode, fast clock

	bit_set(PCTL0, BIT(PPRE00));					// PSC 0 Control Register - /4 Prescaler
	//bit_set(PCTL1, BIT());						// PSC 1 Control Register
	bit_set(PCTL2, BIT(PARUN2));					// PSC 2 Control Register - Autostart PSC2 with PSC1

	//bit_set(PFRC0A, BIT());						// PSC 0 Input A Control Register - Enable capture, on PSCIN0, falling edge trigger, noise canceler off, no action only used for resonant inverter phase detection by reading input capture register
	//bit_set(PFRC0B, BIT());						// PSC 0 Input B Control Register
	bit_set(PFRC1A, BIT(PCAE0A));					// PSC 1 Input A Control Register - Enable capture, on PSCIN0, falling edge trigger, noise canceler off, no action only used for resonant inverter phase detection by reading input capture register
	//bit_set(PFRC1B, BIT());						// PSC 1 Input B Control Register
	bit_set(PFRC2A, BIT(PCAE2A) | BIT(PRFM2A2) | BIT(PRFM2A1) | BIT(PRFM2A0));	// PSC 2 Input A Control Register - Enable capture, on PSCIN2, low level trigger, mode 7 (halt)
	//bit_set(PFRC2B, BIT());						// PSC 2 Input B Control Register

	//bit_set(PIM0, BIT(PEVE0A));					// PSC 0 Interrupt Mask Register - Enable interrupt on block A capture event
	//bit_set(PIM1, BIT(PEVE1A));					// PSC 1 Interrupt Mask Register - Enable interrupt on block A capture event
	//bit_set(PIM2, BIT(PEVE2A));					// PSC 2 Interrupt Mask Register - Enable interrupt on block A capture event

	// Analog Comparator Setup
	//bit_set(AC0CON, BIT(AC0EN));					// ACMP0
	//bit_set(DIDR1, BIT(ACMP0D));					// Digital Input Disable for ACMP0

	// ADC Setup
	bit_set(ADMUX, BIT(MUX3) | BIT(MUX2) | BIT(MUX1) | BIT(MUX0));		// ADC Multiplexer Register (V ref on AREF, right adjust, Ground ADC input)
	bit_set(ADCSRA, BIT(ADEN) | BIT(ADSC) | BIT(ADPS2));				// ADC Control and Status Register A (Enable ADC, Start ADC (initialize), Prescaler 16)
	bit_set(ADCSRB, BIT(ADHSM));										// ADC Control and Status Register B (High Speed Mode)
	bit_set(DIDR0, BIT(ADC2D));											// Digital Input Disable Register 0
	bit_set(DIDR1, BIT(ADC8D) | BIT(ADC9D) | BIT(ADC10D));				// Digital Input Disable Register 1

	// LCD Setup
	lcd_init(LCD_DISP_ON);
	lcd_clrscr();

	// Scan One Wire for devices
	OW_nSensors = search_sensors();

	// INT1 Interrupt - AC Mains ZCD
    bit_set(EICRA, BIT(ISC11));					// INT1 Falling edge trigger
    bit_set(EIMSK, BIT(INT1));					// Enable INT1

	// INT3 Interrupt - Rotary encoder B output
	bit_set(EICRA, BIT(ISC31));					// INT3 Falling edge trigger
	bit_set(EIMSK, BIT(INT3));					// Enable INT3

	// Timer1 Interrupt - AC mains phase synchronization angle timer
	bit_set(TCCR1A, BIT(WGM12));				// Timer1 CTC Mode
	bit_set(TIMSK1, BIT(OCIE1A));				// Enable CTC Interrupt

	sei();										// Enable Global Interrupt

	inverter_current_max = 200;					// Initial maximum inverter current

	Kp = 1;
	Ki = 1;
	Kd = 0;

	phase_offset = 140;
}

/****************************************************************************
* MAIN
****************************************************************************/
int main(void)
{
	//_delay_ms(100);
	setup();
	reset_ss();

	lcd_gotoxy(0,0);
	lcd_puts("  ReactorForge.com");

	lcd_gotoxy(0,1);
	lcd_puts(" ");

	lcd_gotoxy(0,2);
	lcd_puts("ReactorCore V0.0a1");

	lcd_gotoxy(0,3);

	lcd_key = read_buttons();


// 	lcd_clrscr();
// 	while (1)
// 	{
// 		lcd_key = read_buttons();
//
// 		if (lcd_key == 1) sel = 1;
// 		if (lcd_key == 2) sel = 2;
// 		if (lcd_key == 3) sel = 3;
// 		if (lcd_key == 4) sel = 4;
//
// 		if (lcd_key == 5)
// 		{
// 			bit_flip(PCTL0, BIT(PRUN0));
// 			_delay_ms(250);
// 		}
//
// 		if (enc > enc_prev)
// 		{
// 			if (sel == 1) OCR0SA += 10;
// 			if (sel == 2) OCR0RA += 10;
// 			if (sel == 3) OCR0SB += 10;
// 			if (sel == 4) OCR0RB += 10;
// 		}
// 		if (enc < enc_prev)
// 		{
// 			if (sel == 1) OCR0SA -= 10;
// 			if (sel == 2) OCR0RA -= 10;
// 			if (sel == 3) OCR0SB -= 10;
// 			if (sel == 4) OCR0RB -= 10;
// 		}
// 		enc_prev = enc;
//
// 		lcd_gotoxy(14,0);
// 		if (sel == 1) lcd_puts("OCR0SA");
// 		if (sel == 2) lcd_puts("OCR0RA");
// 		if (sel == 3) lcd_puts("OCR0SB");
// 		if (sel == 4) lcd_puts("OCR0RB");
//
// 		lcd_gotoxy(0,0);
// 		itoa(OCR0SA, str, 10);
// 		lcd_puts("OCR0SA=");
// 		lcd_puts(str);
// 		lcd_puts("   ");
//
// 		lcd_gotoxy(0,1);
// 		itoa(OCR0RA, str, 10);
// 		lcd_puts("OCR0RA=");
// 		lcd_puts(str);
// 		lcd_puts("   ");
//
// 		lcd_gotoxy(0,2);
// 		itoa(OCR0SB, str, 10);
// 		lcd_puts("OCR0SB=");
// 		lcd_puts(str);
// 		lcd_puts("   ");
//
// 		lcd_gotoxy(0,3);
// 		itoa(OCR0RB, str, 10);
// 		lcd_puts("OCR0RB=");
// 		lcd_puts(str);
// 		lcd_puts("   ");
// 	}

// 	lcd_puts("    Select Mode");
// 	lcd_key = btnNONE;
// 	while(lcd_key == btnNONE)
// 	{
// 		lcd_key = read_buttons();
// 	}
// 	lcd_clrscr();


	bit_set(CONTACTOR_PORT, BIT(CONTACTOR_BIT));
	//#########################################################################################
	if (lcd_key == btn1)	// Enter manual control mode if button 1 is held during startup
	{
		//puts("Test Mode");
		lcd_clrscr();
		lcd_puts("Manual Control Mode");
		// ***Test Control***
		while(1)
		{
			lcd_key = read_buttons();

			if (lcd_key == 4)
			{
				_delay_ms(1000);
				lcd_key = read_buttons();
				if (lcd_key == 4) bit_flip(CONTACTOR_PORT, BIT(CONTACTOR_BIT));
			}

			if (bit_is_clear(SS_SW_PIN, SS_SW_BIT))
			{
				if (running_flag == 0)
				{
					reset_ss();
					start_psc();
					lcd_clrscr();
					bit_set(EIMSK, BIT(INT1));	// Enable INT1
					bit_set(INDICATOR_PORT, BIT(INDICATOR_BIT));	// Indicator on
				}
			}
			else
			{
				if (running_flag == 1)
				{
					stop_psc(0);
					lcd_clrscr();
					lcd_puts_P("Manual Control Mode");
					bit_clear(INDICATOR_PORT, BIT(INDICATOR_BIT));	// Indicator off
					bit_clear(PCTL0, BIT(PRUN0));		// Stop buzzer
				}
			}

// 			if (lcd_key == btn1)
// 			{
// 				if (running_flag == 0)
// 				{
// 					reset_ss();
// 					start_psc();
//
// 					lcd_clrscr();
// 					bit_set(EIMSK, BIT(INT1));	// Enable INT1
// 				}
// 				else
// 				{
// 					stop_psc(0);
//
// 					lcd_clrscr();
// 					lcd_puts_P("Test Mode");
// 					//puts("Test Mode");
// 				}
//
// 				_delay_ms(250);
// 			}

			if (enc > enc_prev)
			{
				decr_freq();
			}
			if (enc < enc_prev)
			{
				incr_freq(1);
			}
			enc_prev = enc;

			if (lcd_key == btn2)
			{
				incr_freq(1);
				_delay_ms(10);
			}

			if (lcd_key == btn1)
			{
				decr_freq();
				_delay_ms(10);
			}

// 			if (lcd_key == btn3)
// 			{
// 				decr_freq();
//
// 				//incr_duty_cycle(5);
//
// //  				deadtime = deadtime + 1;
// //  				set_frequency();
// //  				_delay_ms(250);
//  			}
//
// 			if (lcd_key == btn4)
// 			{
// 				incr_freq(1);
//
// 				//decr_duty_cycle(5);
//
// //  				deadtime = deadtime - 1;
// //  				set_frequency();
// //  				_delay_ms(250);
// 			}

			if (running_flag == 0) control_loop++;
			if ( (running_flag == 0) & (control_loop == 5) )
			{
				control_loop = 0;
				lcd_gotoxy(0,1);
				if ( DS18X20_start_meas( DS18X20_POWER_EXTERN, NULL ) == DS18X20_OK)
				{
					_delay_ms( DS18B20_TCONV_12BIT );
					for ( i = 0; i < OW_nSensors; i++ )
					{
						if ( DS18X20_read_decicelsius( &gSensorIDs[i][0], &deciCelsius ) == DS18X20_OK )
						{
							if (i == 2) lcd_gotoxy(0,2);
							if (i == 0) lcd_puts("Out ");
							if (i == 1) lcd_puts("Inv ");
							if (i == 2) lcd_puts("Amb ");
							if (i == 3) lcd_puts("In  ");

							char str[5];
							deciCelsius = ((18 * (deciCelsius / 10) + 32 * 10) + 5);
							DS18X20_format_from_decicelsius( deciCelsius , str, 10 );
							//itoa(deciCelsius, str, 10);
							lcd_puts(str);
							lcd_puts(" ");
						}
					}
				}
			}

			if (running_flag == 0)
			{
				if (enc > enc_prev) phase_offset++;
				if (enc < enc_prev) phase_offset--;
				enc_prev = enc;

				char str[5];					// Buffer for the ASCII string
				lcd_gotoxy(0,3);
				itoa(phase_offset, str, 10);
				lcd_puts(str);
				lcd_puts("  ");
			}

			if ( (running_flag == 1) & (ac_peak == 1) )
			{
				ac_peak = 0;

				phase_current = (PICR1 & 0x0FFF) - phase_offset;
				//phase_delta = phase_previous - phase_current;
				phase_delta = abs(phase_previous - phase_current);

				if (phase_delta < 100)
				{
					phase_average = ( ( (filter_level - 1) * phase_average) + phase_current) / filter_level;	// Moving average filter
					error_current = ( ( ( (filter_level - 1) * error_current) + abs(phase_average - ontime) ) / filter_level );
				}

				//setpoint = (phase_current - phase_offset);

				// Get ADC Values
				inverter_current = get_adc(9);
				mains_current = get_adc(8);

				inverter_current_avg = ( ( (filter_level - 1) * inverter_current_avg) + inverter_current) / filter_level;
				mains_current_avg = ( ( (filter_level - 1) * mains_current_avg) + mains_current) / filter_level;

				if (bit_is_set(PIFR2, PEV2A))						// If IGBT hardware fault has been detected on PSCIN2 via external event A interrupt flag register
				{
					stop_psc(igbt_fault);
				}

// 				if (lock_flag = 0)
// 				{
// 					if (phase_average > ontime) decr_freq();
// 					if (phase_average < ontime) lock_flag = 1;
// 				}
// 				if (lock_flag = 1)
// 				{
// 					if ((phase_average - (phase_deadband * 1)) > ontime) decr_freq();
// 					if ((phase_average + (phase_deadband * 1)) < ontime) incr_freq(1);
// 				}



				if (control_loop == 0 || control_loop == 5)
				{
					print_buf[0] = start_code;
					print_buf[1] = 0;//setpoint;
					print_buf[2] = ontime;
					print_buf[3] = phase_current;
					print_buf[4] = phase_average;
					print_buf[5] = phase_adjustment;
					print_buf[6] = error_current;
					print_buf[7] = inverter_current_max;
					print_buf[8] = inverter_current_avg;
					print_buf[9] = mains_current_avg;

					for (i = 0; i < 10; i++)
					{
						uart_print16( print_buf[i] );
					}

					char str[5];					// Buffer for the ASCII string
					//lcd_clrscr();
					lcd_home();
					itoa(inverter_current, str, 10);
					lcd_puts(str);
					lcd_puts(" ");
					itoa(mains_current, str, 10);
					lcd_puts(str);
					lcd_puts("   ");

// 					lcd_gotoxy(0,1);
// 					itoa(ontime, str, 10);
// 					lcd_puts(str);
// 					lcd_puts(" ");
// 					itoa(setpoint, str, 10);
// 					lcd_puts(str);
// 					lcd_puts(" ");
// 					itoa(phase_current, str, 10);
// 					lcd_puts(str);
// 					lcd_puts("   ");

// 					lcd_gotoxy(0,3);
// 					itoa(lcd_key, str, 10);   // convert integer into string (decimal format)
// 					lcd_puts(str);

					lcd_gotoxy(0,1);
				}

				if (control_loop == 1)
				{
					DS18X20_start_meas( DS18X20_POWER_EXTERN, NULL );

					control_loop2++;
					if (control_loop2 == 10)
					{
						control_loop2 = 0;
						bit_set(PCTL0, BIT(PRUN0));		// Buzzer on
					}
				}

				if ( (control_loop >= 6) & (control_loop <=9) )
				{
					i = control_loop - 6;

					if ( DS18X20_read_decicelsius( &gSensorIDs[i][0], &deciCelsius ) == DS18X20_OK )
					{
						//uart_print16( deciCelsius );

						if (i == 2) lcd_gotoxy(0,2);
						if (i == 0) lcd_puts("Out ");
						if (i == 1) lcd_puts("Inv ");
						if (i == 2) lcd_puts("Amb ");
						if (i == 3) lcd_puts("In  ");

						char str[5];
						deciCelsius = ((18 * (deciCelsius / 10) + 32 * 10) + 5);
						DS18X20_format_from_decicelsius( deciCelsius , str, 10 );
						lcd_puts(str);
						lcd_puts(" ");
					}

// 					if ( DS18X20_start_meas( DS18X20_POWER_EXTERN, NULL ) == DS18X20_OK)
// 					{
// 						//if (OW_nSensors == 0)
// 						//{
// 						//	lcd_puts("No sensors found");
// 						//}
// 						//_delay_ms( DS18B20_TCONV_12BIT );
// 						for ( i = 0; i < OW_nSensors; i++ )
// 						{
// 							if ( DS18X20_read_decicelsius( &gSensorIDs[i][0], &deciCelsius ) == DS18X20_OK )
// 							{
// 								uart_print16( deciCelsius );
// 								if (i == 2) lcd_gotoxy(0,2);
//
// 								char str[5];
// 								DS18X20_format_from_decicelsius( deciCelsius, str, 10 );
// 								lcd_puts(str);
// 								lcd_puts(" ");
//
// 								//fahrenheit = (deciCelsius * 9.0)/ 5.0 + 32.0;
// 								//fahrenheit = (18*(deciCelsius)+(10*16*32))/16;
// 								//DS18X20_format_from_decicelsius( fahrenheit, s, 10 );
//
// 							}
// 							//else
// 							//{
// 							//	lcd_puts( "CRC Error" );
// 							//	OW_error++;
// 							//}
// 						}
// 					}
				}

				control_loop++;
				if (control_loop == 10)
				{
					control_loop = 0;
					//bit_flip(INDICATOR_PORT, BIT(INDICATOR_BIT));
					bit_clear(PCTL0, BIT(PRUN0));		// Buzzer off
				}

				phase_previous = phase_current;

				bit_set(EIMSK, BIT(INT1));	// Enable INT1

			}

		}
		// ***END Test Control***
	}

	//#########################################################################################
	//#########################################################################################
	//if (lcd_key == btnRIGHT)	// Enter manual control mode if button 2 is held during startup
	//{
		//puts("Ready");
		//_delay_ms(500);
		lcd_puts("Ready");

		while(1)
		{
			lcd_key = read_buttons();

			if (bit_is_clear(SS_SW_PIN, SS_SW_BIT))
			{
				if (running_flag == 0)
				{
					reset_ss();
					start_psc();
					lcd_clrscr();
					bit_set(EIMSK, BIT(INT1));	// Enable INT1
					bit_set(INDICATOR_PORT, BIT(INDICATOR_BIT));	// Indicator on
				}
			}
			else
			{
				if (running_flag == 1)
				{
					stop_psc(0);
					lcd_clrscr();
					lcd_puts_P("Ready");
					bit_clear(INDICATOR_PORT, BIT(INDICATOR_BIT));	// Indicator off
					bit_clear(PCTL0, BIT(PRUN0));		// Stop buzzer
				}
			}

			if (running_flag == 1)
			{
				if (enc < enc_prev)
				{
					if (inverter_current_max > 14)
					{
						inverter_current_max -= 5;
					}
				}
				if (enc > enc_prev)
				{
					inverter_current_max += 5;
					if (inverter_current_max > 1024)
					{
						inverter_current_max = 1024;
					}
				}
				enc_prev = enc;

// 				if (lcd_key == btn3)
// 				{
// 					inverter_current_max = inverter_current_max + 5;
// 					if (inverter_current_max > 1024)
// 					{
// 						inverter_current_max = 1024;
// 					}
// 					//Ki++;
// 				}
//
// 				if (lcd_key == btn2)
// 				{
// 					if (inverter_current_max > 14)
// 					{
// 						inverter_current_max = inverter_current_max - 5;
// 					}
// 					//Ki--;
// 					//if (Ki < 0) Ki = 0;
// 				}

	// 			if (lcd_key == btn4)
	// 			{
	// 				//setpoint++;
	// 				//phase_adjustment++;
	// 				Kp++;
	// 			}
	//
	// 			if (lcd_key == btn3)
	// 			{
	// 				//setpoint--;
	// 				//phase_adjustment--;
	// 				Kp--;
	// 				if (Kp < 1) Kp = 1;
	// 			}
			}

			if (running_flag == 0)
			{
				if (enc > enc_prev) phase_offset++;
				if (enc < enc_prev) phase_offset--;
				enc_prev = enc;

				char str[5];					// Buffer for the ASCII string
				lcd_gotoxy(0,1);
				itoa(phase_offset, str, 10);
				lcd_puts(str);
				lcd_puts("  ");
			}

			if ( (running_flag == 1) & (ac_peak == 1) )
			{
				//bit_set(PORTE, BIT(1));
				ac_peak = 0;

				phase_current = (PICR1 & 0x0FFF);					// Get the value from the last PSC0 capture event (Mask out upper 4 bits) - represents phase offset between inverter voltage and inverter current
				phase_delta = abs(phase_previous - phase_current);

				if (phase_delta < 100)
				{
					phase_average = ( ( (filter_level - 1) * phase_average) + phase_current) / filter_level;	// Moving average filter
				}

				// Get ADC Values
				inverter_current = get_adc(9);
				mains_current = get_adc(8);
				//mains_current = ( ( 1 * mains_current) + get_adc(3)) / 2; // test filter on mains current

				// Calculate resonant phase errors
				//error_current = phase_current - setpoint;
				//error_current = abs(setpoint - ontime);
				error_current = ( ( ( (filter_level - 1) * error_current) + abs(setpoint - ontime) ) / filter_level );
				error_delta = abs(error_current - error_previous);	// Save error delta between current and previous error
				error_previous = error_current;						// Save current error to previous error before next control loop iteration

				// ***Halt condition checks***
				if (bit_is_set(PIFR2, PEV2A))						// If IGBT hardware fault has been detected on PSCIN2 via external event A interrupt flag register
				{
					stop_psc(igbt_fault);
				}

// 	 			if ((ontime == ontime_ceiling) & (first_pass == 1))						// If ontime ceiling is reached then halt
// 				{
// 	 				//error_status = ceiling_reached;
// 	 				stop_psc(ceiling_reached);
// 	 			}

	 			if (ontime == ontime_floor)							// If ontime floor is reached then halt
				{
	 				//error_status = floor_reached;
	 				stop_psc(floor_reached);
	 			}

				if ((scan_period == 0) & (lock_flag == 0))			// If no lock has occurred in the alloted time
				{
					//error_status = scan_timeout;
					stop_psc(scan_timeout);
				}

// 				if (lock_flag == 1)
// 				{
// 	//  				if (inverter_current > (inverter_current_max + 200))	// If instantaneous inverter current spikes above maximum allowed after resonant lock
// 	// 				{
// 	// 					//error_status = inverter_current_spike;
// 	// 					stop_psc(inverter_current_spike);
// 	// 				}
//
// 					if (phase_delta > delta_max)
// 					{
// 						//error_status = phase_delta_max;
// 						stop_psc((phase_delta_max));
// 					}
//
// 					if (phase_delta < delta_min)
// 					{
// 						//error_status = phase_delta_min;
// 						stop_psc((phase_delta_min));
// 					}
//
// 					if (upper_limit_cnt == upper_limit_hits)		// If the upper limit has been hit over the Upper_limit_hits setting then halt
// 					{
// 						//error_status = upper_soft_limit;
// 						stop_psc(upper_soft_limit);
// 					}
//
// 					if (lower_limit_cnt == lower_limit_hits)		// If the lower limit has been hit over the Lower_limit_hits setting then halt
// 					{
// 						//error_status = lower_soft_limit;
// 						stop_psc(lower_soft_limit);
// 					}
// 				}
				// ***END Halt condition checks***


				// Resonant lock detection
				//if ((phase_current == setpoint) & (error_current < phase_deadband) & (abs(phase_delta) < 5) & (error_current < setpoint) & (error_current > 0) & (first_pass == 1) & (lock_flag == 0))	//Resonance has been achieved, set flag and lock value
				//if ((phase_current == setpoint) & (first_pass == 1) & (lock_flag == 0))	//Resonance has been achieved, set flag and lock value
				if ( (error_current < (phase_deadband * 2)) & (error_delta < 3) & (first_pass == 1) & (lock_flag == 0) & (ontime >= setpoint - phase_deadband) )
				{
					lock_flag = 1;
					lock_value = ontime;
				}

				//Proportional control for resonant tank frequency to regulate maximum inverter current
	 			//if (lock_flag == 1)
	 			//{
// 	 				if (mains_current > inverter_current_max)
// 	 				{
// 	 					phase_adjustment = phase_adjustment + 2;
//
// 						if ( (mains_current - inverter_current_max) > 200)
// 						{
// 							phase_adjustment = phase_adjustment + 10;
// 						}
// 	 				}
//
// 	 				if (((mains_current + 25) < inverter_current_max) & phase_adjustment > 0)
// 	 				{
// 	 					phase_adjustment = phase_adjustment - 1;
// 	 				}

	  				//phase_adjustment = (inverter_current - inverter_current_max) + phase_adjustment;

// 					pid_error = inverter_current_max - inverter_current;
// 					pTerm = Kp * pid_error;
// 					if ( (inverter_current > inverter_current_max) || (phase_adjustment < 0)) iTerm += Ki * pid_error;
// 					//dTerm = Kd * (pid_error - pid_previous_error);
// 					//pid_previous_error = pid_error;
// 					pid_value = pTerm + iTerm + dTerm;
//
// 					phase_adjustment = pid_value;
//
// 	  				if (phase_adjustment > 10) // || (inverter_current < inverter_current_max)
// 	  				{
// 		  				phase_adjustment = 0;
// 	  				}
// 	  				if (phase_adjustment < -250)
// 	  				{
// 		  				phase_adjustment = -250;
// 	  				}
	 			//}

				// Calculate resonant setpoint
				setpoint = (phase_current - phase_offset) - phase_adjustment;
// 				word_temp = (phase_average - (phase_offset - phase_adjustment) );
// 				if (phase_adjustment != 0)
// 				{
// 					if ( (setpoint > (word_temp - 50)) || (setpoint < (word_temp + 50)) ) setpoint = word_temp;
// 					if (setpoint < ontime_ceiling) setpoint = ontime_ceiling;
// 					if (setpoint > ontime_floor) setpoint = ontime_floor;
// 				}
// 				else
// 				{
// 					setpoint = word_temp;
// 				}

// 				if (lock_flag == 0)
// 				{
// 					//setpoint = ((ontime / 2) - deadtime) - 10;
// 					//setpoint = ((ontime - (deadtime)) / 2);					// Re-calculate setpoint
//
// 					//setpoint = ((ontime / 2) - (deadtime * 2)) - 5;
//
//
// 					scan_period = scan_period - 1;							// Count down from initial scan period, this value is ignored after a successful lock
// 					first_pass = 1;											// Set first pass to indicate to future control loop iterations that at least control loop one pass has already taken place
// 				}

				// Frequency adjustment
	// 			phase_offset = (setpoint + phase_adjustment) + phase_deadband;
	// 			if (phase_current > phase_offset)							// Decrease frequency if phase reading is greater than the setpoint + deadband (account for phase_adjustment)
	// 			{
	// 				decr_freq();
	// 			}
	//
	// 			phase_offset = (setpoint + phase_adjustment) - phase_deadband;
	// 			if ((phase_current < phase_offset) & (lock_flag == 1))		// Increase frequency if the phase reading is less than the setpoint + deadband and a lock has already been established (pre-lock should scan down from ceiling to floor only) (account for phase_adjustment)
	// 			{
	// 				incr_freq();
	// 			}
				if ( (setpoint - phase_deadband) < ontime )				// Increase frequency if the phase reading is less than the setpoint + deadband and a lock has already been established (pre-lock should scan down from ceiling to floor only) (account for phase_adjustment)
				{
					incr_freq(1);
				}

				if ( (setpoint + phase_deadband) > ontime )			// Decrease frequency if phase reading is greater than the setpoint + deadband (account for phase_adjustment)
				{
					decr_freq();

					if (lock_flag == 0) decr_freq();					// Increase pre-lock scanning speed (decrease time to a successful lock)
				}

	// 			if (((phase_current - ontime) > 100) | ((phase_current - ontime) < 95))
	// 			{
	// 				ontime = phase_current - 100;
	// 				set_frequency();
	// 			}



				/* PID example */
	// 			pid_error = setpoint_value - current_value;
	// 			pTerm = Kp * pid_error;
	// 			iTerm += Ki * pid_error;
	// 			//dTerm = Kd * (pid_error - pid_previous_error);
	// 			pid_previous_error = pid_error;
	// 			pid_value = pTerm + iTerm + dTerm;

				phase_previous = phase_current;		// Save current phase value


				//print_buf[1] = start_code;
//  				print_buf[1] = error_current;
//  				print_buf[2] = phase_offset;//tank_voltage;
//  				print_buf[3] = inverter_current;
//  				print_buf[4] = setpoint;
//  				print_buf[5] = ontime;
//  				print_buf[6] = phase_current;
//  				print_buf[7] = mains_current;
//  				print_buf[8] = phase_adjustment;
//  				print_buf[9] = lock_value;
//  				print_buf[10] = inverter_current_max;
//
//  				int i;
//  				for (i = 1; i < 11; i++)
//  				{
//  					char str[5];					// Buffer for the ASCII string
//  					itoa(print_buf[i], str, 10);	// Convert the integer (print_buf) to an ASCII string (str) in base 10
//  					printf("%s,", str);				// Print ASCII value with , separator
//  				}
//  				puts("");							// New line

				control_loop++;
				if (control_loop == 5)
				{
					control_loop = 0;

					print_buf[0] = start_code;
					print_buf[1] = setpoint;
					print_buf[2] = ontime;
					print_buf[3] = phase_current;
					print_buf[4] = phase_average;
					print_buf[5] = phase_adjustment;
					print_buf[6] = error_current;
					print_buf[7] = inverter_current_max;
					print_buf[8] = inverter_current;
					print_buf[9] = mains_current;


					int i;
					for (i = 0; i < 10; i++)
					{
						//char str[5];					// Buffer for the ASCII string
						//itoa(print_buf[i], str, 10);	// Convert the integer (print_buf) to an ASCII string (str) in base 10
						//printf("%s,", str);				// Print ASCII value with , separator
						uart_print16( print_buf[i] );
					}
					//puts("");							// New line

					char str[5];					// Buffer for the ASCII string
					//lcd_clrscr();
// 					lcd_home();
// 					itoa(inverter_current, str, 10);
// 					lcd_puts(str);
// 					lcd_puts_P(" ");
// 					itoa(mains_current, str, 10);
// 					lcd_puts(str);
// 					lcd_puts_P(" ");

 					lcd_gotoxy(0,1);
  					itoa(inverter_current_max, str, 10);
  					lcd_puts(str);
  					lcd_puts(" ");
					itoa(Kp, str, 10);
					lcd_puts(str);
					lcd_puts(" ");
					//if (lock_flag == 1) { lcd_puts_P("Lock"); }
   					//itoa(setpoint, str, 10);
   					//lcd_puts(str);
   					//lcd_puts(" ");
   					//itoa(phase_current, str, 10);
   					//lcd_puts(str);
   					//lcd_puts("   ");

					//bit_flip(INDICATOR_PORT, BIT(INDICATOR_BIT));
					bit_clear(PCTL0, BIT(PRUN0));		// Buzzer off
 				}

				if (control_loop == 0)
				{
					bit_set(INDICATOR_PORT, BIT(INDICATOR_BIT));

				}

				first_pass = 1;

				//bit_clear(PORTE, BIT(1));
				bit_set(EIMSK, BIT(INT1));	// Enable INT1
			}
		}
	//}
}


/****************************************************************************
* NAME:        reset_ss
* DESCRIPTION: Reset cycle updated system settings
* ARGUMENTS:   void
* RETURNS:     void
****************************************************************************/
void reset_ss(void)
{
	setpoint = 160;
	phase_deadband = 5;
	phase_adjustment = 0;
	ontime_floor = 800;
	ontime_ceiling = 95;		// 170= ~78KHz
	ontime = ontime_ceiling + 1;
	deadtime = 32;
	frq_range = 25;
	scan_period = 500;
	delta_max = 200;
	delta_min = -200;
	upper_limit_hits = 100;
	lower_limit_hits = 100;

	set_frequency();
	bit_clear(PCTL1, BIT(PRUN1));		// Ensure PSC is stopped

	// Reset program variables
	running_flag = 0;
	first_pass = 0;
	lock_flag = 0;
	bit_set(PIFR2, BIT(PEV2A));			// Ensure external event A interrupt flag is cleared
	lock_value = 0;
	upper_limit_cnt = 0;
	lower_limit_cnt = 0;
	phase_current = 0;
	phase_previous = 0;
	phase_delta = 0;
	//phase_offset = 100;
	phase_average = 200;
	filter_level = 5;
	error_current = 100;
	error_previous = 0;
	error_delta = 0;

	pTerm = 0;
	iTerm = 0;
	dTerm = 0;

	OCR1A = 72;		// Mains AC firing angle - ADC synchronization to pulse peak

	OCR0SA = 0;		// Buzzer frequency: 600 = aprox. 3310Hz
	OCR0RA = 600;
	OCR0SB = 0;
	OCR0RB = 1200;
}


/****************************************************************************
* NAME:        start_psc
* DESCRIPTION: Start PSC
* ARGUMENTS:   void
* RETURNS:     void
****************************************************************************/
void start_psc(void)
{
	if (bit_is_clear(PIND, 2))			// Do not start PSC if an IGBT hardware fault is detected
	{
		stop_psc(igbt_fault);
	}
	else
	{
		bit_set(PCTL1, BIT(PRUN1));
		running_flag = 1;
	}
}


/****************************************************************************
* NAME:        stop_psc
* DESCRIPTION: Stop PSC
* ARGUMENTS:   uint8_t error_status
* RETURNS:     void
****************************************************************************/
void stop_psc(uint8_t error_status)
{
	bit_clear(PCTL1, BIT(PRUN1));
	running_flag = 0;

	if (error_status > 0)
	{
		char str[5];					// Buffer for the ASCII string
		itoa(error_status, str, 10);	// Convert the integer (print_buf) to an ASCII string (str) in base 10
		puts(str);						// Print ASCII value

		lcd_clrscr();
		lcd_puts("Fault=");
		lcd_puts(str);
	}
}


/****************************************************************************
* NAME:        incr_freq
* DESCRIPTION: Increment PSC frequency by 1 ontime step
* ARGUMENTS:   void
* RETURNS:     void
****************************************************************************/
void incr_freq(uint8_t steps)
{
	if (lock_flag == 1)
	{
		byte_temp = (lock_value + phase_adjustment) - frq_range;	// Calculate upper soft frequency limit
		if (ontime == byte_temp)									// Increment upper limit count and exit if limit is hit
		{
			upper_limit_cnt = upper_limit_cnt + 1;
			return;
		}
		upper_limit_cnt = 0;
	}
	ontime = ontime - steps;							// Increment PSC frequency
	if (ontime < 95)
	{
		ontime = 95;									// Hard upper limit
	}

	set_frequency();
}


/****************************************************************************
* NAME:        decr_freq
* DESCRIPTION: Decrement PSC frequency by 1 ontime step
* ARGUMENTS:   void
* RETURNS:     void
****************************************************************************/
void decr_freq(void)
{
	if (lock_flag == 1)
	{
		byte_temp = (lock_value + phase_adjustment) + frq_range;	// Calculate lower soft frequency limit
		if (ontime == byte_temp)									// Increment lower limit count and exit if limit is hit
		{
			lower_limit_cnt = lower_limit_cnt + 1;
			return;
		}
		lower_limit_cnt = 0;
	}
	ontime = ontime + 1;								// Decrement PSC frequency
	if (ontime > 800)
	{
		ontime = 800;									// Hard lower limit
	}

	set_frequency();
}


/****************************************************************************
* NAME:        set_frequency
* DESCRIPTION: Calculate and set inverter PWM output compare registers based on deadtime and ontime
* ARGUMENTS:   void
* RETURNS:     void
****************************************************************************/
void set_frequency(void)
{
	bit_set(PCNF2, BIT(PLOCK2));					// Set lock bits to prevent change during update
	bit_set(PCNF1, BIT(PLOCK1));

	//***PSC2***									// PSC2 Inverter ouput compare registers (1 ramp mode)
	OCR2SA = deadtime;
	OCR2RA = OCR2SA + ontime;
	OCR2SB = OCR2RA + deadtime;
	OCR2RB = OCR2SB + ontime;

	//***PSC1***									// PSC1 only used for phase detection - Mirror PSC1 ouput compare registers (1 ramp mode)
	OCR1SA = OCR2SA;
	OCR1RA = OCR2RA;
	OCR1SB = OCR2SB;
	OCR1RB = OCR2RB;

	bit_clear(PCNF2, BIT(PLOCK2));					// Release lock bits to apply update
	bit_clear(PCNF1, BIT(PLOCK1));
}


/****************************************************************************
* NAME:        get_adc
* DESCRIPTION: Get ADC value on the requested pin, return 16 bit value
* ARGUMENTS:   uint8_t mux_val
* RETURNS:     uint16_t
****************************************************************************/
uint16_t get_adc(uint8_t mux_val)
{
	ADMUX = mux_val;									// Connect ADC MUX to requested pin
	bit_set(ADCSRA, BIT(ADSC));							// Start ADC conversion
	loop_until_bit_is_clear(ADCSRA, ADSC);				// Wait for conversion
	return (ADCL | (ADCH << 8));						// Return 16-bit ADC reading
}


/****************************************************************************
* NAME:        incr_duty_cycle
* DESCRIPTION: Increment buck converter PWM duty cycle by 1 ontime step
* ARGUMENTS:   void
* RETURNS:     void
****************************************************************************/
// void incr_duty_cycle(uint16_t dc_duty_cycle_change)
// {
// 	if (dc_duty_cycle_on < dc_max_duty_cycle)
// 	{
//
// 		dc_duty_cycle_on = dc_duty_cycle_on + dc_duty_cycle_change;
// 		set_duty_cycle();
// 	}
// }


/****************************************************************************
* NAME:        decr_duty_cycle
* DESCRIPTION: Decrement buck converter PWM duty cycle by 1 ontime step
* ARGUMENTS:   void
* RETURNS:     void
****************************************************************************/
// void decr_duty_cycle(uint16_t dc_duty_cycle_change)
// {
// 	if (dc_duty_cycle_on > dc_min_duty_cycle)
// 	{
// 		dc_duty_cycle_on = dc_duty_cycle_on - dc_duty_cycle_change;
// 	set_duty_cycle();
// 	}
// }


/****************************************************************************
* NAME:        set_duty_cycle
* DESCRIPTION: Calculate and set buck converter PWM output compare registers
* ARGUMENTS:   void
* RETURNS:     void
****************************************************************************/
// void set_duty_cycle(void)
// {
// 	bit_set(PCNF2, BIT(PLOCK2));							// Set lock bits to prevent change during update
//
// 	dc_duty_cycle_off = (dc_ontime * 2) - dc_duty_cycle_on;	// Duty cycle calculation
//
// 	//***PSC2***											// Buck converter compare registers (4 ramp mode)
// 	OCR2SA = dc_deadtime;
// 	OCR2RA = dc_duty_cycle_on;
// 	OCR2SB = dc_deadtime;
// 	OCR2RB = dc_duty_cycle_off;
//
// 	bit_clear(PCNF2, BIT(PLOCK2));							// Release lock bits to apply update
// }


// /****************************************************************************
// * NAME:        uart_get
// * DESCRIPTION: Get one byte (8 bits) from UART
// * ARGUMENTS:   void
// * RETURNS:     uint8_t
// ****************************************************************************/
// uint8_t uart_get(void)
// {
// while(!(UCSRA & (1<<RXC))){}	// Wait until a data is available
// return UDR;
// }
//
//
/****************************************************************************
* NAME:        uart_print8
* DESCRIPTION: Write one byte (8 bits) to UART
* ARGUMENTS:   uint8_t
* RETURNS:     void
****************************************************************************/
static void uart_print8(uint8_t val)
{
	while(!(UCSRA & (1<<UDRE))){}	// Wait until the transmitter is ready
	UDR=val;
}


/****************************************************************************
* NAME:        uart_print16
* DESCRIPTION: Write one word (16 bits) to UART
* ARGUMENTS:   uint16_t
* RETURNS:     void
****************************************************************************/
static void uart_print16(uint16_t val)
{
	uart_print8(val);
	uart_print8(val >> 8);
}


static int uart_putchar(char c, FILE *stream) {
	if (c == '\n') {
		uart_putchar('\r', stream);
	}
	loop_until_bit_is_set(UCSRA, UDRE);
	UDR = c;
  return 0;
}


static int uart_getchar(FILE *stream) {
	loop_until_bit_is_set(UCSRA, RXC); /* Wait until data exists. */
	return UDR;
}


static uint8_t search_sensors(void)
{
	uint8_t i;
	uint8_t id[OW_ROMCODE_SIZE];
	uint8_t diff, nSensors;

	//uart_puts_P( NEWLINESTR "Scanning Bus for DS18X20" NEWLINESTR );

	ow_reset();

	nSensors = 0;

	diff = OW_SEARCH_FIRST;
	while ( diff != OW_LAST_DEVICE && nSensors < MAXSENSORS ) {
		DS18X20_find_sensor( &diff, &id[0] );

		if( diff == OW_PRESENCE_ERR ) {
			//uart_puts_P( "No Sensor found" NEWLINESTR );
			break;
		}

		if( diff == OW_DATA_ERR ) {
			//uart_puts_P( "Bus Error" NEWLINESTR );
			break;
		}

		for ( i=0; i < OW_ROMCODE_SIZE; i++ )
		gSensorIDs[nSensors][i] = id[i];

		nSensors++;
	}

	return nSensors;
}


int read_buttons(void)
{
	adc_key_in = get_adc(10);					// Get ADC value from button port

	//if (key_lockout_time > 0) key_lockout_time--;

	//if ( (key_lockout_time == 0) || (adc_key_in > 1000) )
	//{
		if (adc_key_in > 1000) return btnNONE;

		//key_lockout_time = 2000;
		if (adc_key_in < 50)   return btn5;
		if (adc_key_in < 200)  return btn4;
		if (adc_key_in < 400)  return btn3;
		if (adc_key_in < 600)  return btn2;
		if (adc_key_in < 800)  return btn1;
	//}


	return btnNONE;
}


/****************************************************************************
* NAME:        debounce_button
* DESCRIPTION: Simple button debounce routine
* ARGUMENTS:   volatile uint8_t * pin_in, uint8_t bit
* RETURNS:     int
****************************************************************************/
// int debounce_button(volatile uint8_t * pin_in, uint8_t bit)
// {
// 	if (BIT_VAL (*pin_in, bit) == 0)
// 	{
// 		_delay_ms(DEBOUNCE_TIME);
// 		if (BIT_VAL (*pin_in, bit) == 0) return 1;
// 	}
//
// 	return 0;
// }


/****************************************************************************
* NAME:        ISR:PSC0_CAPT
* DESCRIPTION: INT1 Interrupt service routine - trips when an IGBT hardware fault occurs - PSC2 Capture Event Block A (active low)
* ARGUMENTS:   PSC2_CAPT_vect
* RETURNS:     void
****************************************************************************/
// ISR(PSC2_CAPT_vect)
// {
// 	bit_set(PIFR2, BIT(PEV2A));		// Clear interrupt flag register
// 	fault_flag = 1;					// Set fault flag
// }


/****************************************************************************
* NAME:        ISR:INT1_vect
* DESCRIPTION: INT1 Interrupt service routine - triggered on the falling edge of AC mains ZCD pulse
* ARGUMENTS:   INT1_vect
* RETURNS:     void
****************************************************************************/
ISR(INT1_vect, ISR_BLOCK)
{
	bit_set(TCCR1B, BIT(CS12) | BIT(CS10));			// Start Timer1 with a Prescaler of 1024 - Starts the AC mains phase synchronization angle timer
}


/****************************************************************************
* NAME:        ISR:INT3_vect
* DESCRIPTION: INT3 Interrupt service routine - triggered on the falling edge of rotary encoder B output
* ARGUMENTS:   INT3_vect
* RETURNS:     void
****************************************************************************/
ISR(INT3_vect, ISR_BLOCK)
{
	// Check the level of encoder output A to determine direction
	if (bit_is_set(ENC_PIN, ENC_BIT))
		enc++;
	else
		enc--;
}


/****************************************************************************
* NAME:        ISR:TIMER1_COMP_A_vect
* DESCRIPTION: TIMER1 CTC Interrupt service routine - AC mains phase synchronization angle timer
* ARGUMENTS:   TIMER1_COMPA_vect
* RETURNS:     void
****************************************************************************/
ISR(TIMER1_COMPA_vect, ISR_BLOCK)
{
	bit_clear(TCCR1B, BIT(CS12) | BIT(CS10));	// Stop Timer1
	TCNT1 = 0;									// Clear the timer counter so we start fresh on the next delay (TODO is this needed?)

	bit_clear(EIMSK, BIT(INT1));				// Disable INT1
	ac_peak = 1;								// Set AC peak detected flag
}
