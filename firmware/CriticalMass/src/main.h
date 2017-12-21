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

#include "lcd.h"
#include "ds18x20.h"

#define USART_BAUDRATE 57600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
#define NEWLINESTR "\r"



#endif /* MAIN_H */
