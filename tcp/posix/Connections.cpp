#ifndef WINDOWS
#include "tcp/posix/Connections.h"

#include <algorithm>

#include "tcp/posix/ServerConnection.h"

namespace c11http {
namespace tcp {
namespace posix {

bool ServerConnectionFinder::operator()(const ServerConnection* rhs) const
{
    return (mSocket == rhs->getSocket());
}

Connections::Connections(fd_set& _read)
        : mRead(_read), mFdMax(-1)
{

}

void Connections::addServerConnection(ServerConnection* client)
{
    mContainer.push_back(client);
    mMapping[client->getIdentifier()] = client;
    mFdMax = std::max(mFdMax, client->getSocket());
    FD_SET(client->getSocket(), &mRead);
}

void Connections::removeServerConnection(const int sckt)
{
    FD_CLR(sckt, &mRead);
    std::vector<ServerConnection*>::iterator iter = std::remove_if(
            mContainer.begin(), mContainer.end(), ServerConnectionFinder(sckt));

    if (iter != mContainer.end())
    {
        mMapping.erase((*iter)->getIdentifier());
        delete (*iter);

        mContainer.erase(iter);

        mFdMax = -1;

        for (iter = mContainer.begin(); iter != mContainer.end(); ++iter)
        {
        	mFdMax = std::max(mFdMax, (*iter)->getSocket());
        }
    }
}

Connections::~Connections()
{
    clear();
}

std::vector<ServerConnection*>& Connections::getConnections()
{
    return mContainer;
}

ServerConnection* Connections::getServerConnection(const int sckt)
{
    std::vector<ServerConnection*>::iterator iter = std::find_if(
    		mContainer.begin(), mContainer.end(), ServerConnectionFinder(sckt));

    if (iter != mContainer.end())
    {
        return (*iter);
    }
    return 0;
}

ServerConnection* Connections::getServerConnection(
        const std::string& identifier) throw (std::runtime_error)
{
    std::map<std::string, ServerConnection*>::iterator iter = mMapping.find(
            identifier);
    if (iter == mMapping.end())
        throw(std::runtime_error("Connection not found"));
    return iter->second;
}

void Connections::clear()
{
    for (std::vector<ServerConnection*>::iterator iter = mContainer.begin();
            iter != mContainer.end(); ++iter)
    {
        delete (*iter);
    }

    mContainer.clear();
}

int Connections::getMax() const
{
    return mFdMax;
}

const size_t Connections::size() const
{
    return mContainer.size();
}

}
}
}

#endif
