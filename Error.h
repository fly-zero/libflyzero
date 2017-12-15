#pragma once

namespace flyzero
{
    enum class errcode
    {
        ok = 0,
        no_buffer,			// 提供的缓冲区空间不足
        no_memory,			// 内存不足
        invalid_param		// 无效参数		
    };
}