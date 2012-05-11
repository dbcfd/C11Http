#include "objects/HttpRequest.h"

namespace c11http {
   namespace objects {

      HttpRequest::HttpRequest(Method reqMethod, const std::string& body) : mReqMethod(reqMethod), mBody(body) {

      }
      HttpRequest::~HttpRequest() {

      }

      HttpRequest::Method HttpRequest::getRequestMethod() const {
         return mReqMethod;
      }
      const std::string& HttpRequest::getBody() const {
         return mBody;
      }

   }
}