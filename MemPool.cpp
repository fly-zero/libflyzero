#include <exception>
#include <new>
#include "MemPool.h"

void * MemPool::alloc(size_t size)
{
    size_t idx = findChunkId(size);
    if (idx != -1)
    {
        Chunk & chunk = m_chunk[idx];
        BlockList & free = chunk.free;
        BlockList & used = chunk.used;
        size_t size;

        if (!free.empty())
        {
            BlockNode * block = free.pop();
            used.push(block);
            return &block[1];
        }
        else if (size = chunk.blockSize + sizeof (BlockNode) + sizeof (size_t), avialableSize() >= size)
        {
            BlockNode * block = (BlockNode *)(m_cur);
            block->chunk = &chunk;
            used.push(block);
            *((size_t *)(m_cur + sizeof(BlockNode) + chunk.blockSize)) = block->calcChecksum();
            m_cur += size;
            return &block[1];
        }
    }
    throw std::bad_alloc();
}

void MemPool::free(void * ptr)
{
    BlockNode * block = (BlockNode *)((unsigned char *)(ptr) - sizeof (BlockNode));
    if (block->check())
    {
        Chunk & chunk = *block->chunk;
        if (block->checksum == *(size_t *)((unsigned char *)(ptr) + chunk.blockSize))
        {
            chunk.used.detach(block);
            chunk.free.push(block);
        }
        else
        {
            // check failed
        }
    }
    else
    {
        // check failed!
    }
}

#ifdef MEMPOOL_STATISTICS
double MemPool::usageRatio(void)
{
	return 0.0;
}
#endif // MEMPOOL_STATISTICS

void MemPool::init(void)
{
    for (auto i = 0; i < sizeof(m_chunk) / sizeof (m_chunk[0]); ++i)
        m_chunk[i].blockSize = (size_t(1) << i) * sizeof (void *);
}

size_t MemPool::findChunkId(size_t size)
{
    for (size_t i = 0; i < sizeof(m_chunk) / sizeof (m_chunk[0]); ++i)
        if (size <= m_chunk[i].blockSize)
            return i;
    return -1;
}

MemPool::BlockNode * MemPool::BlockList::pop(void)
{
	BlockNode * ret = m_head;
	m_head = m_head->next;
	if (m_head)
		m_head->prev = nullptr;

#ifdef MEMPOOL_STATISTICS
	--m_length;
#endif // MEMPOOL_STATISTICS

	return ret;
}

void MemPool::BlockList::push(MemPool::BlockNode * node)
{
	node->prev = nullptr;
	node->next = m_head;
	if (m_head)
		m_head->prev = node;
	m_head = node;

#ifdef MEMPOOL_STATISTICS
	++m_length;
#endif // MEMPOOL_STATISTICS
}

void MemPool::BlockList::detach(MemPool::BlockNode * node)
{
	BlockNode * prev = node->prev;
	BlockNode * next = node->next;
	
	if (m_head == node && !prev)
		pop();
	else
	{
		prev->next = next;
		next->prev = prev;

#ifdef MEMPOOL_STATISTICS
		--m_length;
#endif // MEMPOOL_STATISTICS
	}
}
