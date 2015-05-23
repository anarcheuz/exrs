#include <stdlib.h>
#include <string.h>

int main(int argc, const char* argv[])
{
  // Allocate three large contiguous chunks.
  char* p = malloc(1024 - 8);
  char* p2 = malloc(1024 - 8);
  char* p3 = malloc(1024 - 8);
  // Set them up with easily recognizable values.
  memset(p, 'A', 1024 - 8);
  memset(p2, 'B', 1024 - 8);
  memset(p3, 'C', 1024 - 8);
  // Overflow the second chunk, off-by-one, with a NUL byte.
  p2[1024 - 8] = '\0';
  // Free the first chunk.
  // Because of the overflow, the second chunk will incorrectly be seen as free.
  // The first 16 bytes of the second chunk, 0x4242424242424242 (x2), will be
  // treated as freelist pointers.
  // You'll get a crash at around 0x4242424242424242.
  free(p);
}
