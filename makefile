VERBOSE=-v
CFLAGS=-D_POSIX_C_SOURCE=199309L -std=c11
INSTALLPATH=/usr/local/bin/
EXECUTABLE=encoder2artnet
C_SUCCESS=\033[0;32m
C_FAIL=\033[1;31m
C_NORMAL=\033[0m

all: build/$(EXECUTABLE)
	@echo "$(C_SUCCESS)** BUILD SUCCESS **$(C_NORMAL)"
	@echo

verbose: FLAGS += -v
verbose: build/$(EXECUTABLE)

install: build/$(EXECUTABLE)
	@echo
	@echo "$(C_SUCCESS)** Install Success **$(C_NORMAL)"
	cp -f build/$(EXECUTABLE) $(INSTALLPATH)$(EXECUTABLE)

build/$(EXECUTABLE): build/encoder2artnet.o build/artnetsender.o build/preferences.o
	@echo
	@echo "$(C_SUCCESS)** Starting build **$(C_NORMAL)"
	$(CC) $(CFLAGS) -pthread -o build/$(EXECUTABLE) build/encoder2artnet.o build/artnetsender.o build/preferences.o

build/encoder2artnet.o: encoder2artnet.c
	@echo
	@echo "$(C_SUCCESS)** Starting build/encoder2artnet.o **$(C_NORMAL)"
	mkdir -p build
	$(CC) $(CFLAGS) -c encoder2artnet.c -o build/encoder2artnet.o

build/artnetsender.o: artnetsender.c
	@echo
	@echo "$(C_SUCCESS)** Starting build/encoder2artnet.o **$(C_NORMAL)"
	mkdir -p build
	$(CC) $(CFLAGS) -c artnetsender.c -o build/artnetsender.o

build/preferences.o: preferences.c
	@echo
	@echo "$(C_SUCCESS)** starting build/preferences.o **$(C_NORMAL)"
	mkdir -p build
	$(CC) $(CFLAGS) -c preferences.c -o build/preferences.o

clean:
	@echo
	@echo "$(C_SUCCESS)** Starting clean **$(C_NORMAL)"
	rm -f build/*;
	rm -f *.h.gch
