#pragma once

#include <vector>
#include <map>

#include "tcp/windows/Platform.h"
#include "tcp/windows/Winsock2.h"

namespace c11http {
namespace tcp {
namespace windows {

class ServerConnection;

/**
 * Predicate used to find ServerConnection objects when using std::find
 */
struct ServerConnectionFinder {
	explicit ServerConnectionFinder(const HANDLE _handle) :
			mHandle(_handle) {
	}
	bool operator()(const ServerConnection* rhs) const;

	HANDLE mHandle;
};

/**
 * Container of ServerConnection objects. Allows connections to be added, removed, and retrieved
 * by specific paramters.
 */
class TCP_WINDOWS_API Connections {
public:
	Connections();
	/**
	 * Get a set of handles to be used by WaitForMultipleObjects. Server handle always is listed
	 * first, such that new connections are handled first. Rotates handles to prevent starvation of
	 * connections.
	 */
	std::vector<WSAEVENT> getHandleArray(WSAEVENT serverHandle);

	void removeServerConnection(const HANDLE handle);
	void removeServerConnection(const std::string& identifier);

	/**
	 * Retrieve a server connection by the file descriptor utilized by the connection.
	 */
	ServerConnection* getServerConnection(const HANDLE handle) const
			throw (std::runtime_error);
	/**
	 * Retrieve a server connection by the identifier associated with the connection.
	 */
	ServerConnection* getServerConnection(const std::string& identifier) const
			throw (std::runtime_error);

	void addServerConnection(ServerConnection* client,
			const std::string& identifier);

	~Connections();

	const size_t size() const;
	std::vector<ServerConnection*> getConnections() const;
private:
	std::vector<ServerConnection*> mContainer;
	std::map<std::string, ServerConnection*> mMapping;

};

}
}
}
