// #include <stdlib.h>
// #include <stdio.h>

// #include <string.h>



#define __NO_STD_VECTOR // Use cl::vector instead of STL version
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <utility>
#include <iostream>
#include <fstream>
#include <string>
#include <climits>
using namespace cl;

#include "bmpow.hh"

#define DEVICE_TYPE CL_DEVICE_TYPE_ALL

#define PLAINTEXT_LENGTH 72

#define uint64_t unsigned long long

typedef struct {
    unsigned long long target;
    char v[PLAINTEXT_LENGTH+1];
} sha512_key;


// convert ulonglong to big endian
// http://stackoverflow.com/a/27770821/171237
void write64be(char out[8], unsigned long long in)
{
    out[0] = in >> 56 & 0xff;
    out[1] = in >> 48 & 0xff;
    out[2] = in >> 40 & 0xff;
    out[3] = in >> 32 & 0xff;
    out[4] = in >> 24 & 0xff;
    out[5] = in >> 16 & 0xff;
    out[6] = in >>  8 & 0xff;
    out[7] = in >>  0 & 0xff;
}

// Digest to trial value
unsigned long long trialFromDigest(unsigned char digest[])
{
  unsigned long long trial = 0;
  trial = ((unsigned long long)digest[0] << 56) +
          ((unsigned long long)digest[1] << 48) +
          ((unsigned long long)digest[2] << 40) +
          ((unsigned long long)digest[3] << 32) +
          ((unsigned long long)digest[4] << 24) +
          ((unsigned long long)digest[5] << 16) +
          ((unsigned long long)digest[6] <<  8) +
          ((unsigned long long)digest[7]);
  return trial;
}


unsigned long long pow(unsigned long long target, char* string) {
  // unsigned char digest[SHA512_DIGEST_LENGTH];
  // unsigned long long nonce = 0;
  // char noncebe[8];
  // char str[72];
  // SHA512_CTX ctx;
  // unsigned long long trialValue = ULLONG_MAX;

  // while (trialValue > target) {
  //   nonce++;
  //   write64be(noncebe, nonce);
  //   memcpy(&str[0], noncebe, 8);
  //   memcpy(&str[8], string, 64);

  //   SHA512_Init(&ctx);
  //   SHA512_Update(&ctx, str, sizeof(str));
  //   SHA512_Final(digest, &ctx);

  //   SHA512_Init(&ctx);
  //   SHA512_Update(&ctx, digest, 64);
  //   SHA512_Final(digest, &ctx);

  //   // char mdString[SHA512_DIGEST_LENGTH*2+1];
  //   // for (int i = 0; i < SHA512_DIGEST_LENGTH; i++)
  //   //     sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);

  //   // printf("SHA512 digest: %s\n", mdString);

  //   trialValue = trialFromDigest(digest);
  //   // char correct[8];
  //   // write64be(correct, (unsigned long long)result);
  // }
  // return nonce;

  try {
    // Get available platforms
    vector<Platform> platforms;
    Platform::get(&platforms);
 
    // Select the default platform and create a context using this platform and the GPU
    cl_context_properties cps[3] = { 
      CL_CONTEXT_PLATFORM, 
      (cl_context_properties)(platforms[0])(), 
      0 
    };
    Context context( DEVICE_TYPE, cps);

    // Get a list of devices on this platform
    vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

    // Create a command queue and use the first device
    CommandQueue queue = CommandQueue(context, devices[0]);
    
    // Read source file
    std::ifstream sourceFile("bitmessage_pow_kernel.cl");
    std::string sourceCode(std::istreambuf_iterator<char>(sourceFile),
			   (std::istreambuf_iterator<char>()));
    Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length()+1));

    // Make program of the source code in the context
    Program program = Program(context, source);
    
    // Build program for these specific devices
    program.build(devices);
 
    // Make kernel
    Kernel kernel(program, "bitmessage_pow");

    sha512_key hash;
    hash.target = target;
    for (int i = 0; i < 8; i++) {
      hash.v[i] = 0;
    }
    for (int i = 0; i < 64; i++) {
      hash.v[i+8] = string[i];
    }

    // uint64_t junk[9];
    // uint64_t * s = (uint64_t*) hash.v;
    // for (int i = 1; i < 9; i++) {
    //   junk[i] = s[i];
    //   printf("Part %d: %llu\n", i, junk[i]);
    // }

    
    int ret;
    Buffer hash_buf = Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(sha512_key), &hash, &ret);
    Buffer dest_buf = Buffer(context, CL_MEM_WRITE_ONLY, sizeof(unsigned long long));

    // Set arguments to kernel
    kernel.setArg(0, hash_buf);
    kernel.setArg(1, dest_buf);

    char noncebe[8];


    
    int worksize = 4;		// get this one from the workgroup info later
    unsigned long long startpos = 0;
    int globamt = worksize * 1000;

    unsigned long long *output;

    do {
      // write64be(noncebe, startpos);
      // kernel.setArg(2, (unsigned long long)noncebe);
      kernel.setArg(2, startpos);
      
      // Run the kernel on specific ND range
      NDRange global(globamt);
      NDRange local(worksize);
      queue.enqueueNDRangeKernel(kernel, NullRange, global, local);
      
      queue.enqueueReadBuffer(dest_buf, CL_TRUE, 0, sizeof(output), &output);
      
      startpos += globamt;
      // std::cout << startpos << std::endl;
      // std::cout << "Output: " << output << std::endl;
    } while (output == 0);
    std::cout << output << std::endl;
    printf("Bull's eye: %llu\n", (unsigned long long)output);
    return (unsigned long long)output;
  } catch(Error error) {
    std::cout << error.what() << "(" << error.err() << ")" << std::endl;
  }

  return 0;
}
