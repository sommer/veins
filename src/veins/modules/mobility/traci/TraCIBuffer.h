#ifndef VEINS_MOBILITY_TRACI_TRACIBUFFER_H_
#define VEINS_MOBILITY_TRACI_TRACIBUFFER_H_

#include <cstddef>
#include <string>

#include "veins/base/utils/MiXiMDefs.h"

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

		template<typename T> T read() {
			T buf_to_return;
			unsigned char *p_buf_to_return = reinterpret_cast<unsigned char*>(&buf_to_return);

			if (isBigEndian()) {
				for (size_t i=0; i<sizeof(buf_to_return); ++i) {
					if (eof()) throw cRuntimeError("Attempted to read past end of byte buffer");
					p_buf_to_return[i] = buf[buf_index++];
				}
			} else {
				for (size_t i=0; i<sizeof(buf_to_return); ++i) {
					if (eof()) throw cRuntimeError("Attempted to read past end of byte buffer");
					p_buf_to_return[sizeof(buf_to_return)-1-i] = buf[buf_index++];
				}
			}

			return buf_to_return;
		}

		template<typename T> void write(T inv) {
			unsigned char *p_buf_to_send = reinterpret_cast<unsigned char*>(&inv);

			if (isBigEndian()) {
				for (size_t i=0; i<sizeof(inv); ++i) {
					buf += p_buf_to_send[i];
				}
			} else {
				for (size_t i=0; i<sizeof(inv); ++i) {
					buf += p_buf_to_send[sizeof(inv)-1-i];
				}
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
