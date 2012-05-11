#pragma once

#include "objects/Platform.h"

namespace c11http {
namespace objects {

class OBJECTS_API HttpRequest {
public :
   enum Method {
      GET,
      POST,
      PUT,
      DELETE
   };
   HttpRequest(Method reqMethod = GET, const std::string& body = "");
   ~HttpRequest();

   Method getRequestMethod() const;
   const std::string& getBody() const;

private:
   Method mReqMethod;
   std::string mBody;
};

}
}