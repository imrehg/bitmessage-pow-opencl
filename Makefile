CCFLAGS = -g -std=c99 -fPIC -shared -Wall

UNAME := $(shell uname -r)
ifneq (, $(findstring parallella, $(UNAME)))
  INCS += -I/usr/local/browndeer/include
  LIBS += -L/usr/local/browndeer/lib -locl
else
  INCS += -I/usr/include/CL
  LIBS += -lOpenCL
endif

binaries = bmpow.so

all: $(binaries)

bmpow.so: bmpow.c
	$(CC) $(CCFLAGS) -o $@ $^ $(INCS) $(LIBS)

clean:
	rm $(binaries)

.PHONY: $(binaries)
