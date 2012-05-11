#include "objects/HttpResponse.h"

namespace c11http {
   namespace objects {

      HttpResponse::HttpResponse(const std::string& body) : mBody(body) {

      }
      HttpResponse::~HttpResponse() {

      }

      const std::string& HttpResponse::getBody() const {
         return mBody;
      }

   }
}