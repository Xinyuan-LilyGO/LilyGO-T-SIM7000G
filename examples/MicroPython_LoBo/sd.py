import os
import uos
import machine

# The SD SPI pin of the firmware compiled by lobo is not the pin used by T-SIM7000,
# so here we use uos to initialize the SD card
uos.sdconfig(uos.SDMODE_SPI, clk=14, mosi=15, miso=2, cs=13, maxspeed=16)
os.mountsd()
os.listdir('/sd')
