#include "stdafx.h"

using namespace Byndi;

WebSocketServer server;

/*
# Eventually I'd like to bind all of this into some kind of scripting system
# but for now, this is how it is meant to be used
# I hope you enjoy it! And call it shitty so I can improve it!
# For now, it's a basic chat server with some REAL basic support for commands
# For example "/who" will list all the users on the server.
*/

DWORD WINAPI lpSocketConnectionThread(LPVOID lpParam) {
	WebSocketClient client = *(WebSocketClient*) lpParam;

	while(true) {
		WebSocketMsgReader::WebSocketPacket msg;

		if(server.readPacket(&client, &msg)) {
			printf("RAW [%s]\n", msg.payload.c_str());

			picojson::value v;
			picojson::parse(v, msg.payload.begin(), msg.payload.end());

			if(v.get("type").is<std::string>()) {
				if(v.get("type").to_str().compare("msg") == 0) {
					std::string txt = v.get("data").get("text").to_str();

					printf("text [%s]\n", txt.c_str());

					if(txt.c_str()[0] == '/') {
						if(txt.substr(1).compare("who") == 0) {
							std::stringstream ss;
							ss << "{\"type\":\"server\", \"data\":{\"text\":\"Clients connected: ";
							ss << server.getClients().size();
							ss << "\"}}";

							printf("sending [%s]\n", ss.str().c_str());

							server.writePacket(&client, ss.str());
						} else {
							printf("Invalid command [%s]\n", txt.substr(1).c_str());

							// Yeah... We didn't want to bother with a JSON writer library right now...
							server.writePacket(&client, "{\"type\":\"server\", \"data\":{\"text\":\"Invalid command!\"}}");
						}

						continue; // Nope!
					}
				}
			}

			// Chat Message
			for(size_t i = 0; i < server.getClients().size(); i++) {
				if(server.getClients()[i].socket != client.socket) {
					server.writePacket(&server.getClients()[i], (void*) msg.payload.c_str(), msg.payload.length());
				}
			}
		} else {
			printf("Packet problem?\n");
		}

		Sleep(100);
	}

	return 0;
}

int main(int argc, char* argv[]) {

	WSADATA data;

	if(WSAStartup(MAKEWORD(2, 2), &data) != NO_ERROR) {
		printf("Unable to initialize Winsock2!\n");
		return 0;
	}

	if(server.init("127.0.0.1", "8008", 1024)) {
		if(server.isValidSocket()) {
			while(true) {
				WebSocketClient client;

				if(server.accept(&client)) {
					CreateThread(0, 0, lpSocketConnectionThread, &client, 0, 0);

					printf("Created thread for new client...\n");
				} else {
					printf("Accept failed...\n");
				}
			}
		} else {
			printf("ERROR: Unable to create valid socket!\n");
		}
	} else {
		printf("ERROR: Unable to create server\n");
	}

	return 0;
}