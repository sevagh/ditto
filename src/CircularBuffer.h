#ifndef STOMPBOX_CIRCULARBUFFER_H
#define STOMPBOX_CIRCULARBUFFER_H

#include "stompbox/MiscUtil.h"
#include <array>

namespace stompbox_private::circular_buffer {
    template <std::size_t N>
    class CircularBuffer {
        static_assert(stompbox::detail::misc_util::is_power_of_two(N));

    public:
        std::array<float, N> buffer = {};

        CircularBuffer() : writeIndex(0) {};

        float &operator[](std::size_t i)
        {
            return buffer[(i + writeIndex) & (N-1)];
        }

        void append(float v)
        {
            buffer[writeIndex] = v;
            writeIndex = (writeIndex + 1) & (N-1);
        }

    private:
        std::size_t writeIndex;
    };
} // namespace stompbox_private::circbuf

#endif // STOMPBOX_CIRCULARBUFFER_H
