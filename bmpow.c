#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include <CL/cl.h>

#include <limits.h>

#include "bmpow.h"

#define DEVICE_TYPE CL_DEVICE_TYPE_ALL
#define OPENCL_KERNEL_FILE "bitmessage_pow_kernel.cl"

#define PLAINTEXT_LENGTH 72

/* https://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/scalarDataTypes.html */
#define uint8_t cl_uchar
#define uint32_t cl_uint
#define uint64_t cl_ulong

typedef struct {
    uint64_t target;
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

uint64_t proofOfWork(uint64_t target, char* string) {
  /* From https://code.google.com/p/simple-opencl/ */
  char build_c[4096];
  size_t srcsize;
  size_t worksize = (2 << 3);
  cl_int error;
  cl_platform_id platform;
  cl_device_id device;
  cl_uint platforms, devices;

  /* Fetch the Platforms, we only want one. */
  error=clGetPlatformIDs(1, &platform, &platforms);
  if (error != CL_SUCCESS) {
    printf("\n Error number %d", error);
  }
  /* Fetch the Devices for this platform */
  error=clGetDeviceIDs(platform, DEVICE_TYPE, 1, &device, &devices);
  if (error != CL_SUCCESS) {
    printf("\n Error number %d", error);
  }

  /* Create a memory context for the device we want to use  */
  cl_context_properties properties[]={CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0};
  cl_context context=clCreateContext(properties, 1, &device, NULL, NULL, &error);
  if (error != CL_SUCCESS) {
    printf("\n Error number %d", error);
  }
  /* Create a command queue to communicate with the device */
  cl_command_queue cq = clCreateCommandQueue(context, device, 0, &error);
  if (error != CL_SUCCESS) {
    printf("\n Error number %d", error);
  }

  /* Read the source kernel code in exmaple.cl as an array of char's */
  char src[8192];
  FILE *fil = fopen(OPENCL_KERNEL_FILE,"r");
  srcsize = fread(src, sizeof src, 1, fil);
  fclose(fil);

  const char *srcptr[]={src};
  /* Submit the source code of the kernel to OpenCL, and create a program object with it */
  cl_program prog=clCreateProgramWithSource(context,
					    1, srcptr, &srcsize, &error);
  if (error != CL_SUCCESS) {
    printf("\n Error number %d", error);
  }

  /* Compile the kernel code (after this we could extract the compiled version) */
  error=clBuildProgram(prog, 0, NULL, "", NULL, NULL);
  if ( error != CL_SUCCESS ) {
    printf( "Error on buildProgram " );
    printf("\n Error number %d", error);
    fprintf( stdout, "\nRequestingInfo\n" );
    clGetProgramBuildInfo( prog, device, CL_PROGRAM_BUILD_LOG, 4096, build_c, NULL );
    printf( "Build Log for %s_program:\n%s\n", "example", build_c );
  }

  sha512_key hash;
  hash.target = target;
  for (int i = 0; i < 8; i++) {
    hash.v[i] = 0;
  }
  for (int i = 0; i < 64; i++) {
    hash.v[i+8] = string[i];
  }

  cl_mem hash_buf, dest_buf;
  hash_buf = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(sha512_key), &hash, &error);
  if (error != CL_SUCCESS) {
    printf("\n Error number %d", error);
  }
  dest_buf = clCreateBuffer(context, CL_MEM_WRITE_ONLY,  sizeof(uint64_t), NULL, &error);
  if (error != CL_SUCCESS) {
    printf("\n Error number %d", error);
  }

  /* Create a kernel object with the compiled program */
  cl_kernel k_example=clCreateKernel(prog, "bitmessage_pow", &error);
  if (error != CL_SUCCESS) {
    printf("\n Error number %d", error);
  }

  /* Set the kernel parameters */
  error = clSetKernelArg(k_example, 0, sizeof(hash_buf), &hash_buf);
  if (error != CL_SUCCESS) {
    printf("\n Error number %d", error);
  }
  error = clSetKernelArg(k_example, 1, sizeof(dest_buf), &dest_buf);
  if (error != CL_SUCCESS) {
    printf("\n Error number %d", error);
  }

  uint64_t startpos = 0;
  size_t globamt = worksize * (2 << 12);

  uint64_t output = 0;

  do {
    error = clSetKernelArg(k_example, 2, sizeof(startpos), &startpos);
    if (error != CL_SUCCESS) {
      printf("\n Error number %d", error);
    }

    /* Tell the Device, through the command queue, to execute que Kernel */
    error=clEnqueueNDRangeKernel(cq, k_example, 1, NULL, &globamt, &worksize, 0, NULL, NULL);
    if (error != CL_SUCCESS) {
      printf("\n Error number %d", error);
    }

    /* Read the result back into buf2 */
    error=clEnqueueReadBuffer(cq, dest_buf, CL_FALSE, 0, sizeof(output), &output, 0, NULL, NULL);
    if (error != CL_SUCCESS) {
      printf("\n Error number %d", error);
    }

    error=clFinish(cq);
    if (error != CL_SUCCESS) {
      printf("\n Error number %d", error);
    }

    startpos += (uint64_t)globamt;
  } while (output == 0);

  return output;
}
