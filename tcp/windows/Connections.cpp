#ifdef WINDOWS
#include "tcp/windows/Connections.h"

#include <algorithm>

#include "tcp/windows/ServerConnection.h"

namespace c11http {
namespace tcp {
namespace windows {

bool ServerConnectionFinder::operator()(const ServerConnection* rhs) const
{
    return (mHandle == rhs->getOverlap()->hEvent);
}

Connections::Connections()
{

}
/**
 * rotates handles to prevent starvation of connections
 */
std::vector<WSAEVENT> Connections::getHandleArray(WSAEVENT serverHandle)
{

    std::vector < WSAEVENT > handles;
    handles.reserve(mContainer.size() + 1);
    handles.push_back(serverHandle);

    if(mContainer.size() > 0)
    {
        ServerConnection* client = mContainer.front();
        mContainer.erase(mContainer.begin());
        mContainer.push_back(client);
        std::vector<ServerConnection*>::const_iterator iter = mContainer.begin();
        int i = 0;

        for(iter; iter != mContainer.end(); ++iter)
        {
            handles.push_back((*iter)->getOverlap()->hEvent);
        }
    }

    return handles;
}

void Connections::removeServerConnection(const std::string& identifier)
{
    std::map<std::string, ServerConnection*>::iterator iter = mMapping.find(identifier);

    if(iter != mMapping.end())
    {
        removeServerConnection(iter->second->getOverlap()->hEvent);
    }
}

void Connections::removeServerConnection(const HANDLE handle)
{
    std::vector<ServerConnection*>::iterator iter = std::find_if(mContainer.begin(),
            mContainer.end(), ServerConnectionFinder(handle));

    if(iter != mContainer.end())
    {
        mMapping.erase((*iter)->getIdentifier());
        delete (*iter);
        mContainer.erase(iter);
    }
}

ServerConnection* Connections::getServerConnection(const HANDLE handle) const
        throw(std::runtime_error)
{
    std::vector<ServerConnection*>::const_iterator iter = std::find_if(mContainer.begin(),
            mContainer.end(), ServerConnectionFinder(handle));

    if(iter == mContainer.end())
        throw(std::runtime_error("Failed to find client"));

    return (*iter);
}

ServerConnection* Connections::getServerConnection(const std::string& identifier) const
        throw(std::runtime_error)
{
    std::map<std::string, ServerConnection*>::const_iterator iter = mMapping.find(identifier);

    if(iter == mMapping.end())
        throw(std::runtime_error("Failed to find client"));

    return iter->second;
}

void Connections::addServerConnection(ServerConnection* client, const std::string& identifier)
{
    client->setIdentifier(identifier);
    mMapping[identifier] = client;
    mContainer.push_back(client);
}

Connections::~Connections()
{
    for(std::vector<ServerConnection*>::iterator iter = mContainer.begin();
            iter != mContainer.end(); ++iter)
    {
        delete (*iter);
    }

    mContainer.clear();
    mMapping.clear();
}

const size_t Connections::size() const
{
    return mContainer.size();
}
std::vector<ServerConnection*> Connections::getConnections() const
{
    return mContainer;
}

}
}
}

#endif
