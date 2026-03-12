
#include"util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <unistd.h>

// TODO: define your own buffer size
#define BUFF_SIZE (1<<21)

#define L1_SIZE 32768
#define L2_SIZE 1048576
#define L3_SIZE 11534336

#define LINE_SIZE 64

// int main(int argc, char **argv)
// {

//     int fd = shm_open("/covert_channel", O_CREAT | O_RDWR, 0666);
//     ftruncate(fd, BUFF_SIZE);

//     // Allocate a buffer using huge page
//     // See the handout for details about hugepage management
//     char *buf = mmap(NULL, BUFF_SIZE,
//         PROT_READ|PROT_WRITE,
//         MAP_SHARED, fd, 0);
  
//     if (buf == (void*) - 1) {
//      perror("mmap() error\n");
//      exit(EXIT_FAILURE);
//     }

    
//     // The first access to a page triggers overhead associated with
//     // page allocation, TLB insertion, etc.
//     // Thus, we use a dummy write here to trigger page allocation
//     // so later access will not suffer from such overhead.
//     *((char *)buf) = 1; // dummy write to trigger page allocation

//     volatile char tmp;

//     // TODO:
//     // Put your covert channel setup code here

//     uint64_t *eviction_buffer = (uint64_t *)malloc(L1_SIZE + L2_SIZE + 100000);

//     if (eviction_buffer == NULL) {
//         perror("Unable to malloc eviction buffer");
//         return EXIT_FAILURE;
//     }

//     // printf("Please type a message.\n");
//     bool sending = true;
//     while (sending) {
//         printf("Please type a message.\n");
//         char text_buf[128];
//         fgets(text_buf, sizeof(text_buf), stdin);
        
//         // TODO:
//         // Put your covert channel code here

//         volatile char *flag = (volatile char *)&buf[4096];
//         volatile char *flag2 = (volatile char *)&buf[8192];

//         int i = 0;
        
//         while (text_buf[i] != '\n' && text_buf[i] != '\0') {
    
//             int number_value = text_buf[i] - '0';
            
//             for (volatile int bit_in_char = 0; bit_in_char < 4; bit_in_char++) {
//                 *flag2 = 0;
//                 // int transmit_bit = (number_value >> bit_in_char) & 0b1;
//                 int volatile transmit_bit = (number_value >> (7 - bit_in_char)) & 1;

//                 // printf("\nBit to transmit: %d\n", transmit_bit);

//                 if (transmit_bit == 0b1) { // make latency small
//                     for (volatile int j = 0; j < 1090; j++) {
//                         tmp = ((char*)buf)[0];
//                     }
//                 } else { // make latency big (evict)
//                     for (volatile int j = 0; j < (L1_SIZE + L2_SIZE + 100000) / 8; j++) {
//                         tmp = eviction_buffer[j];
//                     }
//                 }
//                 // signal to receiver that bit is ready
                
//                 *flag = 1;

//                 for (volatile int dummy = 0; dummy < 100000; dummy++);

//                 // printf("\nArrived at bit: %d\n", bit_in_char);
//                 while (*flag2 != 1) {
//                     // wait until receiver is ready to move on to bext bit
//                 }
//             }
//             i++;
//         }
//         sending = false;
//     }

//   printf("Sender finished.\n\n\n");
//   free(eviction_buffer);
//   return 0;
// }


// ! SINGLE BIT TRANSFER THAT WORKS

// int main(int argc, char **argv)
// {

//     int fd = shm_open("/covert_channel", O_CREAT | O_RDWR, 0666);
//     ftruncate(fd, BUFF_SIZE);

//     // Allocate a buffer using huge page
//     // See the handout for details about hugepage management
//     char *buf = mmap(NULL, BUFF_SIZE,
//         PROT_READ|PROT_WRITE,
//         MAP_SHARED, fd, 0);
  
//     if (buf == (void*) - 1) {
//      perror("mmap() error\n");
//      exit(EXIT_FAILURE);
//     }

    
//     // The first access to a page triggers overhead associated with
//     // page allocation, TLB insertion, etc.
//     // Thus, we use a dummy write here to trigger page allocation
//     // so later access will not suffer from such overhead.
//     *((char *)buf) = 1; // dummy write to trigger page allocation

//     volatile char tmp;

//     // TODO:
//     // Put your covert channel setup code here

//     uint64_t *eviction_buffer = (uint64_t *)malloc(L3_SIZE * 2);

//     if (eviction_buffer == NULL) {
//         perror("Unable to malloc eviction buffer");
//         return EXIT_FAILURE;
//     }

//     // printf("Please type a message.\n");
//     bool sending = true;
//     while (sending) {
//         printf("Please type a message.\n");
//         char text_buf[128];
//         fgets(text_buf, sizeof(text_buf), stdin);

//         // TODO:
//         // Put your covert channel code here

//         volatile char c = text_buf[0];
//         volatile int transmit_bit = c & 0b1;

//         printf("\nBit to transmit: %d\n", transmit_bit);

//         *((char *)buf) = 1;

//         if (transmit_bit == 0b1) { // make latency small
//             for (volatile int i = 0; i < 1090; i++) {
//                 tmp = ((char*)buf)[0];
//             }
//         } else { // make latency big (evict)
//             for (volatile int i = (L3_SIZE * 2) / sizeof(uint64_t); i >= 0 ; i--) {
//                 tmp = eviction_buffer[i];
//             }
//         }
//         sending = false;
//         // Signal the receiver that the bit is ready
//         volatile char *flag = (volatile char *)&buf[2048];
//         *flag = 1;
//         usleep(10);
//         // for (volatile int i = 0; i < 100000; i++);
//     }

//   printf("Sender finished.\n\n\n");
//   free(eviction_buffer);
//   return 0;
// }

// ! END SINGLE BIT TRANSFER

int main(int argc, char **argv)
{
    int fd = shm_open("/covert_channel", O_CREAT | O_RDWR, 0666);
    ftruncate(fd, BUFF_SIZE);

    char *buf = mmap(NULL, BUFF_SIZE,
        PROT_READ|PROT_WRITE,
        MAP_SHARED, fd, 0);
  
    if (buf == (void*) - 1) {
        perror("mmap() error\n");
        exit(EXIT_FAILURE);
    }

    *((char *)buf) = 1; // dummy write to trigger page allocation

    volatile char tmp;

    uint64_t *eviction_buffer = (uint64_t *)malloc(L3_SIZE * 2);

    if (eviction_buffer == NULL) {
        perror("Unable to malloc eviction buffer");
        return EXIT_FAILURE;
    }

    volatile char *flag1 = (volatile char *)&buf[4096];
    volatile char *flag2 = (volatile char *)&buf[8192];

    printf("Please type a message.\n");
    char text_buf[128];
    fgets(text_buf, sizeof(text_buf), stdin);

    int value = string_to_int(text_buf);

    for (int bit_idx = 7; bit_idx >= 0; bit_idx--) {
        int transmit_bit = (value >> bit_idx) & 1;

        // Ensure buf[0] starts in cache before potential eviction
        *((char *)buf) = 1;

        if (transmit_bit == 1) { // keep in cache
            for (volatile int i = 0; i < 109000; i++) {
                tmp = ((char*)buf)[0];
            }
        } else { // evict from cache
            // Multiple passes over 2x L2 to defeat adaptive replacement
            for (volatile int pass = 0; pass < 3; pass++) {
                for (volatile size_t i = 0; i < (L3_SIZE * 2) / sizeof(uint64_t); i++) {
                    tmp = eviction_buffer[i];
                }
            }
        }

        // signal receiver, yield so it can measure without cache interference
        *flag1 = 1;
        usleep(10);

        // wait for receiver to finish measuring
        while (*flag2 != 1) { }
        *flag2 = 0;
    }
    printf("Sender finished.\n");
    free(eviction_buffer);
    return 0;
}