#include "file_descriptor.h"

namespace flyzero
{

    size_t file_descriptor::write(const char * buff, size_t size) const
    {
        for (size_t pos = 0; pos < size; )
        {
            auto const nwrite = ::write(fd_, buff + pos, size - pos);
            if (nwrite == -1)
                return size_t(-1);
            pos += nwrite;
        }

        return size;
    }

}
