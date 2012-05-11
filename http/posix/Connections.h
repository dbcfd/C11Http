#pragma once

#include <map>
#include <vector>

#include "tcp/posix/Platform.h"
#include "tcp/posix/posix.h"

namespace c11http {
namespace tcp {
namespace posix {

class Callback;
class ServerConnection;

/**
 * Predicate used to find ServerConnection objects when using std::find
 */
struct ServerConnectionFinder
{
    explicit ServerConnectionFinder(const int _sckt)
            : mSocket(_sckt)
    {
    }
    bool operator()(const ServerConnection* rhs) const;
    int mSocket;
};

/**
 * Container of ServerConnection objects. Allows connections to be added, removed, and retrieved
 * by specific paramters.
 */
class TCP_POSIX_API Connections
{
public:
    Connections(fd_set& read);
    ~Connections();

    void addServerConnection(ServerConnection* client);
    void removeServerConnection(const int sckt);
    void clear();
    /**
     * Retrieve a server connection by the identifier associated with the connection.
     */
    ServerConnection* getServerConnection(const std::string& identifier)
            throw (std::runtime_error);
    /**
     * Retrieve a server connection by the file descriptor utilized by the connection.
     */
    ServerConnection* getServerConnection(const int sckt);
    std::vector<ServerConnection*>& getConnections();

    int getMax() const;
    const size_t size() const;
private:
    std::vector<ServerConnection*> mContainer;
    std::map<std::string, ServerConnection*> mMapping;
    fd_set& mRead;
    int mFdMax;
};

}
}
}
