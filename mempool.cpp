#include <exception>
#include <new>
#include "mempool.h"

namespace flyzero
{

    inline std::size_t align8(const std::size_t n)
    {
        return (n + 7) & ~7;
    }

    void * mempool::alloc(std::size_t size)
    {
        size = align8(size);

        const auto idx = find_chunk_id(size);

        if (idx != -1)
        {
            auto & chunk = chunks_[idx];
            auto & free = chunk.free_list;
            auto & used = chunk.used_list;

            if (!free.empty())
            {
                const auto block = free.pop();
                used.push(block);
                return &block[1];
            }

            const auto impl_size = chunk.get_impl_size();

            if (avialable_size() >= impl_size)
            {
                auto block = reinterpret_cast<block_node *>(cur_);
                block->chunk = &chunk;
                used.push(block);
                *reinterpret_cast<std::size_t *>(cur_ + sizeof(block_node) + chunk.block_size) = block->calc_checksum();
                cur_ += size;
                return &block[1];
            }
        }

        return nullptr;
    }

    void mempool::free(void * ptr)
    {
        const auto block = reinterpret_cast<block_node *>(reinterpret_cast<unsigned char *>(ptr)-sizeof (block_node));
        if (block->check())
        {
            chunk & chunk = *block->chunk;
            if (block->checksum == *reinterpret_cast<std::size_t *>(reinterpret_cast<unsigned char *>(ptr)+chunk.block_size))
            {
                chunk.used_list.detach(block);
                chunk.free_list.push(block);
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

    double mempool::usage_ratio()
    {
        return 0.0;
    }

    void mempool::init()
    {
        for (auto i = 0; i < chunks_.size(); ++i)
            chunks_[i].block_size = std::size_t(1) << (i + 3);
    }

    std::size_t mempool::find_chunk_id(const std::size_t size) const
    {
        for (std::size_t i = 0; i < chunks_.size(); ++i)
            if (size <= chunks_[i].block_size)
                return i;
        return -1;
    }

    mempool::block_node * mempool::block_list::pop(void)
    {
        const auto ret = head_;
        head_ = head_->next;
        if (head_)
            head_->prev = nullptr;
        --length_;
        return ret;
    }

    void mempool::block_list::push(block_node * node)
    {
        node->prev = nullptr;
        node->next = head_;
        if (head_)
            head_->prev = node;
        head_ = node;
        ++length_;
    }

    void mempool::block_list::detach(block_node * node)
    {
        const auto prev = node->prev;
        const auto next = node->next;

        if (head_ == node && !prev)
            pop();
        else
        {
            prev->next = next;
            next->prev = prev;
            --length_;
        }
    }

}
