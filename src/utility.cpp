#include "utility.h"

#include <cassert>
#include <cstdarg>

namespace flyzero {
namespace utility {

std::system_error system_error(int err, const char* format, ...) {
    char    buf[4096];
    va_list ap;
    va_start(ap, format);
    auto const ret [[maybe_unused]] = vsnprintf(buf, sizeof(buf), format, ap);
    assert(ret > 0);
    va_end(ap);
    return std::system_error{err, std::system_category(), buf};
}

}  // namespace utility
}  // namespace flyzero