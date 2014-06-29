#pragma once

namespace Byndi {

	class WebSocketWriter {
	public:
		WebSocketWriter() : bufferRaw(NULL) {}

		size_t count() {
			return step;
		}

		size_t size() {
			return raw.size();
		}

		void write(void* buffer, size_t size) {
			for(size_t i = 0; i < size; i++) {
				raw.push_back(((uint8_t*)buffer)[i]);
			}
		}

		template<typename T> void write(T data) {
			write(&data, sizeof(T));
		}

		void* buffer() {
			free();

			bufferRaw = new uint8_t[size()];

			for(size_t i = 0; i < size(); i++) {
				bufferRaw[i] = raw[i];
			}

			return bufferRaw;
		}

		void free() {
			if(bufferRaw != NULL) {
				delete[] bufferRaw;

				bufferRaw = NULL;
			}
		}

	private:
		size_t step;
		std::vector<uint8_t> raw;
		uint8_t* bufferRaw;
	};

};