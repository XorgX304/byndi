#pragma once

namespace Byndi {
	class WebSocketClient  {
	public:
		SOCKET socket;
		struct sockaddr info;
		struct sockaddr_in ipv4info;
	};
};