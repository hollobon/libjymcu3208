F_CPU = 8000000
ARCH = AVR8
MCU = atmega8

VPATH = include

TARGET = jymcu3208

SRC = src/mq.c src/timers.c src/ht1632c.c src/io.c

include rules.mk
