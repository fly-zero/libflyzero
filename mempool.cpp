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

        const auto idx = find_block_manager(size);

        void * ret = nullptr;

        if (idx != -1)
        {
            auto & manager = managers_[idx];
            ret = manager.free_list.empty() ? alloc_new(manager) : alloc_exist(manager);
        }

        return ret;
    }

    void mempool::free(void * ptr)
    {
        const auto block = reinterpret_cast<block_header *>(reinterpret_cast<unsigned char *>(ptr)-sizeof (block_header));
        if (block->check())
        {
            auto & manager = *block->mgr;
            if (block->checksum == *reinterpret_cast<std::size_t *>(reinterpret_cast<unsigned char *>(ptr)+manager.block_size))
            {
                manager.used_list.detach(block);
                manager.free_list.push(block);
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
        for (auto i = 0; i < managers_.size(); ++i)
            managers_[i].block_size = std::size_t(8) << i;
    }

    std::size_t mempool::find_block_manager(const std::size_t size) const
    {
        for (std::size_t i = 0; i < managers_.size(); ++i)
            if ((8 << i) > size)
                return i;
        return -1;
    }

    void * mempool::alloc_new(block_manager & manager)
    {
        void * ret = nullptr;
        const auto impl_size = manager.get_impl_size();

        if (free_size() >= impl_size)
        {
            const auto block = reinterpret_cast<block_header *>(cur_);
            cur_ += impl_size;
            block->mgr = &manager;
            manager.used_list.push(block);
            block->checksum = block->calc_checksum();
            *reinterpret_cast<std::size_t *>(cur_ - sizeof (std::size_t)) = block->checksum;
            ret = &block[1];
        }

        return ret;
    }

    mempool::block_header * mempool::block_list::pop()
    {
        const auto ret = head_;
        head_ = head_->next;
        if (head_)
            head_->prev = nullptr;
        --length_;
        return ret;
    }

    void mempool::block_list::push(block_header * node)
    {
        node->prev = nullptr;
        node->next = head_;
        if (head_)
            head_->prev = node;
        head_ = node;
        ++length_;
    }

    void mempool::block_list::detach(block_header * node)
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
