#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif
#include <sys/mman.h>
#include <malloc.h>

#define MEM_ALIGNMENT        4096
//#define MEM_ALIGNMENT        2*1024*1024
static inline void* alloc(size_t size)
{
  void* p = memalign(MEM_ALIGNMENT, size);
  //madvise(p, size, MADV_NOHUGEPAGE);
  return p;
}

