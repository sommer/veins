#pragma once

#include <cstddef>
#include <string>

#include "veins/veins.h"

namespace Veins {

struct TraCICoord;

bool isBigEndian();

/**
 * Byte-buffer that stores values in TraCI byte-order
 */
class TraCIBuffer {
public:
    TraCIBuffer();
    TraCIBuffer(std::string buf);

    template <typename T>
    T read()
    {
        T buf_to_return;
        unsigned char* p_buf_to_return = reinterpret_cast<unsigned char*>(&buf_to_return);

        if (isBigEndian()) {
            for (size_t i = 0; i < sizeof(buf_to_return); ++i) {
                if (eof()) throw cRuntimeError("Attempted to read past end of byte buffer");
                p_buf_to_return[i] = buf[buf_index++];
            }
        }
        else {
            for (size_t i = 0; i < sizeof(buf_to_return); ++i) {
                if (eof()) throw cRuntimeError("Attempted to read past end of byte buffer");
                p_buf_to_return[sizeof(buf_to_return) - 1 - i] = buf[buf_index++];
            }
        }

        return buf_to_return;
    }

    template <typename T>
    void write(T inv)
    {
        unsigned char* p_buf_to_send = reinterpret_cast<unsigned char*>(&inv);

        if (isBigEndian()) {
            for (size_t i = 0; i < sizeof(inv); ++i) {
                buf += p_buf_to_send[i];
            }
        }
        else {
            for (size_t i = 0; i < sizeof(inv); ++i) {
                buf += p_buf_to_send[sizeof(inv) - 1 - i];
            }
        }
    }

    void readBuffer(unsigned char* buffer, size_t size)
    {
        if (isBigEndian()) {
            for (size_t i = 0; i < size; ++i) {
                if (eof()) throw cRuntimeError("Attempted to read past end of byte buffer");
                buffer[i] = buf[buf_index++];
            }
        }
        else {
            for (size_t i = 0; i < size; ++i) {
                if (eof()) throw cRuntimeError("Attempted to read past end of byte buffer");
                buffer[size - 1 - i] = buf[buf_index++];
            }
        }
    }

    template <typename T>
    T read(T& out)
    {
        out = read<T>();
        return out;
    }

    template <typename T>
    TraCIBuffer& operator>>(T& out)
    {
        out = read<T>();
        return *this;
    }

    template <typename T>
    TraCIBuffer& operator<<(const T& inv)
    {
        write(inv);
        return *this;
    }

    /**
     * @brief
     * read and check type, then read and return an item from the buffer
     */
    template <typename T>
    T readTypeChecked(int expectedTraCIType)
    {
        uint8_t read_type(read<uint8_t>());
        ASSERT(read_type == static_cast<uint8_t>(expectedTraCIType));
        return read<T>();
    }

    bool eof() const;
    void set(std::string buf);
    void clear();
    std::string str() const;
    std::string hexStr() const;

    static void setTimeAsDouble(bool val)
    {
        timeAsDouble = val;
    }

private:
    std::string buf;
    size_t buf_index;
    static bool timeAsDouble;
};

template <>
std::vector<std::string> TraCIBuffer::readTypeChecked(int expectedTraCIType);
template <>
void TraCIBuffer::write(std::string inv);
template <>
void TraCIBuffer::write(TraCICoord inv);
template <>
std::string TraCIBuffer::read();
template <>
TraCICoord TraCIBuffer::read();
template <>
void TraCIBuffer::write(simtime_t o);
template <>
simtime_t TraCIBuffer::read();

} // namespace Veins
