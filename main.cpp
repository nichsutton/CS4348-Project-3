#include <vector>
#include <deque>
#include <cstdint>
#include <iostream>
#include <map>

#define KB_SIZE 1024 // in bytes
#define MEM_SIZE 1000000 // in bytes

#define FRAME_SIZE (4 * KB_SIZE) // 4kb frames
#define FRAME_COUNT 256

#define PAGE_SIZE (4 * KB_SIZE) // 4kb frames
#define PAGE_COUNT 256

// to use this just type physical_memory[frameNum, offset], should return whatever is at the address
std::vector<std::vector<uint8_t> > physical_memory(FRAME_COUNT, std::vector<uint8_t>(FRAME_SIZE));

int free_frame_count = FRAME_COUNT;

// Dequeues used for page replacement algorithms
std::deque<int> frame_order_fifo;
std::deque<int> frame_order_lru;

struct frame_table_entry {
    bool occupied;
    std::string pid;
    int virtual_page_num;
};
frame_table_entry frame_table[FRAME_SIZE];

struct page_table_entry {
    bool valid_bit = false;
    int frame_num = 0;
    bool modified_bit = false;
};
std::map<std::string, std::map<int, page_table_entry> > process;

// Swaps out frame to be replaced
// Mode 0 -> FIFO
// Mode 1 -> LRU
// Invalid Mode -> FIFO
void swap_out_frame(int mode) {
    auto& frame_order = frame_order_fifo;
    if (mode == 1) {
        auto& frame_order = frame_order_lru;
    }

    auto& fte = frame_table[frame_order.back()];
    fte.occupied = false;
    auto& page_table = process[fte.pid];
    auto& pte = page_table[fte.virtual_page_num];
    pte.valid_bit = false;
    frame_order.pop_back();
    free_frame_count++;
}

// gets a free frame for use
int get_free_frame(std::string& pid, int virtual_page_num) {
    if (free_frame_count <= 0) swap_out_frame(0);
    for (int i = 0; i < FRAME_COUNT; i++) {
        if(!frame_table[i].occupied) {
            frame_table[i].occupied = true;
            frame_table[i].pid = pid;
            frame_table[i].virtual_page_num = virtual_page_num;
            frame_order_fifo.push_front(i);
            frame_order_lru.push_front(i);
            free_frame_count--;
            return i;
        }
    }
    return -1;
}

// Swaps in page of process
void swap_in_page(std::string& pid, int virtual_page_num) {
    int free_frame = get_free_frame(pid, virtual_page_num);
    auto& page_table = process[pid];
    auto& pte = page_table[virtual_page_num];
    pte.valid_bit = true;
    pte.frame_num = free_frame;
}

// alloc command
void alloc(std::string& pid, int num_of_pages) {
    auto& page_table = process[pid];
    int virtual_page_num = page_table.size();
    for (int i = 0; i < num_of_pages; i++) {
        int free_frame = get_free_frame(pid, i);
        if (free_frame == -1) {
            std::cout << "Allocation error for PID: " << pid << "\n";
            return;
        }
        page_table_entry pte;
        pte.valid_bit = true;
        pte.frame_num = free_frame;
        page_table[virtual_page_num + i] = pte; // adds entry to the page_table if successful
    }
    std::cout << "Allocated " << num_of_pages << " pages for process " << pid << "\n";
}

// free command
void free(std::string& pid) {
    if (process.count(pid) == 0) {std::cout << "Process " << pid << " not found\n"; return;}
    for (auto& [page_num, pte] : process[pid]) {
        if (pte.valid_bit) {
            frame_table[pte.frame_num].occupied = false;
            // Remove frame from frame order
            for (std::deque<int>::iterator it = frame_order_fifo.begin(); it != frame_order_fifo.end();)
            {
                if (*it == pte.frame_num)
                    it = frame_order_fifo.erase(it);
                else
                    ++it;
            }
            for (std::deque<int>::iterator it = frame_order_lru.begin(); it != frame_order_lru.end();)
            {
                if (*it == pte.frame_num)
                    it = frame_order_lru.erase(it);
                else
                    ++it;
            }
            free_frame_count++;
        }
    }
    process.erase(pid);
    std::cout <<  "Freed memory for process " << pid << "\n";
}

// access command
void access(std::string& pid, int virtual_address, std::string& mode) {
    auto& page_table = process[pid];
    int page_num = virtual_address / FRAME_SIZE;
    int page_offset = virtual_address % FRAME_SIZE;

    int total_page_num = page_table.size();
    if (page_num >= total_page_num) {
        std::cout << "Invalid virtual address!\n";
        return;
    }

    // Needs to be swapped in if it's not in memory
    if (!page_table[page_num].valid_bit) {
        swap_in_page(pid, page_num);
    }

    int physical_address = page_table[page_num].frame_num * 4096 + page_offset;
    std::cout << "Translated virtual address " << virtual_address << " (" << pid << ") → physical address " << physical_address << "\n";
}


void setupFrameTable() {
    for (int i = 0; i < FRAME_COUNT; i++)
    {
        frame_table_entry fte;
        fte.occupied = false;
        fte.virtual_page_num = -1;
        frame_table[i] = fte;
    }
}

int main() {
    setupFrameTable();

    std::string cmd;

    while (true) {
        std::cin >> cmd;
        std::string pid;

        if (cmd == "quit") {break;}

        // run commands here
        if (cmd == "alloc") {
            int num_of_pages;
            std::cin >> pid >> num_of_pages;
            alloc(pid, num_of_pages);
        }

        if (cmd == "free") {
            std::cin >> pid;
            free(pid);
        }

        if (cmd == "access") {
            int virtual_address;
            std::string mode;
            std::cin >> pid >> virtual_address >> mode;
            access(pid, virtual_address, mode);
        }
        
    }


    return 0;
}
