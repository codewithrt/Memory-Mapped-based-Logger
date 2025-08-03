#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <cstdint>
#include <x86intrin.h>

struct TradeLogEntry
{
    uint64_t buy_order_id;
    uint64_t sell_order_id;
    double price;
    uint32_t quantity;
    uint64_t timestamp; // Timestamp of trade
};

class TradeLogger
{
private:
    int fd;
    size_t max_entries;
    TradeLogEntry *log_ptr;
    size_t current_index;

public:
    TradeLogger(const char* file_name, size_t max_entries_) : max_entries(max_entries_), current_index(0){
        size_t filesize = max_entries * sizeof(TradeLogEntry);

        fd = open(file_name, O_RDWR | O_CREAT ,0666);
        if(fd < 0){
            perror("open");
            exit(1);
        }

        if(lseek(fd,filesize - 1,SEEK_SET) == -1){
            perror("lseek");
            exit(1);
        }

        if(write(fd,"",1) == -1){
            perror("write");
            exit(1);
        }

        log_ptr = (TradeLogEntry*) mmap(nullptr,filesize,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
        if (log_ptr == MAP_FAILED)
        {
            perror("mmap");
            exit(1);
        }
    };

    void log_trade(const TradeLogEntry& log_entry){
        if (current_index < max_entries)
        {
            log_ptr[current_index++] = log_entry;
        }else{
            std::cerr<<"Logs full, cannot write more trades"<<std::endl;
        }
    }

    ~TradeLogger(){
        if(munmap(log_ptr,max_entries * sizeof(TradeLogEntry))){
            perror("munmap");
            exit(1);
        }
        close(fd);
    }
};

int main()
{
    unsigned aux;
    auto logs = TradeLogEntry{
        .buy_order_id = 124,
        .sell_order_id = 120987,
        .price = 23423,
        .quantity = 20,
        .timestamp = __rdtscp(&aux)
    };

    auto logger_handler = new TradeLogger("logs.txt",20);
    logger_handler->log_trade(logs);

    return 0;
}