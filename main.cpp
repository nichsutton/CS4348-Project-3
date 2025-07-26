#include <vector>
#include <cstdint>
#include <iostream>
#include <map>

#define KB_SIZE 1024 // in bytes
#define MEM_SIZE 1000000 // in bytes

#define FRAME_SIZE (4 * KB_SIZE) // 4kb frames
#define FRAME_COUNT 256

#define PAGE_SIZE (4 * KB_SIZE) // 4kb frames
#define PAGE_COUNT 256

// to use this just type physicalMemory[frameNum, offset], should return whatever is at the address
std::vector<std::vector<uint8_t> > physical_memory(FRAME_COUNT, std::vector<uint8_t>(FRAME_SIZE));

int free_frame_count = FRAME_COUNT;
// gets a free frame for use
std::vector<bool> frame_used(FRAME_COUNT, false);
int get_free_frame() {
    if (free_frame_count <= 0) return -1;
    for (int i = 0; i < FRAME_COUNT; i++) {
        if(!frame_used[i]) {
            frame_used[i] = true;
            free_frame_count--;
            return i;
        }
    }
    return -1;
}

/*
to use this type:
page_table[page_num].valid_bit = true or false
page_table[page_num].frame_num = frame num 
page_table[page_num].reference_bit = true or false
page_table[page_num].modified_bit = true or false
*/
struct page_table_entry {
    bool valid_bit = false;
    int frame_num = 0;
    bool reference_bit = false; // for LRU
    bool modified_bit = false;
};
std::map<std::string, std::map<int, page_table_entry> > process;

// alloc command
void alloc(std::string& pid, int num_of_pages) {
    auto& page_table = process[pid];
    int virtual_page_num = page_table.size();
    if (free_frame_count < num_of_pages) {std::cout << "Not enough frames avaliable.\n"; return;}
    for (int i = 0; i < num_of_pages; i++) {
        int free_frame = get_free_frame();
        if (free_frame == -1) {
            std::cout << "Allocation error for PID: " << pid << "\n";
            return;
        }
        page_table_entry pte;
        pte.valid_bit = true;
        pte.frame_num = free_frame;
        page_table[virtual_page_num + i] = pte; // adds entry to the page_table if successful
    }
    std::cout << "Allocation of " << num_of_pages << " pages was successful for PID: " << pid << "\n";
}

// free command
void free(std::string& pid) {
    if (process.count(pid) == 0) {std::cout << "Process with PID: " << pid << " not found\n"; return;}
    for (auto& [page_num, pte] : process[pid]) {
        if (pte.valid_bit) {
            frame_used[pte.frame_num] = false;
            free_frame_count++;
        }
    }
    process.erase(pid);
    std::cout <<  "Memory freed for PID: " << pid << "\n";
}


// this is just for reference to get page num and offset from a virtual address
// 4095 is last address of first frame, 4096 is first address of second frame. everything is 0 indexed
int virtual_addr = 4096;
int page_num = virtual_addr / FRAME_SIZE;
int page_offset = virtual_addr % FRAME_SIZE;

int main() {
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
        
    }


    return 0;
}
