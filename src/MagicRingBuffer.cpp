#include "stompbox/MagicRingBuffer.h"
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <string>

namespace stompbox::magic_ring_buffer {
MagicRingBuffer::MagicRingBuffer(std::size_t requested_capacity)
    : write_offset(0)
    , read_offset(0)
    , mem(detail::MirroredMemory())
{
    int err = detail::init_mirrored_memory(&mem, requested_capacity*detail::FloatSize);
    if (err != 0)
        throw std::runtime_error(std::string("Couldn't init ringbuffer, ") + std::to_string(err));

    capacity = mem.capacity;
}

std::size_t MagicRingBuffer::get_capacity() {
    return capacity/detail::FloatSize;
}

float *MagicRingBuffer::write_ptr() {
    auto write_load = write_offset.load();
    return mem.address + (write_offset % capacity);
}

float *MagicRingBuffer::read_ptr() {
    auto read_load = read_offset.load();
    return mem.address + (read_offset % capacity);
}

void MagicRingBuffer::advance_write_ptr(std::size_t count) {
    write_offset.fetch_add(count*detail::FloatSize);
    assert(fill_count() >= 0);
}

void MagicRingBuffer::advance_read_ptr(std::size_t count) {
    read_offset.fetch_add(count*detail::FloatSize);
    assert(fill_count() >= 0);
}

std::size_t MagicRingBuffer::fill_count() {
    // Whichever offset we load first might have a smaller value. So we load
    // the read_offset first.
    auto read_load = read_offset.load();
    auto write_load = write_offset.load();

    std::size_t count = write_load - read_load;
    assert(count >= 0);
    assert(count <= rb->capacity);
    return count/detail::FloatSize;
}

std::size_t MagicRingBuffer::free_count() {
    return (capacity - fill_count())/detail::FloatSize;
}

void MagicRingBuffer::clear() {
    write_offset.store(read_offset);
}

MagicRingBuffer::~MagicRingBuffer()
{
    detail::deinit_mirrored_memory(&mem);
}
} // namespace stompbox::magic_ring_buffer