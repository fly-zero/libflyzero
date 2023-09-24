#include "hex.h"
#include "memory.h"

namespace flyzero
{
    static const int hex_map[] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
        -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    };

    namespace hex_impl
    {
        union value
        {
            int8_t		int8_val;
            uint8_t		uint8_val;
            int16_t		int16_val;
            uint16_t	uint16_val;
            int32_t		int32_val;
            uint32_t	uint32_val;
            int64_t		int64_val;
            uint64_t	uint64_val;
        };
    }

    // real work function
    static errcode hex_str(const char * str, unsigned int const length, hex_impl::value & val)
    {
        val.uint64_val = 0;
        for (unsigned int i = 0; i < length; ++i)
        {
            auto const tmp = static_cast<unsigned int>(hex_map[str[i]]);
            if (-1 == tmp)
                return errcode::invalid_param;
            val.uint64_val = tmp + (val.uint64_val << 4);
        }
        return errcode::ok;
    }

    errcode hex::hex_str(const char * str, unsigned int const length, uint64_t & val)
    {
        errcode ret;
        hex_impl::value res;
        if (errcode::ok == (ret = flyzero::hex_str(str, length, res)))
            val = res.uint64_val;
        return ret;
    }

    errcode hex::hex_str(const char * str, unsigned int const length, uint32_t & val)
    {
        errcode ret;
        hex_impl::value res;
        if (errcode::ok == (ret = flyzero::hex_str(str, length, res)))
            val = res.uint32_val;
        return ret;
    }

    errcode hex::hex_str(const char * str, unsigned int const length, uint8_t & val)
    {
        errcode ret;
        hex_impl::value res;
        if (errcode::ok == (ret = flyzero::hex_str(str, length, res)))
            val = res.uint8_val;
        return ret;
    }

    errcode hex::hex_str(const char * str, unsigned int length, void * buffer, unsigned int & size)
    {
        if (length / 2 > size)
            return errcode::no_buffer;

        auto in = str;
        auto out = reinterpret_cast<char *>(buffer);

        while (length > 1)
        {
            uint8_t res;
            hex_str(in, 2, res);
            *reinterpret_cast<uint8_t *>(out) = res;

            in += 2;
            length -= 2;
            ++out;
        }

        if (0 == length)
        {
            size = out - reinterpret_cast<char *>(buffer);
            return errcode::ok;
        }

        return errcode::invalid_param;
    }
}
