#pragma once

#include "Error.h"

namespace flyzero
{

    class Chunked
    {
    public:
        typedef void (*chunked_handler_t)(const void *buffer, unsigned int buflen, void *data);

    public:
        Chunked(void);
        void SetHandler(const chunked_handler_t handler) { this->handler = handler; }
        void SetUserData(void *data) { user_data = data; }
        ErrorCode Parse(const void * src, unsigned int srclen);
        ErrorCode Parse(const void * src, unsigned int srclen, chunked_handler_t chunked_handler, void *data);

    private:
        unsigned int last_chunked_length;
        unsigned int last_handled_length;
        chunked_handler_t handler;
        void * user_data;
    };

}