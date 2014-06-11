'*******************************************************************************
'        I²Reactor
'        Induction Heater Control - Resonant PLL Fo lock w/current limit
'        Ver 1.0
'        07/2012
'****************************** Chip Settings **********************************
'$regfile = "m168def.dat"
$regfile = "4433def.dat"
$crystal = 10000000
'$baud = 9600
'$prog &HFF , &HF7 , &HCC , &HF9                             ' Fuse bytes
'
'****************************** Port Settup ************************************
Config Portb = &B00111111                                   ' Config I/O ports 1=output (PORTB.1 is PWM output OC1A)
Config Portc = Input
Config Portd = &B11110000
'Didr0 = &B00111111                                          ' Disable the digital input buffer on all ADC ports (PORTC)

Vco_enable Alias Portb.0                                    ' VCO enable/disable - Active high (Low disables)
Reset Vco_enable                                            ' Ensure VCO is disabled

Sw1 Alias Pinc.2                                            ' Switch ports (Active low)
Sw2 Alias Pinc.3
Sw3 Alias Pinc.4
Sw4 Alias Pinc.5
Sw5 Alias Pind.1
Set Portc.2                                                 ' Set pull-ups
Set Portc.3
Set Portc.4
Set Portc.5
Set Portd.1

Led Alias Portd.4
'
'********************************** LCD Setup **********************************
Config Lcdpin = Pin , Db4 = Portd.7 , Db5 = Portb.2 , Db6 = Portb.3 , Db7 = Portb.4 , E = Portd.6 , Rs = Portd.5
Config Lcd = 16 * 1a
Initlcd

Deflcdchar [0] , 24 , 4 , 8 , 28 , 32 , 32 , 32 , 32        ' Squared ^2
'Deflcdchar 1 , 4 , 4 , 4 , 4 , 21 , 14 , 4 , 32             ' Down arrow
'Deflcdchar 2 , 32 , 4 , 2 , 31 , 2 , 4 , 32 , 32            ' Holding arrow ->
'Deflcdchar 3 , 4 , 14 , 21 , 4 , 4 , 4 , 4 , 32             ' Up arrow
Cursor Off Noblink
Cls
'
'****************************** Program Variables ******************************
Dim Desired_phase As Word                                   ' Desired phase offset
Dim Phase(5) As Word                                        ' Phase buffer    ' Reduced from 25 to 5 for 4433, was working really well on 25
Dim Phase_avg As Long                                       ' Running average phase reading

'Dim Desired_current As Word                                 ' Desired current value
'Dim Current(7) As Word                                      ' Current buffer
'Dim Current_avg As Long                                     ' Running average current
'Dim Current_diff As Word                                    ' Delta between average and desired current
Dim Over_current As Bit                                     ' Over current flag 1=Over current limit
Dim Current_offset As Word

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

Config Int0 = Low Level                                     ' Over current trigger
On Int0 Int0_isr
Enable Int0

Enable Interrupts
'
'******************************* Main Program **********************************
Lcd "I" + Chr(0) + "Reactor Ready"
Desired_phase = 670                                         ' ORG=630
'Desired_current = 200
Phase_avg = 0
'Current_avg = 0

Over_current = 0
Current_offset = 0

Display_delay = 0
Samples = 1
Running_flag = 0
Reset Led                                                   ' LED off

Do
   Debounce Sw1 , 0 , Switch1 , Sub                         ' Start inverter
   Debounce Sw2 , 0 , Switch2 , Sub                         ' Incremnt desired current
   Debounce Sw3 , 0 , Switch3 , Sub                         ' Decrement desired current
   Debounce Sw4 , 0 , Switch4 , Sub                         ' Incremnt desired phase shift
   Debounce Sw5 , 0 , Switch5 , Sub                         ' Decrement desired phase shift

   If Running_flag = 1 Then

      Phase(samples) = Getadc(6)                            ' Read inverter phase shift value, PLL phase comp I output (trying to lock on 90°)
      'Current(samples) = Getadc(7)                          ' Read mains current value
      ' move current adc inside buffer loop maybe
      Incr Samples
      If Samples = 5 Then

         For Loop_count = 1 To 5
            Phase_avg = Phase(loop_count) + Phase_avg
            'Current_avg = Current(loop_count) + Current_avg
         Next
         Phase_avg = Phase_avg / 6
         'Current_avg = Current_avg / 8

         Phase_avg = Phase_avg - Current_offset

         If Phase_avg > Desired_phase Then
            'If Over_current = 1 Then
            '   Gosub Incr_pwm

            '   Over_current = 0
            '   Reset Led
               'Enable Int0
            'Else
               Decr Pwm1a
            'End If

            'If Current_avg > Desired_current Then
            '   Gosub Incr_pwm
            '   Current_diff = Current_avg - Desired_current
            '   If Current_diff > 100 Then
            '      If Pwm1a < 1010 Then Pwm1a = Pwm1a + 10
            '      Waitus 200
            '   End If
            'Else
            '   Decr Pwm1a
            'End If
         Else
            Gosub Incr_pwm
         End If


         Samples = 1

      End If

      Incr Display_delay
      If Display_delay = 2048 Then
         'Toggle Led
         Cls : Lcd "P=" + Phase_avg + " V" + Pwm1a          ' + " I=" + Current_avg
         'Print "P=" ; Phase_avg ; "    PWM=" ; Pwm1a ; "    I=" ; Current_avg
         Display_delay = 0
      End If

      If Sw1 = 1 Then Gosub Switch1                         ' Stop inverter
   End If

Loop

Incr_pwm:
   'If Pwm1a does not equal 1023 then allow increment
   If Pwm1a <> 1023 Then                                    ' Increment but do not allow a roll over to zero (Min Fo will destroy the inverter at full power)
      Incr Pwm1a
   Else
      Decr Pwm1a
   End If

   If Current_offset > 0 Then
      Decr Current_offset
   Else
      Reset Led
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
      'Reset Led                                             ' LED off
      Cls : Lcd "PLL VCO Off"
      Wait 1
      Cls : Lcd "I" + Chr(0) + "Reactor Ready"
   Else                                                     ' 1 = Start inverter
      Pwm1a = 1023                                          ' Ensure a start from Fo max
      Set Vco_enable                                        ' Enable PLL VCO
      Cls : Lcd "PLL VCO On"
   End If
Return

Switch2:                                                    ' Incremnt desired current
   'Desired_current = Desired_current + 5
   'Cls : Lcd "Desired I=" ; Desired_current
Return

Switch3:                                                    ' Decrement desired current
   'If Desired_current <> 80 Then Desired_current = Desired_current - 5
   'Cls : Lcd "Desired I=" ; Desired_current
Return

Switch4:                                                    ' Incremnt desired phase shift adc reading
   Desired_phase = Desired_phase + 10
   Cls : Lcd "Desired Phase=" ; Desired_phase
Return

Switch5:                                                    ' Decrement desired phase shift adc reading
   Desired_phase = Desired_phase - 10
   Cls : Lcd "Desired Phase=" ; Desired_phase
Return
'
'******************************** Interrupt Routines ***************************
Int0_isr:

   'Disable Int0

   'Do
   '   Gosub Incr_pwm
   'Loop Until Pind.2 = 1

   Set Led
   'Over_current = 1
   Incr Current_offset
Return