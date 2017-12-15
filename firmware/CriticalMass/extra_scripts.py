Import('env')

#Override fuses set in board manifest
#http://eleccelerator.com/fusecalc/fusecalc.php?chip=at90pwm316&LOW=F3&HIGH=DE&EXTENDED=C9&LOCKBIT=FF
env.Replace(FUSESCMD="avrdude $UPLOADERFLAGS -e -Ulfuse:w:0xF3:m -Uhfuse:w:0xDE:m -Uefuse:w:0xC9:m -Ulock:w:0xFF:m")
