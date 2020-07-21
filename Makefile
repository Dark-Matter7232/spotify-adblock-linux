CC=gcc-9
CFLAGS=-Wall -dynamiclib -fPIC
LDLIBS=-ldl -lcurl
TARGET=spotadblock-mac


.PHONY: all
all: $(TARGET).dylib

$(TARGET).dylib: $(TARGET).c whitelist.h blacklist.h
	$(CC) $(CFLAGS) -o $@ $(LDLIBS) $^

.PHONY: clean
	rm -f $(TARGET).dylib 