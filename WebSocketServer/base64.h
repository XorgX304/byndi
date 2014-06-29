#pragma once

class Base64 {
public:
	static const char* chars() {
		return "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	}

	static void code(unsigned char* ca3, unsigned char* ca4) {
		ca4[0] = (ca3[0] & 0xFC) >> 2;
		ca4[1] = ((ca3[0] & 0x03) << 4) + ((ca3[1] & 0xF0) >> 4);
		ca4[2] = ((ca3[1] & 0x0F) << 2) + ((ca3[2] & 0xC0) >> 6);
		ca4[3] = ca3[2] & 0x3F;
	}

	static bool isValid(unsigned char c) {
		return (isalnum(c) || (c == '+') || (c == '/'));
	}

	static std::string encode(void* data, size_t len) {
		int i = 0, j = 0;
		unsigned char ca3[3], ca4[4];
		unsigned char* udata = (unsigned char*) data;

		std::string r;

		while(len--) {
			ca3[i++] = *(udata++);

			if(i == 3) {
				Base64::code(ca3, ca4);

				for(i = 0; i < 4; i++) {
					r += Base64::chars()[ca4[i]];
				}

				i = 0;
			}
		}

		if(i) {
			for(j = i; j < 3; j++) {
				ca3[j] = '\0';
			}

			Base64::code(ca3, ca4);

			for(j = 0; j < i + 1; j++) {
				r += Base64::chars()[ca4[j]];
			}

			while(i++ < 3) {
				r += '=';
			}
		}

		return r;
	}

	static std::string encode(std::string data) {
		return Base64::encode((unsigned char*) data.c_str(), data.length());
	}
};