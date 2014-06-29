#pragma once

namespace Byndi {

	class WebSocketReader {
	public:
		WebSocketReader(void* buffer, size_t size) {
			for(size_t i = 0; i < size; i++) {
				raw.push_back(((unsigned char*)buffer)[i]);
			}

			step = 0;
		}

		void rewind() {
			step = 0;
		}

		void rewind(size_t i) {
			step -= i;
		}

		void forward(size_t i) {
			step += i;
		}

		size_t count() {
			return step;
		}

		size_t size() {
			return raw.size();
		}

		void readAt(size_t idx, void* data, size_t size) {
			if(size == 0) return;
			for(size_t i = 0; i < size; i++) {
				((unsigned char*)data)[i] = raw[i + idx];
			}
		}

		void read(void* data, size_t size) {
			if(size == 0) return;
			readAt(step, data, size);
			forward(size);
		}

		template<typename T> T readAt(size_t idx) {
			T r;
			readAt(idx, &r, sizeof(T));
			return r;
		}

		template<typename T> T read() {
			T r;
			read(&r, sizeof(T));
			return r;
		}

	private:
		size_t step;
		std::vector<unsigned char> raw;
	};

};