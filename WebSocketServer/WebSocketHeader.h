#pragma once

namespace Byndi {
	class WebSocketHeader {
	public:
		enum WebSocketOpcode : uint16_t {
			kContinuation = 0x0,
			kText = 0x1,
			kBinary = 0x2,
			kNonControl_RESERVED0 = 0x3,
			kNonControl_RESERVED1 = 0x4,
			kNonControl_RESERVED2 = 0x5,
			kNonControl_RESERVED3 = 0x6,
			kNonControl_RESERVED4 = 0x7,
			kConnectionClose = 0x8,
			kPing = 0x9,
			kPong = 0xA,
			kControl_RESERVED0 = 0xB,
			kControl_RESERVED1 = 0xC,
			kControl_RESERVED2 = 0xD,
			kControl_RESERVED3 = 0xE,
			kControl_RESERVED4 = 0xF,
		};

		__forceinline unsigned char getFin() {
			return this->fin;
		}

		__forceinline WebSocketOpcode getOpcode() {
			return (WebSocketOpcode) this->opcode;
		}

		__forceinline uint8_t getBasicLength() {
			return this->payload_length;
		}

		__forceinline uint8_t getRSV1() {
			return this->rsv1;
		}

		__forceinline uint8_t getRSV2() {
			return this->rsv2;
		}

		__forceinline uint8_t getRSV3() {
			return this->rsv3;
		}

		__forceinline bool isContinuation() {
			return (getFin() == 0 || getOpcode() == kContinuation);
		}

		__forceinline bool isMasked() {
			return (this->mask == 1);
		}

		__forceinline bool isOpcodeKnown() {
			return getOpcode() == kContinuation || getOpcode() == kText || getOpcode() == kBinary;
		}
		
		__forceinline bool isKnownControlOpCode() {
			return getOpcode() == kConnectionClose || getOpcode() == kPing || getOpcode() == kPong;
		}

	private:
		uint8_t opcode: 4;
		uint8_t rsv3: 1;
		uint8_t rsv2: 1;
		uint8_t rsv1: 1;
		uint8_t fin: 1;
		uint8_t payload_length: 7;
		uint8_t mask: 1;
	};
};