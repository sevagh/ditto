#ifndef DITTO_MAGIC_RING_BUFFER_H
#define DITTO_MAGIC_RING_BUFFER_H

#include <atomic>
#include <cstddef>

namespace ditto {
namespace detail {
    struct MirroredMemory {
        std::size_t capacity;
        char *address;
    };

    int init_mirrored_memory(struct MirroredMemory *mem, std::size_t requested_capacity);
    void deinit_mirrored_memory(struct MirroredMemory *mem);
}

class MagicRingBuffer {
public:
    explicit MagicRingBuffer(std::size_t capacity);
    ~MagicRingBuffer();

    std::size_t fill_count();
    std::size_t get_capacity();
    void clear();
    std::size_t free_count();
    void advance_read_ptr(std::size_t count);
    void advance_write_ptr(std::size_t count);
    char *write_ptr();
    char *read_ptr();

private:
    struct detail::MirroredMemory mem;

    std::size_t capacity;
    std::atomic_uint64_t write_offset;
    std::atomic_uint64_t read_offset;
};
} // namespace ditto

#endif /* DITTO_MAGIC_RING_BUFFER_H */
