#pragma once

namespace Byndi {

	class WebSocketServer {
	public:
		WebSocketServer() {
			//
		}

		WebSocketServer(std::string ip, std::string port, unsigned long bufferSize, std::string path = "/", std::string origin = "") {
			this->init(ip, port, bufferSize, path, origin);
		}

		~WebSocketServer() {
			shutdown(this->listenfd, WSAECONNRESET);
			closesocket(this->listenfd);
		}

		bool init(std::string ip, std::string port, unsigned long bufferSize, std::string path = "/", std::string origin = "") {
			this->origin = origin;
			this->port = port;
			this->path = path;
			this->ipAddress = ip;
			this->location = "ws://" + this->ipAddress + ":" + this->port + this->path;
			this->bufferSize = bufferSize;

			this->server.sin_family = AF_INET;
			this->server.sin_addr.s_addr = inet_addr(this->ipAddress.c_str());
			this->server.sin_port = htons((unsigned short) atoi(this->port.c_str()));

			this->listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			if(bind(listenfd, (SOCKADDR*) &server, sizeof(server)) != SOCKET_ERROR) {
				if(listen(listenfd, SOMAXCONN) != SOCKET_ERROR) {
					return true;
				} else {
					printf("Listen error\n");
				}
			} else {
				printf("Bind error\n");
			}

			return false;
		}

		bool isValidSocket() {
			return (this->listenfd != INVALID_SOCKET && this->listenfd != SOCKET_ERROR);
		}

		bool accept(WebSocketClient* out) {
			WebSocketClient cl;

			cl.socket = ::accept(listenfd, &cl.info, NULL);

			if(cl.socket == INVALID_SOCKET || cl.socket == SOCKET_ERROR) {
				printf("Client Socket Error\n");
				return false;
			}

			memcpy(&cl.ipv4info, &cl.info, sizeof(struct sockaddr_in));

			char* request0 = new char[bufferSize];

			int received = recv(cl.socket, request0, bufferSize, 0);

			if(received > bufferSize) {
				printf("Buffer size overflow\n");

				delete[] request0;

				return false; // bufferSize overflow
			}

			bool ret = handshake(&cl, request0);

			delete[] request0;

			if(ret) {
				clients.push_back(cl);

				if(out != NULL) {
					memcpy(out, &cl, sizeof(WebSocketClient));
				}

				return true;
			} else {
				shutdown(cl.socket, 0);
				cl.socket = SOCKET_ERROR;
			}

			return false;
		}

		bool handshake(WebSocketClient* client, char* request) {
			std::hash_map<std::string, std::string> data;

			if(parseHeaders(&data, request)) {
				if(!data["Sec-WebSocket-Version"].size() || data["Sec-WebSocket-Version"].compare("13") != 0) {
					printf("Client error: Invalid WebSocket version!\n");
					return false;
				}

				if(data["Sec-WebSocket-Key1"].size()) {
					return handshakeRespondHixie(&data, client);
				} else if(data["Sec-WebSocket-Key"].size()) {
					return handshakeRespondSha(&data, client);
				} else {
					printf("Client error: No WebSocket key! Rejected!\n");
				}
			} else {
				printf("Client error: Failed to parse headers! Rejected!\n");
			}

			return false;
		}

		bool handshakeRespondHixie(std::hash_map<std::string, std::string>* headers, WebSocketClient* client) {
			printf("Server does not yet support Hixie backward compatibility...\n");
			return false;
		}

		bool handshakeRespondSha(std::hash_map<std::string, std::string>* headers, WebSocketClient* client) {
			std::string finalKey = (*headers)["Sec-WebSocket-Key"] + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

			char* sha1hash;

			Sha1.init();
		
			for(size_t i = 0; i < finalKey.size(); i++) {
				Sha1.write(finalKey[i]);
			}

			sha1hash = (char*) Sha1.result();
			sha1hash[20] = '\0';

			std::string b64result = Base64::encode(sha1hash, 20);

			std::string reply = 
				"HTTP/1.1 101 Web Socket Protocol Handshake\r\n"
				"Upgrade: websocket\r\n"
				"Connection: Upgrade\r\n"
				"WebSocket-Location: " + this->location + "\r\n"
				+ ((this->origin.length() > 0) ? ("WebSocket-Origin: " + this->origin) : "") + 
				"Sec-WebSocket-Accept: " + b64result + "\r\n"
				"\r\n";

			int sent = send(client->socket, reply.c_str(), reply.length(), 0);

			return sent > 0;
		}

		bool parseHeaders(std::hash_map<std::string, std::string>* map, char* request) {
			std::istringstream l(request);

			int lineNum = 0;

			std::string line;
			while(std::getline(l, line)) {
				if(memcmp(line.c_str(), "GET /", 5) == 0 && lineNum == 0) {
					std::size_t s1 = line.find_first_of(' ');
					std::size_t s2 = line.find_last_of(' ');
					std::string ver = line.substr(s2 + 1);
					ver.erase(ver.end() - 1, ver.end());

					(*map)["_SERVER_METHOD"] = line.substr(0, s1);
					(*map)["_SERVER_PATH"] = line.substr(s1 + 1, s2 - 4);
					(*map)["_SERVER_HTTPVER"] = ver;
				} else if(memcmp(line.c_str(), "POST", 4) == 0 && lineNum == 0) {
					// Why is this happening? wtf
					return false;
				} else {
					if(lineNum > 0) {
						std::size_t spl = line.find_first_of(':');
						std::string a1 = line.substr(0, spl);
						std::string a2 = line.substr(spl + 1);

						if(a2.c_str()[0] == ' ') {
							a2 = a2.substr(1); //remove space after :
							a2.erase(a2.end() - 1, a2.end()); //erase \r
						}

						(*map)[a1] = a2;
					}
				}

				lineNum++;
			}

			// Incorrect format of headers no matter what
			if(lineNum < 3) {
				return false;
			}

			if((*map).empty() || (*(*map).begin()).first.compare("_SERVER_METHOD") != 0) {
				printf("INVALID HEADER: NO SERVER METHOD\n");

				return false; //if _SERVER_METHOD isn't found, the method isn't GET, so reject it.
			}

			return true;
		}

		bool readPacket(WebSocketClient* cl, WebSocketMsgReader::WebSocketPacket* msg) {
			if(cl == NULL || msg == NULL)
				return false;

			char* data = new char[bufferSize];

			if(data == NULL) {
				return false;
			}

			int r = recv(cl->socket, data, bufferSize, 0);

			if(r == -1 || r == 0) {
				printf("readPacket ERROR: recv failure (%i -> %i)\n", r, WSAGetLastError());

				return false;
			}

			bool result = WebSocketMsgReader::readRfc6455(data, r, msg);

			delete[] data;

			return result;
		}

		bool writePacket(WebSocketClient* cl, void* data, size_t size) {
			WebSocketWriter writer;

			if(WebSocketMsgWriter::writeRfc6455(data, size, WebSocketHeader::kText, &writer)) {
				int s = send(cl->socket, (char*) writer.buffer(), writer.size(), 0);

				writer.free();

				return s > 0;
			}

			return false;
		}

		bool writePacket(WebSocketClient* cl, std::string str) {
			return this->writePacket(cl, (void*) str.c_str(), str.length());
		}

		__forceinline std::vector<WebSocketClient> getClients() {
			return clients;
		}

	private:
		SOCKET listenfd;
		struct sockaddr_in server;
		std::vector<WebSocketClient> clients;
		std::string port;
		std::string ipAddress;
		std::string path;
		std::string origin;
		std::string location;
		unsigned long bufferSize;
	};

};