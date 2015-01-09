#ifndef VEINS_MOBILITY_TRACI_TRACIBUFFER_H_
#define VEINS_MOBILITY_TRACI_TRACIBUFFER_H_

#include <cstddef>
#include <string>

#include "veins/base/utils/MiXiMDefs.h"

namespace Veins {

struct TraCICoord;

namespace detail {

template<unsigned> struct size_trait;

template<> struct size_trait<1> {
	typedef uint8_t type;
};

template<> struct size_trait<2> {
	typedef uint16_t type;
};

template<> struct size_trait<4> {
	typedef uint32_t type;
};

template<> struct size_trait<8> {
	typedef uint64_t type;
};

} // namespace detail

/**
 * Byte-buffer that stores values in TraCI byte-order
 */
class TraCIBuffer {
	public:
		TraCIBuffer();
		TraCIBuffer(std::string buf);

		template<typename T> T read() {
			union {
				typename detail::size_trait<sizeof(T)>::type uint;
				T t;
			} output;
			output.uint = 0;

			if (buf_index + sizeof(output) <= buf.length()) {
				for (size_t i = 0; i < sizeof(output); ++i) {
					if (sizeof(T) > 1) output.uint <<= 8;
					output.uint |= buf[buf_index++] & 0xff;
				}
			}
			else {
				throw cRuntimeError("Attempted to read past end of byte buffer");
			}

			return output.t;
		}

		template<typename T> void write(T inv) {
			union {
				typename detail::size_trait<sizeof(T)>::type uint;
				T t;
			} input;
			input.t = inv;

			buf.append(sizeof(input), '\0');
			std::string::reverse_iterator it = buf.rbegin();
			for (size_t i = 0; i < sizeof(input); ++i) {
				*it++ = static_cast<uint8_t>(input.uint);
				if (sizeof(T) > 1) input.uint >>= 8;
			}
		}

		template<typename T> T read(T& out) {
			out = read<T>();
			return out;
		}

		template<typename T> TraCIBuffer& operator >>(T& out) {
			out = read<T>();
			return *this;
		}

		template<typename T> TraCIBuffer& operator <<(const T& inv) {
			write(inv);
			return *this;
		}

		bool eof() const;
		void set(std::string buf);
		void clear();
		std::string str() const;
		std::string hexStr() const;

	private:
		std::string buf;
		size_t buf_index;
};

template<> void TraCIBuffer::write(std::string inv);
template<> void TraCIBuffer::write(TraCICoord inv);
template<> std::string TraCIBuffer::read();
template<> TraCICoord TraCIBuffer::read();

}

#endif /* VEINS_MOBILITY_TRACI_TRACIBUFFER_H_ */
