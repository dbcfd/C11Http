#pragma once

#include "objects/Platform.h"

#include <functional>

namespace c11http {
namespace objects {

class OBJECTS_API HttpRequest;
class OBJECTS_API HttpResponse;

typedef std::function<c11http::objects::HttpResponse(c11http::objects::HttpRequest)> HttpRequestToResponse;

}
}