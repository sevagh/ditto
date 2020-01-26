#include "stompbox/MagicRingBuffer.h"
#include <asm/mman.h>
#include <mutex>
#include <sys/mman.h>
#include <sys/sysconf.h>
#include <zconf.h>

static long page_size;
static std::once_flag page_size_init_flag;

static void page_size_init()
{
    std::call_once(page_size_init_flag, []() {
          page_size = sysconf(_SC_PAGESIZE);
    });
}

static inline size_t ceil_dbl_to_size_t(double x) {
    const double truncation = (size_t)x;
    return truncation + (truncation < x);
}

namespace stompbox::magic_ring_buffer::detail {
int init_mirrored_memory(struct MirroredMemory *mem, std::size_t requested_capacity) {
    page_size_init();

    size_t actual_capacity = ceil_dbl_to_size_t(requested_capacity / (double)page_size) * page_size;

    char shm_path[] = "/dev/shm/stompbox-XXXXXX";
    char tmp_path[] = "/data/local/tmp/stompbox-XXXXXX";
    char *chosen_path;

    int fd = mkstemp(shm_path);
    if (fd < 0) {
        fd = mkstemp(tmp_path);
        if (fd < 0) {
            return -1;
        } else {
            chosen_path = tmp_path;
        }
    } else {
        chosen_path = shm_path;
    }

    if (unlink(chosen_path)) {
        close(fd);
        return -2;
    }

    if (ftruncate(fd, actual_capacity)) {
        close(fd);
        return -3;
    }

    char *address = (char*)mmap(NULL, actual_capacity * 2, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (address == MAP_FAILED) {
        close(fd);
        return -4;
    }

    char *other_address = (char*)mmap(address, actual_capacity, PROT_READ|PROT_WRITE,
                                      MAP_FIXED|MAP_SHARED, fd, 0);
    if (other_address != address) {
        munmap(address, 2 * actual_capacity);
        close(fd);
        return -5;
    }

    other_address = (char*)mmap(address + actual_capacity, actual_capacity,
                                PROT_READ|PROT_WRITE, MAP_FIXED|MAP_SHARED, fd, 0);
    if (other_address != address + actual_capacity) {
        munmap(address, 2 * actual_capacity);
        close(fd);
        return -6;
    }

    mem->address = address;

    if (close(fd))
        return -7;

    mem->capacity = actual_capacity;
    return 0;
}

void deinit_mirrored_memory(struct MirroredMemory *mem) {
    if (!mem->address)
        return;
    int err = munmap(mem->address, 2 * mem->capacity);
    assert(!err);
    mem->address = NULL;
}
} // namespace stompbox::magic_ring_buffer::detail
