#pragma once

#include "objects/Platform.h"

namespace c11http {
namespace objects {

class OBJECTS_API HttpResponse {
public:
   HttpResponse(const std::string& body = "");
   ~HttpResponse();

   const std::string& getBody() const;
private:
   std::string mBody;
};

}
}