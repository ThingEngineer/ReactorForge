'*******************************************************************************
'        Induction Heater Control - Resonant PLL control w/current limit
'        Ver 1.0
'        07/2012
'****************************** Chip Settings **********************************
$regfile = "m48def.dat"
$crystal = 10000000
$baud = 9600
'
'****************************** Port Settup ************************************
Config Portb = &B00100011                                   ' Config I/O ports 1=output (PORTB.1 is PWM output OC1A)
Config Portc = Input
Config Portd = &B00000000
Didr0 = &B00111111                                          ' Disable the digital input buffer on all ADC ports (PORTC)

Vco_enable Alias Portb.0                                    ' VCO enable/disable - Active high (Low disables)

Sw1 Alias Pind.7                                            ' Switch ports (Active low)
Sw2 Alias Pind.6
Sw3 Alias Pind.5
Sw4 Alias Pind.4
Sw5 Alias Pind.3
Set Portd.7                                                 ' Set pull-ups
Set Portd.6
Set Portd.5
Set Portd.4
Set Portd.3

Led Alias Portb.5
'
'********************************** LCD Setup **********************************
'Config Lcd = 16 * 2
'Config Lcdpin = Pin , Db4 = Portd.7 , Db5 = Portb.2 , Db6 = Portb.3 , Db7 = Portb.4 , E = Portd.6 , Rs = Portd.5
'
'****************************** Program Variables ******************************
Dim Desired_phase As Word                                   ' Desired phase offset
Dim Phase(7) As Word                                        ' Phase buffer
Dim Phase_avg As Long                                       ' Average phase reading

Dim Desired_current As Word                                 ' Desired current value
Dim Current(7) As Word                                      ' Current buffer
Dim Current_avg As Long                                     ' Average current
Dim Current_diff As Word                                    ' Delta between average and desired current

Dim Running_flag As Bit                                     ' 1=VCO is enabled and IH is running, 0=VCO is disabled and IH is in standby
Dim Loop_count As Byte                                      ' Temporary loop counter
Dim Samples As Byte                                         ' Sample counter
Dim Display_delay As Word                                   ' Display delay counter
'
'******************************** Interrupts ***********************************
Config Adc = Single , Prescaler = Auto , Reference = Off

Config Timer1 = Pwm , Pwm = 10 , Compare A Pwm = Clear Up , Compare B Pwm = Disconnect , Prescale = 1
Pwm1a = 1023                                                ' 0 = 0% duty cycle (off) : 1023 = 100% duty cycle on (Start at max VCO Fo, pwm=1023) - Output on PORTB.1

'Config Timer0 = Timer , Prescale = 8
'On Ovf0 Tmr0_isr
'Enable Timer0

'Config Int0 = Low Level
'On Int0 Int0_isr
'Enable Int0

Enable Interrupts
'
'******************************* Main Program **********************************
Reset Vco_enable                                            ' Ensure VCO is disabled
Print "I2Reactor Ready"
Desired_phase = 650                                         ' ORG=630
Desired_current = 120
Phase_avg = 0
Current_avg = 0
Display_delay = 0
Samples = 1
Running_flag = 0
Set Led                                                     ' LED off

Do
   Debounce Sw1 , 0 , Switch1 , Sub                         ' Start inverter
   Debounce Sw2 , 0 , Switch2 , Sub                         ' Incremnt desired current by 10
   Debounce Sw3 , 0 , Switch3 , Sub                         ' Decrement desired current by 10
   Debounce Sw4 , 0 , Switch4 , Sub                         ' Incremnt desired phase shift adc reading by 10
   Debounce Sw5 , 0 , Switch5 , Sub                         ' Decrement desired phase shift adc reading by 10

   If Running_flag = 1 Then

      Phase(samples) = Getadc(4)                            ' Read inverter phase shift value, PLL phase comp I output (trying to lock on 90°)
      Current(samples) = Getadc(5)                          ' Read mains current value

      Incr Samples
      If Samples = 7 Then

         For Loop_count = 1 To 7
            Phase_avg = Phase(loop_count) + Phase_avg
            Current_avg = Current(loop_count) + Current_avg
         Next
         Phase_avg = Phase_avg / 8
         Current_avg = Current_avg / 8

         If Phase_avg > Desired_phase Then
            If Current_avg > Desired_current Then
               Gosub Incr_pwm
               Current_diff = Current_avg - Desired_current
               If Current_diff > 100 Then
                  If Pwm1a < 1010 Then Pwm1a = Pwm1a + 10
                  Waitus 200
               End If
            Else
               Decr Pwm1a
            End If
         Else
            Gosub Incr_pwm
         End If

         Samples = 1
      End If

      Incr Display_delay
      If Display_delay = 1024 Then
         Toggle Led
         Print "P=" ; Phase_avg ; "    PWM=" ; Pwm1a ; "    I=" ; Current_avg
         Display_delay = 0
      End If

      If Sw1 = 1 Then Gosub Switch1                         ' Stop inverter
   End If

Loop

Incr_pwm:
   If Pwm1a <> 1023 Then                                    ' Increment but do not allow a roll over to zero (Min Fo will destroy the inverter at full power)
      Incr Pwm1a
   Else
      Decr Pwm1a
   End If
   Return

Switch1:
   Toggle Running_flag

   If Running_flag = 0 Then                                 ' 0 = Stop inverter
      Do                                                    ' Ramp up frequency for soft stop
         Incr Pwm1a
         Waitus 500
      Loop Until Pwm1a >= 1023

      Reset Vco_enable                                      ' Disable PLL VCO
      Set Led                                               ' LED off
      Print "PLL VCO Off"
   Else                                                     ' 1 = Start inverter
      Pwm1a = 1023                                          ' Ensure a start from Fo max
      Set Vco_enable                                        ' Enable PLL VCO
      Print "PLL VCO On"
   End If
Return

Switch2:                                                    ' Incremnt desired current by 10
   Desired_current = Desired_current + 5
   Print "Des Current=" ; Desired_current
Return

Switch3:                                                    ' Decrement desired current by 10
   If Desired_current <> 80 Then Desired_current = Desired_current - 5
   Print "Des Current=" ; Desired_current
Return

Switch4:                                                    ' Incremnt desired phase shift adc reading by 10
   Desired_phase = Desired_phase + 10
   Print "Des Phase=" ; Desired_phase
Return

Switch5:                                                    ' Decrement desired phase shift adc reading by 10
   Desired_phase = Desired_phase - 10
   Print "Des Phase=" ; Desired_phase
Return
'
'******************************** Interrupt Routines ***************************