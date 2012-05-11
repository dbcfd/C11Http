#include "client/interface/IClient.h"

namespace c11http {
namespace client {

IClient::IClient(const IClient::ClientResponseCallback& callback) : mCallback(callback) {

}

void IClient::receiveResponseFromServer(const objects::HttpResponse& resp) {
   mCallback(resp);
}

}
}