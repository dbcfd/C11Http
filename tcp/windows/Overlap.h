#pragma once

#include <sstream>

#include "tcp/windows/Platform.h"
#include "tcp/windows/Winsock2.h"

namespace c11http {
namespace tcp {
namespace windows {

class TCP_WINDOWS_API OverlapCreationException: public std::runtime_error
{
public:
    OverlapCreationException(const std::string& what)
            : std::runtime_error(what)
    {
    }
};

/**
 * Overlaps are used by Winsock2 asynchronous calls to provide notifications when events occur,
 * such as a send or recv completing, or a client attempting to connect. Overlaps must be
 * created in a specific way that not only allocates memory appropriately, but also prepares
 * event handles to check for status by WaitForMultipleEvents. To fully describe connections
 * associated with overlaps, the overlap structure must be extended.
 */
template<class T>
class Overlap
{
public:
    template<class T>
    struct OverlapExtend: public WSAOVERLAPPED
    {
        T* derived;
    };
    typedef OverlapExtend<T> OverlapType;
    Overlap()
            : mOverlap(NULL)
    {
    }
    Overlap(T* derived) throw (OverlapCreationException)
    {
        createOverlap(derived);
    }
    virtual ~Overlap()
    {
        WSACloseEvent(mOverlap->hEvent);
        mOverlap->hEvent = WSA_INVALID_EVENT;

        if (mOverlap != NULL)
        {
            HeapFree(GetProcessHeap(), 0, mOverlap);
            mOverlap = NULL;
        }
    }

    inline OverlapType* getOverlap() const
    {
        return mOverlap;
    }

protected:
    void createOverlap(T* derived) throw (std::runtime_error)
    {
        //
        // Allocate an overlapped structure.
        // We use the Offset field to keep track of the socket handle
        // we have accepted a connection on, since there is no other
        // way to pass information to GetOverlappedResult()
        //
        OverlapType* cOverlap = (OverlapType*) HeapAlloc(GetProcessHeap(),
                HEAP_ZERO_MEMORY, sizeof(OverlapType));

        //
        // Did the HeapAllocation FAIL??
        //
        if (cOverlap == NULL)
        {
            std::stringstream sstr;
            sstr << "Overlap HeapAlloc()";
            throw(OverlapCreationException(sstr.str()));
        }

        SecureZeroMemory(cOverlap, sizeof(OverlapType));

        if (WSA_INVALID_EVENT == (cOverlap->hEvent = WSACreateEvent()))
        {
            std::stringstream sstr;
            sstr << "WSACreateEvent";
            throw(OverlapCreationException(sstr.str()));
        }

        cOverlap->derived = derived;

        mOverlap = cOverlap;
    }
private:
    OverlapType* mOverlap;
};

}
}
}
