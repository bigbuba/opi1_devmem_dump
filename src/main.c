#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>




// H3, SRAM A2
#define BLOCK_SIZE      (12 * 4096)
#define BLOCK_START     0x00040000




int main(int argc, char **argv)
{
    char buf[4096] = {0};
    int mem_fd;
    int out_fd;
    unsigned int vrt_offset = 0;
    unsigned int * vrt_block_addr;
    off_t phy_block_addr = 0;




    // -------------------------------------------------------------------------
    // --- mmaping ---
    // -------------------------------------------------------------------------

    printf("opening and mmaping /dev/mem ..\n");

    // opening devmem
    if ( (mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0 )
    {
       printf("can't open /dev/mem file \n");
       exit(-1);
    }

    // make a block of phy memory visible in our user space
    vrt_block_addr = mmap(
       NULL,                    // Any adddress in our space
       BLOCK_SIZE,              // Map length
       PROT_READ | PROT_WRITE,  // Enable reading & writting to mapped memory
       MAP_SHARED,              // Shared with other processes
       mem_fd,                  // File to map
       BLOCK_START              // PHY offset
    );

    // exit program if mmap is failed
    if (vrt_block_addr == MAP_FAILED)
    {
       printf("mmap error %d\n", (int)vrt_block_addr);//errno also set!
       exit(-1);
    }

    // no need to keep phy memory file open after mmap
    close(mem_fd);

    printf("mmaping done. \n\n");




    // -------------------------------------------------------------------------
    // --- opening dump file and writing devmem content to it ---
    // -------------------------------------------------------------------------

    printf("reading from devmem, writing to dump file.. \n");

    if ( (out_fd = open("./devmem_dump_0x00040000_48K.bin", O_CREAT|O_WRONLY|O_SYNC) ) < 0 )
    {
       printf("can't open output file \n");
       exit(-1);
    }

    for ( unsigned char b = 0; b < 12; ++b )
    {
        memcpy(buf, (vrt_block_addr + (b*4096)/4), 4096);

        if ( write(out_fd, buf, 4096) < 0 )
        {
            printf("block %d write failed \n", b);
        }
    }

    close(out_fd);

    printf("reading/writing done. \n\n");




    // -------------------------------------------------------------------------
    // --- unmmaping ---
    // -------------------------------------------------------------------------

    printf("unmmaping /dev/mem .. \n");

    munmap(vrt_block_addr, BLOCK_SIZE);

    printf("unmmaping done.\n\n");




    // -------------------------------------------------------------------------
    // --- exit --
    // -------------------------------------------------------------------------

    printf("all work is done.\n");

    return 0;
}
