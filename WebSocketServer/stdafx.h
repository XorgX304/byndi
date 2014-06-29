// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <hash_map>
#include <sstream>
#include <winsock2.h>
#include <stdint.h>

#pragma comment(lib, "ws2_32")

#include "sha1.h"
#include "base64.h"
#include "picojson.h"

#include "WebSocketHeader.h"
#include "WebSocketReader.h"
#include "WebSocketWriter.h"
#include "WebSocketMsg.h"
#include "WebSocketClient.h"
#include "WebSocketServer.h"