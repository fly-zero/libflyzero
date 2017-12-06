#pragma once

namespace flyzero
{
    enum class ErrorCode
    {
        FZ_SUCCESS = 0,
        FZ_NO_BUFFER,			// 提供的缓冲区空间不足
        FZ_NO_MEMORY,			// 内存不足
        FZ_INVALID_PARAM		// 无效参数		
    };
}