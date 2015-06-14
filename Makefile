CXXFLAGS = -g -O3 -fPIC -shared -Wall
LDFLAGS = -I/usr/include/CL -lOpenCL

binaries = bmpow.so

all: $(binaries)

bmpow.so: bmpow.cc
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm $(binaries)

.PHONY: $(binaries)
