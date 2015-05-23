#include <stdlib.h>
#include <string.h>

int main(int argc, const char* argv[])
{
  // Allocate two large contiguous chunks.
  char* p = malloc(1024 - 8);
  char* p2 = malloc(1024 - 8);
  // Set them up with easily recognizable values.
  memset(p, 'A', 1024 - 8);
  memset(p2, 'B', 1024 - 8);
  // Overflow the first chunk, off-by-one, with a NUL byte.
  p[1024 - 8] = '\0';
  // Free the second chunk.
  // Because of the overflow, the first chunk will incorrectly be seen as free.
  // The last 8 bytes of the first chunk, 0x4141414141414141, will be treated
  // as a size. This size is a back index to the first chunk's alleged chunk
  // header location. The chunk header is deferenced.
  // You'll get a crash at around 0xbebebebebebebebe, or -0x4141414141414141 :)
  free(p2);
}
