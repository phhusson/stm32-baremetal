PREFIX=./toolchain/bin/arm-none-linux-gnueabihf-
CC=$(PREFIX)gcc
LD=$(PREFIX)ld

CFLAGS=-ffreestanding -Iinclude -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

all:
	$(CC) $(CFLAGS) -g crt0.s -c -o crt0.o
	$(CC) $(CFLAGS) -g startup_stm32f40xx.s -c -o startup_stm32f40xx.o
	$(CC) $(CFLAGS) -g init.c -c -o init.o
	$(LD) crt0.o startup_stm32f40xx.o init.o -Tram.lds -o example.ram
	$(LD) crt0.o startup_stm32f40xx.o init.o -Tram.lds -o example.flash

clean:
	rm -f *.o example.ram example.flash
