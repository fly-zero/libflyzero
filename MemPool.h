#pragma once

/****************************************************************************\
 * Chunk Array:                                                             *
 * +----------+----------+-----------+----------+----------+                *
 * | chunk[0] | chunk[1] |    ...    | chunk[i] |    ...   |                *
 * +----------+----------+-----------+----------+----------+                *
 *                                                                          *
 * Block:                                                                   *
 * +-----------+----------------------+----------+                          *
 * | BlockNode |   Allocated Memory   | checksum |                          *
 * +-----------+----------------------+----------+                          *
 *                                                                          *
 * Chunk:                                                                   *
 * +------+   +-------+   +-------+                                         *
 * | free |-->| block |-->| block |--> ...                                  *
 * +------+   +-------+   +-------+                                         *
 * | used |																	*
 * +------+                                                                 *
 * | .... |																	*
 * +------+                                                                 *
 \***************************************************************************/

#include <cstdlib>

#define MEMPOOL_STATISTICS

class MemPool
{
	struct Chunk;

	struct BlockNode
	{
		Chunk * chunk;
		BlockNode * prev;
		BlockNode * next;
		size_t checksum;

		size_t calcChecksum(void)
		{
			return (checksum = (reinterpret_cast<size_t>(chunk) ^ reinterpret_cast<size_t>(this)));
		}

		bool check(void) const
		{
			return (checksum == (reinterpret_cast<size_t>(chunk) ^ reinterpret_cast<size_t>(this)));
		}
	};

	class BlockList
	{
	public:
		bool empty(void) const
		{
			return (m_head == nullptr);
		}

		BlockNode * pop(void);

		void push(BlockNode * node);

		void detach(BlockNode * node);

#ifdef MEMPOOL_STATISTICS
		size_t length(void) const
		{
			return m_length;
		}
#endif // MEMPOOL_STATISTICS

	private:
		BlockNode * m_head { nullptr };

#ifdef MEMPOOL_STATISTICS
		size_t m_length { 0 };
#endif // MEMPOOL_STATISTICS
	};

	struct Chunk
	{
		unsigned int blockSize;
		BlockList free;
		BlockList used;
	};

public:
    MemPool(void * m_beg, void * m_end) :
        m_beg(static_cast<unsigned char *>(m_beg)),
        m_end(static_cast<unsigned char *>(m_end)),
        m_cur(static_cast<unsigned char *>(m_beg))
    {
		init();
    }

    ~MemPool(void)
    {
    }

    void * alloc(size_t size);

    void free(void * ptr);

    size_t avialableSize(void) const
    {
        return (m_end - m_cur);
    }

#ifdef MEMPOOL_STATISTICS
	double usageRatio(void);
#endif // MEMPOOL_STATISTICS

private:
    void init(void);

    size_t findChunkId(size_t size);

private:
    Chunk m_chunk[29];
    unsigned char * m_beg;
    unsigned char * m_end;
    unsigned char * m_cur;
};

