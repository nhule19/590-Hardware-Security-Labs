#include "../shared.hh"
#include "../params.hh"
#include "../util.hh"

#define SAMPLES 1000
#define ROUNDS  1000


/*
 *
 * DO NOT MODIFY BELOW ME
 *
 */
uint64_t median(uint64_t* vals, size_t size);
void print_hist(uint64_t *data, size_t n);

int main (int ac, char **av) {
    
    setvbuf(stdout, NULL, _IONBF, 0);

    // Allocate a large pool of memory (of size BUFFER_SIZE_MB) pointed to
    // by allocated_mem
    allocated_mem = allocate_pages(BUFFER_SIZE_MB * 1024UL * 1024UL);
 
    // Setup PPN_VPN_map
    setup_PPN_VPN_map(allocated_mem, PPN_VPN_map);

    uint64_t bank_latency[SAMPLES] = {0};

    //Calculate bank latencies
    // First, measure bank latencies for addresses in the same bank 
    for (int i = 0; i < SAMPLES; i++){
        char *rnd_A, *rnd_B;
        
        // Get a pair of random addresses
        do {
            rnd_A = get_rand_addr(BUFFER_SIZE_MB * 1024UL * 1024UL);
            rnd_B = get_rand_addr(BUFFER_SIZE_MB * 1024UL * 1024UL);
        } while (rnd_A == rnd_B);

        // measure the latency repeatedly and record the median of the time value
        uint64_t* time_vals = (uint64_t*) calloc(ROUNDS, sizeof(uint64_t));
        for(int j = 0; j < ROUNDS; j++){
            time_vals[j] = measure_bank_latency((volatile char*)rnd_A, (volatile char*)rnd_B);
        }
        bank_latency[i] = median(time_vals, ROUNDS);
        free(time_vals);
    }

    print_hist(bank_latency, SAMPLES);

    return 0;
}

// Supporting functions for printing results in hist

int gt(const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}

uint64_t median(uint64_t* vals, size_t size) {
	qsort(vals, size, sizeof(uint64_t), gt);
	return ((size%2)==0) ? vals[size/2] : (vals[(size_t)size/2]+vals[((size_t)size/2+1)])/2;
}

void print_hist(uint64_t *data, size_t n)
{
    std::map<uint64_t, uint64_t> hist;

    // build histogram
    for (size_t i = 0; i < n; i++)
        hist[data[i]]++;

    // find max count
    uint64_t max_cnt = 0;
    for (auto &p : hist)
        if (p.second > max_cnt)
            max_cnt = p.second;

    const int MAX_BAR = 60; 

    // print
    for (auto &p : hist)
    {
        uint64_t val = p.first;
        uint64_t cnt = p.second;

        int bar_len = (int)((double)cnt / max_cnt * MAX_BAR);
        if (bar_len == 0) bar_len = 1;

        printf("%4lu : %-6lu ", val, cnt);
        for (int i = 0; i < bar_len; i++)
            putchar('#');
        putchar('\n');
    }
}