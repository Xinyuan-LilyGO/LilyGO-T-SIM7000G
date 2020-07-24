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
GSM_PWR.value(1)
time.sleep_ms(300)
GSM_PWR.value(0)


LED = machine.Pin(12, machine.Pin.OUT)
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

print("Connecting to GSM...")
gsm.connect()

while gsm.status()[0] != 1:
    pass

print('IP:', gsm.ifconfig()[0])


print("Connected !")
addr = socket.getaddrinfo('loboris.eu', 80)[0][-1]
s = socket.socket()
s.connect(addr)
s.send(b'GET /ESP32/info.txt HTTP/1.1\r\nHost: loboris.eu\r\n\r\n')
data = s.recv(1000)
print('data=', end='')
print(data)
s.close()

# GSM connection is complete.
# You can now use modules like urequests, uPing, etc.
# Let's try socket API:
