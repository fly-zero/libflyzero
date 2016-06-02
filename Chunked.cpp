#include "Chunked.h"
#include "Hex.h"
#include "Memory.h"

namespace FlyZero
{

	Chunked::Chunked(void) :
		last_chunked_length(0), last_handled_length(0), handler(nullptr), user_data(nullptr)
	{
	}

	ErrorCode Chunked::Parse(const void * src, unsigned int srclen)
	{
		const char *chunked_data = (const char *)src;
		unsigned int chunked_len = srclen;
		unsigned int pos;
		ErrorCode err;

		while (chunked_len > 0)
		{
			// check whether last chunked finished or not
			if (last_chunked_length != last_handled_length)
			{ // last chunked data continue
				unsigned int neededlen = last_chunked_length - last_handled_length;
				if (chunked_len <= neededlen)
				{ // still unfinished chunked data
					handler(chunked_data, chunked_len, user_data);
					last_handled_length += chunked_len;
					return ErrorCode::FZ_SUCCESS;;
				} else
				{ // finally, chunked data finished
					handler(chunked_data, neededlen, user_data);
					last_handled_length += neededlen;
					chunked_data += neededlen;
					chunked_len -= neededlen;
					continue;
				}
			} else
			{ // new chunked following
			  // skip leading empty lines
				while (0 == (pos = Memory::SearchCharacters(chunked_data, chunked_len, "\r\n ", 3)))
				{
					++chunked_data;
					--chunked_len;
				}

				// get chunked data length
				uint32_t length;
				if (ErrorCode::FZ_SUCCESS != (err = Hex::HexStr(chunked_data, pos, length)))
					return err;
				else if (0 == length)
					return ErrorCode::FZ_SUCCESS;

				// skip length characters
				chunked_data += pos;
				chunked_len -= pos;

				// skip empty lines following after chunked length
				while (0 == (pos = Memory::SearchCharacters(chunked_data, chunked_len, "\r\n", 2)))
				{
					++chunked_data;
					--chunked_len;
				}

				if (chunked_len <= length)
				{
					handler(chunked_data, chunked_len, user_data);
					last_chunked_length = length;
					last_handled_length = chunked_len;
					return ErrorCode::FZ_SUCCESS;
				} else
				{
					handler(chunked_data, length, user_data);
					last_chunked_length = length;
					last_handled_length = length;
					chunked_data += length;
					chunked_len -= length;
				}
			}
		}
	}

	ErrorCode Chunked::Parse(const void * src, unsigned int srclen, chunked_handler_t chunked_handler, void *data)
	{
		const char *chunked_data = (const char *)src;
		unsigned int chunked_len = srclen;
		unsigned int pos;
		ErrorCode err;

		while (chunked_len > 0)
		{
			// check whether last chunked finished or not
			if (last_chunked_length != last_handled_length)
			{ // last chunked data continue
				unsigned int neededlen = last_chunked_length - last_handled_length;
				if (chunked_len <= neededlen)
				{ // still unfinished chunked data
					chunked_handler(chunked_data, chunked_len, data);
					last_handled_length += chunked_len;
					return ErrorCode::FZ_SUCCESS;;
				}
				else
				{ // finally, chunked data finished
					chunked_handler(chunked_data, neededlen, data);
					last_handled_length += neededlen;
					chunked_data += neededlen;
					chunked_len -= neededlen;
					continue;
				}
			}
			else
			{ // new chunked following
				// skip leading empty lines
				while (0 == (pos = Memory::SearchCharacters(chunked_data, chunked_len, "\r\n ", 3)))
				{
					++chunked_data;
					--chunked_len;
				}

				// get chunked data length
				uint32_t length;
				if (ErrorCode::FZ_SUCCESS != (err = Hex::HexStr(chunked_data, pos, length)))
					return err;
				else if (0 == length)
					return ErrorCode::FZ_SUCCESS;

				// skip length characters
				chunked_data += pos;
				chunked_len -= pos;

				// skip empty lines following after chunked length
				while (0 == (pos = Memory::SearchCharacters(chunked_data, chunked_len, "\r\n", 2)))
				{
					++chunked_data;
					--chunked_len;
				}

				if (chunked_len <= length)
				{
					chunked_handler(chunked_data, chunked_len, data);
					last_chunked_length = length;
					last_handled_length = chunked_len;
					return ErrorCode::FZ_SUCCESS;
				}
				else
				{
					chunked_handler(chunked_data, length, data);
					last_chunked_length = length;
					last_handled_length = length;
					chunked_data += length;
					chunked_len -= length;
				}
			}
		}
	}

}
