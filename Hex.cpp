#include "Hex.h"
#include "Memory.h"

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

    union Value
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

    // real work function
    static ErrorCode HexStr(const char * str, unsigned int length, Value & val)
    {
        val.uint64_val = 0;
        for (unsigned int i = 0; i < length; ++i)
        {
            auto tmp = static_cast<unsigned int>(hex_map[str[i]]);
            if (-1 == tmp)
                return ErrorCode::FZ_INVALID_PARAM;
            val.uint64_val = tmp + (val.uint64_val << 4);
        }
        return ErrorCode::FZ_SUCCESS;
    }

    ErrorCode Hex::HexStr(const char * str, unsigned int length, uint64_t & val)
    {
        ErrorCode ret;
        Value res;
        if (ErrorCode::FZ_SUCCESS == (ret = flyzero::HexStr(str, length, res)))
            val = res.uint64_val;
        return ret;
    }

    ErrorCode Hex::HexStr(const char * str, unsigned int length, uint32_t & val)
    {
        ErrorCode ret;
        Value res;
        if (ErrorCode::FZ_SUCCESS == (ret = flyzero::HexStr(str, length, res)))
            val = res.uint32_val;
        return ret;
    }

    ErrorCode Hex::HexStr(const char * str, unsigned int length, uint8_t & val)
    {
        ErrorCode ret;
        Value res;
        if (ErrorCode::FZ_SUCCESS == (ret = flyzero::HexStr(str, length, res)))
            val = res.uint8_val;
        return ret;
    }

    ErrorCode Hex::HexStr(const char * str, unsigned int length, void * buffer, unsigned int & size)
    {
        if (length / 2 > size)
            return ErrorCode::FZ_NO_BUFFER;

        const char *in = str;
        char *out = (char *)buffer;
        /*
                while (length >= 16)
                {
                    uint64_t res;
                    HexStr(in, 16, res);
                    *((uint64_t *)out) = Memory::ReverseByteOrder(res);

                    in += 16;
                    length -= 16;
                    out += 8;
                }

                while (length >= 8)
                {
                    uint32_t res;
                    HexStr(in, 16, res);
                    *((uint32_t *)out) = Memory::ReverseByteOrder(res);

                    in += 8;
                    length -= 8;
                    out += 4;
                }
        */
        while (length > 1)
        {
            uint8_t res;
            HexStr(in, 2, res);
            *((uint8_t *)out) = res;

            in += 2;
            length -= 2;
            ++out;
        }

        if (0 == length)
        {
            size = out - (char *)buffer;
            return ErrorCode::FZ_SUCCESS;
        }
        else
            return ErrorCode::FZ_INVALID_PARAM;
    }
}
