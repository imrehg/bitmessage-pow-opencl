CCFLAGS = -g -O3 -fPIC -shared -Wall
LDFLAGS = -I/usr/include/CL -lOpenCL

binaries = bmpow.so

all: $(binaries)

bmpow.so: bmpow.c
	$(CC) $(CCFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm $(binaries)

.PHONY: $(binaries)
