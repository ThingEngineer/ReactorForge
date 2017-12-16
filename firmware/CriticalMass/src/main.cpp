#include <avr/io.h>
#include <util/delay.h>

void setup() {
  //Set port PORTD data direction registers to output
  DDRD = 0xFF;
}

int main(void)
{
  setup();

  while(1)
  {
    // Set all of PORTD to high
    PORTD = 0xFF;
    //1 second delay
    _delay_ms(1000);
    // Set all of PORTD to low
    PORTD= 0x00;
    //1 second delay
    _delay_ms(1000);
  }
}
