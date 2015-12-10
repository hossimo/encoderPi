CC=gcc
FLAGS=-std=c11
INSTALLPATH=/usr/local/bin/
EXECUTABLE=encoder2artnet

all: build/$(EXECUTABLE)

install: build/$(EXECUTABLE)
	cp -f build/$(EXECUTABLE) $(INSTALLPATH)$(EXECUTABLE)

build/$(EXECUTABLE): build/encoder2artnet.o build/artnetsender.o
	$(CC) -o build/$(EXECUTABLE) build/encoder2artnet.o build/artnetsender.o

build/encoder2artnet.o: encoder2artnet.c
	mkdir -p build
	$(CC) $(FLAGS) -c encoder2artnet.c -o build/encoder2artnet.o -lpthread

build/artnetsender.o: artnetsender.c
	mkdir -p build
	$(CC) $(FLAGS) -c artnetsender.c -o build/artnetsender.o

clean:
	rm build/*;
