'''

- Make sure the GPS antenna is firmly connected
- For more documentation on the GPS module, please check the manual

'''

import socket
import machine
import time
import sys
import gsm

# APN credentials (replace with yours)
GSM_APN = ''  # Your APN
GSM_USER = ''  # Your User
GSM_PASS = ''  # Your Pass

# Power on the GSM module
GSM_PWR = machine.Pin(4, machine.Pin.OUT)
LED = machine.Pin(12, machine.Pin.INOUT)

GSM_PWR.value(1)
time.sleep_ms(300)
GSM_PWR.value(0)
LED.value(1)

# Init PPPoS
# gsm.debug(True)  # Uncomment this to see more logs, investigate issues, etc.

gsm.start(tx=27, rx=26, apn=GSM_APN, user=GSM_USER, password=GSM_PASS)

sys.stdout.write('Waiting for AT command response...')
for retry in range(20):
    if gsm.atcmd('AT'):
        break
    else:
        sys.stdout.write('.')
        time.sleep_ms(5000)
else:
    raise Exception("Modem not responding!")
print()

# Turn on GPS positioning
gsm.atcmd('AT+CGNSPWR=1', printable=True)
# Turn on GPS power control, it is controlled by module GPIO4
gsm.atcmd('AT+SGPIO=0,4,1,1', printable=True)

while True:
    # Get positioning data
    raw = gsm.atcmd('AT+CGNSINF', printable=True)
    # Output information
    print(raw)
    string = raw.split(',')
    if(int(string[1]) == 1):
        print('{yy}/{mm}/{dd} == {h}:{m}:{s} == lat:{lat}-lon:{lon}'.format(
            yy=string[2][:4], mm=string[2][4:6], dd=string[2][6:8], h=string[2][8:10], m=string[2][10:12], s=string[2][12:], lat=string[3], lon=string[4]))
        break

    # Let the LED flash when in the positioning state
    LED.value(not LED.value())
    time.sleep_ms(1000)

# Turn off GPS positioning
gsm.atcmd('AT+CGNSPWR=0', printable=True)
# Turn off GPS power control, it is controlled by module GPIO4
gsm.atcmd('AT+SGPIO=0,4,1,0', printable=True)
