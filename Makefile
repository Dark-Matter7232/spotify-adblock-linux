CC=gcc
CFLAGS=-march=native -mtune=native -flto -O3 -Wall -s -dynamiclib -fPIC -I.
LDLIBS=-ldl -lcurl
TARGET=spotify-adblock-macos

.PHONY: all
all: $(TARGET).dylib

$(TARGET).dylib: $(TARGET).c whitelist.h blacklist.h
	$(CC) $(CFLAGS) -o $@ $(LDLIBS) $^

.PHONY: clean
clean:
	rm -f $(TARGET).dylib 