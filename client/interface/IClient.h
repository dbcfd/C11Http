#pragma once

#include "client/interface/Platform.h"

#include <functional>

namespace c11http {

namespace objects {
class HttpRequest;
class HttpResponse;
}

namespace client {

class CLIENT_INTERFACE_API IClient {
public:
   typedef std::function<void(const objects::HttpResponse&)> ClientResponseCallback;

   IClient(const ClientResponseCallback& callback);

   virtual objects::HttpResponse sendRequestToServer(const objects::HttpRequest& req) = 0;
   virtual void connectToServer(const std::string& ip, const unsigned int port) = 0;
   virtual void disconnect() = 0;

   void receiveResponseFromServer(const objects::HttpResponse& resp);
   void connectToServer(const std::string& ip, const unsigned int port);
protected:
   virtual void performServerConnection(const std::string& ip, const unsigned int port) = 0;

private:
   ClientResponseCallback mCallback;
};

}
}