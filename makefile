all: build/gpio

install: build/gpio
	cp -f build/gpio /usr/local/bin/gpio

build/gpio: build/gpio.o build/ArtNetSender.o
	gcc -o build/gpio build/gpio.o build/ArtNetSender.o

build/gpio.o: gpio.c
	mkdir -p build
	gcc -std=c11 -c gpio.c -o build/gpio.o

build/ArtNetSender.o: ArtNetSender.c
	mkdir -p build
	gcc -std=c11 -c ArtNetSender.c -o build/ArtNetSender.o

clean:
	rm build/*;
