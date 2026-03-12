#include "util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>

#define BUF_SIZE (1<<24)
#define NUM_L2_CACHE_SETS 1024
#define LINE_SIZE 64

int main(int argc, char const *argv[]) {
    int flag = -1;

    // Put your capture-the-flag code here

    char *buf = mmap(NULL, BUF_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE,
        -1, 0);

    if(buf == MAP_FAILED){
        perror("mmap");
        exit(1);
    }

    for(int i=0;i<BUF_SIZE;i+=4096){
        buf[i] = 1;
    }

    while(1){

        int best_set = 0;
        CYCLES best_time = 0;

        for(int s=0; s<NUM_L2_CACHE_SETS; s++){

            CYCLES total = 0;

            for(int k=0;k<20;k++){
                total += measure_one_block_access_time(
                    (ADDR_PTR)&buf[s * LINE_SIZE]);
            }

            CYCLES avg = total / 20;

            if(avg > best_time){
                best_time = avg;
                best_set = s;
            }
        }

        flag = best_set;
        break;
    }

    printf("Flag: %d\n", flag);
    return 0;
}