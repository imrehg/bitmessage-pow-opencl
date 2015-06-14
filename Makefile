CXXFLAGS = -g -fPIC -shared -Wall
LDFLAGS = -I/usr/include/CL -lOpenCL -lcrypto

binaries = bmpow.so

all: $(binaries)

bmpow.so: bmpow.cc
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm $(binaries)

.PHONY: $(binaries)
