#pragma once

namespace Byndi {

	class WebSocketMsgReader {
	public:
		struct WebSocketPacket {
			WebSocketHeader header;
			uint64_t payload_length;
			uint8_t mask[4];
			std::string payload;
		};

		static bool readRfc6455(void* buffer, size_t size, WebSocketPacket* packet) {
			WebSocketReader reader(buffer, size);

			reader.read(&packet->header, sizeof(WebSocketHeader)); //16 bits = 2 bytes

			if(!packet->header.isOpcodeKnown() && !packet->header.isKnownControlOpCode()) {
				printf("Unknown Opcode: 0x%X\n", packet->header.getOpcode());
				return false;
			}

			// We need to add this
			if(packet->header.getOpcode() == WebSocketHeader::kContinuation || packet->header.getFin() != 1) {
				printf("Server does not yet support continuation...\n");
				return false;
			}

			if(packet->header.getBasicLength() == 126) {
				unsigned short ushortSize = ntohs(reader.read<unsigned short>());
				packet->payload_length = (unsigned __int64) ushortSize;
			} else if(packet->header.getBasicLength() == 127) {
				uint64_t uint64Size = reader.read<uint64_t>();

				uint8_t* htonll = (uint8_t*) &uint64Size;

				htonll[0] >>= 56;
				htonll[1] >>= 48;
				htonll[2] >>= 40;
				htonll[3] >>= 32;
				htonll[4] >>= 24;
				htonll[5] >>= 16;
				htonll[6] >>= 8;
				htonll[7] >>= 0;

				packet->payload_length = uint64Size;
			} else {
				packet->payload_length = packet->header.getBasicLength();
			}

			if(packet->payload_length > size) {
				printf("Buffer size overflow [%I64d -> %lu]\n",
					packet->payload_length, size);

				return false;
			}

			if(packet->header.isMasked()) {
				reader.read(packet->mask, 0x4);
			}

			std::stringstream payloadStream;
			for(uint64_t i = 0; i < packet->payload_length; i++) {
				uint8_t raw = reader.read<uint8_t>();

				if(packet->header.isMasked()) {
					uint8_t masked = packet->mask[i % 4];
					uint8_t umaskd = raw ^ masked;
					payloadStream << umaskd;
				} else {
					payloadStream << raw;
				}
			}

			packet->payload = payloadStream.str();

			return true;
		}
	};

	class WebSocketMsgWriter {
	public:
		static bool writeRfc6455(void* buffer, size_t size, WebSocketHeader::WebSocketOpcode opcode, WebSocketWriter* writer, bool masked = false) {
			writer->write<uint8_t>(0x80 | (opcode & 0x0F));

			if(size < 126) {
				writer->write<uint8_t>(size);
			} else if(size < 65536) {
				writer->write<uint8_t>(126);
				writer->write<uint16_t>(htons(static_cast<uint16_t>(size)));
			} else {
				writer->write<uint8_t>(127);
				writer->write<uint32_t>(0);
				writer->write<uint32_t>(htonl(static_cast<uint32_t>(size)));
			}

			// What about masking it?

			writer->write(buffer, size);

			return true;
		}

		static bool writeRfc6455(std::string payload, WebSocketHeader::WebSocketOpcode opcode, WebSocketWriter* writer, bool masked = false) {
			return writeRfc6455((void*) payload.c_str(), payload.length(), opcode, writer);
		}
	};

};