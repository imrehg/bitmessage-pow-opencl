CCFLAGS = -g -std=c99 -fPIC -shared -Wall

# Intel CPU
#INCS += -I/usr/include/CL
#LIBS += -lOpenCL

INCS += -I/usr/local/browndeer/include
LIBS += -L/usr/local/browndeer/lib -locl -lcoprthr_opencl


binaries = bmpow.so

all: $(binaries)

bmpow.so: bmpow.c
	$(CC) $(CCFLAGS) -o $@ $^ $(INCS) $(LIBS)

clean:
	rm $(binaries)

.PHONY: $(binaries)
