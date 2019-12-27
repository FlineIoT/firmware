# Pinout Board Fline Breakout
## nRF52
SWDIO: JTAG nRF
SWDCLK: JTAG nRF
RST: P0.21: JTAG nRF
P0.18: JTAG nRF

## WSG303S
P0.02: RX WSG (TX E73): Header haut droite
P0.03: TX WSG (RX E73): Header bas gauche

## ESP8266
P0.24: RX ESP8266
P0.23: TX ESP8266
P0.22: EN

## BME280
SCL: P0.04
SDA: P0.08
SDO: P0.05
CS: P0.07

## LIS2DH12
SCL: P0.04
SDA: P0.08
SDO: P0.05
CS: P0.06
IT1: P0.09

## Lipo Rider
P0.17: EN Battery

## LED Header
1: Vbat: Trou haut gauche coin
2: 3.3V
3: P0.26
4: P0.25
5: GND: Droite, fin du WSG/J

## Headers GPIO
1 5
2 6
3 7
4 8

1: 3.3V         5: P0.19
2: P0.12        6: P0.11
3: P0.13        7: P0.14
4: P0.10        8: GND