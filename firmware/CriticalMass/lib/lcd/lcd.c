/****************************************************************************
 Title	:   HD44780U LCD library
 Author:    Peter Fleury <pfleury@gmx.ch>  http://jump.to/fleury
 File:	    $Id: lcd.c,v 1.14.2.2 2012/02/12 07:51:00 peter Exp $
 Software:  AVR-GCC 3.3
 Target:    any AVR device, memory mapped mode only for AT90S4414/8515/Mega

 DESCRIPTION
       Basic routines for interfacing a HD44780U-based text lcd display

       Originally based on Volker Oth's lcd library,
       changed lcd_init(), added additional constants for lcd_command(),
       added 4-bit I/O mode, improved and optimized code.

 USAGE
       See the C include lcd.h file for a description of each function

*****************************************************************************/

#include <util/delay.h>
#include <lcd.h>


/*
** constants/macros
*/
#define DDR(x) (*(&x - 1))		// Address of data direction register of port x
#define PIN(x) (*(&x - 2))		// Address of input register of port x

#define lcd_e_high()    LCD_E_PORT  |=  _BV(LCD_E_PIN);
#define lcd_e_low()     LCD_E_PORT  &= ~_BV(LCD_E_PIN);
#define lcd_rs_high()   LCD_RS_PORT |=  _BV(LCD_RS_PIN)
#define lcd_rs_low()    LCD_RS_PORT &= ~_BV(LCD_RS_PIN)

#if LCD_LINES==1
#define LCD_FUNCTION_DEFAULT    LCD_FUNCTION_4BIT_1LINE
#else
#define LCD_FUNCTION_DEFAULT    LCD_FUNCTION_4BIT_2LINES
#endif

/*
** function prototypes
*/
static void lcd_e_toggle(void);

/*
** local functions
*/

/* toggle Enable Pin to initiate write */
static void lcd_e_toggle(void)
{
    lcd_e_high();
	_delay_us(1);
    lcd_e_low();
}


/*************************************************************************
Low-level function to write byte to LCD controller one nibble at a time
Input:    data   byte to write to LCD
          rs     1: write data
                 0: write instruction
Returns:  none
*************************************************************************/
static void lcd_write(uint8_t data, uint8_t rs)
{
    //unsigned char dataBits ;

    if (rs)
	{	/* write data        (RS=1, RW=0) */
       lcd_rs_high();
    }
	else
	{	/* write instruction (RS=0, RW=0) */
       lcd_rs_low();
    }

    /* output high nibble first */
    LCD_DATA3_PORT &= ~_BV(LCD_DATA3_PIN);
    LCD_DATA2_PORT &= ~_BV(LCD_DATA2_PIN);
    LCD_DATA1_PORT &= ~_BV(LCD_DATA1_PIN);
    LCD_DATA0_PORT &= ~_BV(LCD_DATA0_PIN);
    if(data & 0x80) LCD_DATA3_PORT |= _BV(LCD_DATA3_PIN);
    if(data & 0x40) LCD_DATA2_PORT |= _BV(LCD_DATA2_PIN);
    if(data & 0x20) LCD_DATA1_PORT |= _BV(LCD_DATA1_PIN);
    if(data & 0x10) LCD_DATA0_PORT |= _BV(LCD_DATA0_PIN);
    lcd_e_toggle();

    /* output low nibble */
    LCD_DATA3_PORT &= ~_BV(LCD_DATA3_PIN);
    LCD_DATA2_PORT &= ~_BV(LCD_DATA2_PIN);
    LCD_DATA1_PORT &= ~_BV(LCD_DATA1_PIN);
    LCD_DATA0_PORT &= ~_BV(LCD_DATA0_PIN);
    if(data & 0x08) LCD_DATA3_PORT |= _BV(LCD_DATA3_PIN);
    if(data & 0x04) LCD_DATA2_PORT |= _BV(LCD_DATA2_PIN);
    if(data & 0x02) LCD_DATA1_PORT |= _BV(LCD_DATA1_PIN);
    if(data & 0x01) LCD_DATA0_PORT |= _BV(LCD_DATA0_PIN);
    lcd_e_toggle();

	_delay_us(55);
}


/*************************************************************************
Move cursor to the start of next line or to the first line if the cursor
is already on the last line.
*************************************************************************/
static inline void lcd_newline(uint8_t pos)
{
    register uint8_t addressCounter;


#if LCD_LINES==1
    addressCounter = 0;
#endif
#if LCD_LINES==2
    if ( pos < (LCD_START_LINE2) )
        addressCounter = LCD_START_LINE2;
    else
        addressCounter = LCD_START_LINE1;
#endif
#if LCD_LINES==4
    if ( pos < LCD_START_LINE3 )
        addressCounter = LCD_START_LINE2;
    else if ( (pos >= LCD_START_LINE2) && (pos < LCD_START_LINE4) )
        addressCounter = LCD_START_LINE3;
    else if ( (pos >= LCD_START_LINE3) && (pos < LCD_START_LINE2) )
        addressCounter = LCD_START_LINE4;
    else
        addressCounter = LCD_START_LINE1;
#endif
    lcd_command((1<<LCD_DDRAM)+addressCounter);

}/* lcd_newline */


/*
** PUBLIC FUNCTIONS
*/

/*************************************************************************
Send LCD controller instruction command
Input:   instruction to send to LCD controller, see HD44780 data sheet
Returns: none
*************************************************************************/
void lcd_command(uint8_t cmd)
{
    lcd_write(cmd,0);
	_delay_ms(2);
}/* lcd_command */


/*************************************************************************
Send data byte to LCD controller
Input:   data to send to LCD controller, see HD44780 data sheet
Returns: none
*************************************************************************/
void lcd_data(uint8_t data)
{
    lcd_write(data,1);
	_delay_ms(2);
}/* lcd_data */


/*************************************************************************
Set cursor to specified position
Input:    x  horizontal position  (0: left most position)
          y  vertical position    (0: first line)
Returns:  none
*************************************************************************/
void lcd_gotoxy(uint8_t x, uint8_t y)
{
#if LCD_LINES==1
    lcd_command((1<<LCD_DDRAM)+LCD_START_LINE1+x);
#endif
#if LCD_LINES==2
    if ( y==0 )
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE1+x);
    else
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE2+x);
#endif
#if LCD_LINES==4
    if ( y==0 )
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE1+x);
    else if ( y==1)
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE2+x);
    else if ( y==2)
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE3+x);
    else /* y==3 */
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE4+x);
#endif

}/* lcd_gotoxy */


/*************************************************************************
Clear display and set cursor to home position
*************************************************************************/
void lcd_clrscr(void)
{
    lcd_command(1<<LCD_CLR);
}/* lcd_clrscr */


/*************************************************************************
Set cursor to home position
*************************************************************************/
void lcd_home(void)
{
    lcd_command(1<<LCD_HOME);
}/* lcd_home */


/*************************************************************************
Display character at current cursor position
Input:    character to be displayed
Returns:  none
*************************************************************************/
void lcd_putc(char c)
{
	lcd_write(c, 1);
}/* lcd_putc */


/*************************************************************************
Display string without auto linefeed
Input:    string to be displayed
Returns:  none
*************************************************************************/
void lcd_puts(const char *s)
/* print string on lcd (no auto linefeed) */
{
    register char c;

    while ( (c = *s++) ) {
        lcd_putc(c);
    }

}/* lcd_puts */


/*************************************************************************
Display string from program memory without auto linefeed
Input:     string from program memory be be displayed
Returns:   none
*************************************************************************/
void lcd_puts_p(const char *progmem_s)
/* print string from program memory on lcd (no auto linefeed) */
{
    register char c;

    while ( (c = pgm_read_byte(progmem_s++)) ) {
        lcd_putc(c);
    }

}/* lcd_puts_p */


/*************************************************************************
Initialize display and select type of cursor
Input:    dispAttr LCD_DISP_OFF            display off
                   LCD_DISP_ON             display on, cursor off
                   LCD_DISP_ON_CURSOR      display on, cursor on
                   LCD_DISP_CURSOR_BLINK   display on, cursor on flashing
Returns:  none
*************************************************************************/
void lcd_init( uint8_t dispAttr )
{
	/* Configure all LCD port bits as output */
	DDR(LCD_DATA0_PORT) |= _BV(LCD_DATA0_PIN);
	DDR(LCD_DATA1_PORT) |= _BV(LCD_DATA1_PIN);
	DDR(LCD_DATA2_PORT) |= _BV(LCD_DATA2_PIN);
	DDR(LCD_DATA3_PORT) |= _BV(LCD_DATA3_PIN);
	DDR(LCD_RS_PORT)	|= _BV(LCD_RS_PIN);
	DDR(LCD_E_PORT)		|= _BV(LCD_E_PIN);

	lcd_e_low();
	lcd_rs_low();

	_delay_ms(100);
	lcd_command(0x30);
	_delay_ms(4.1);
	lcd_command(0x30);
	_delay_us(100);
	lcd_command(0x30);
	_delay_us(100);
	lcd_command(0x20);						// 4 bit mode
	_delay_us(100);

	lcd_command(LCD_FUNCTION_DEFAULT);		//function set: display lines
	lcd_command(LCD_DISP_OFF);              //display off
	lcd_clrscr();                           //display clear
	lcd_command(LCD_MODE_DEFAULT);          //set entry mode
	lcd_command(dispAttr);                  //display/cursor control
}/* lcd_init */
