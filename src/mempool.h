#pragma once

/****************************************************************************\
 * Chunk Array:                                                             *
 * +----------+----------+-----------+----------+----------+                *
 * | block_mgr[0] | block_mgr[1] |    ...    | block_mgr[i] |    ...   |                *
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
#include <array>

namespace flyzero
{

    class mempool
    {
        struct block_manager;

        struct block_header
        {
            struct block_manager * mgr;
            block_header * prev;
            block_header * next;
            std::size_t checksum;

            std::size_t calc_checksum() const
            {
                return reinterpret_cast<std::size_t>(mgr) ^ reinterpret_cast<std::size_t>(this);
            }

            bool check() const
            {
                return checksum == calc_checksum();
            }
        };

        class block_list
        {
        public:
            bool empty() const
            {
                return (head_ == nullptr);
            }

            block_header * pop(void);

            void push(block_header * node);

            void detach(block_header * node);

            std::size_t length() const
            {
                return length_;
            }

        private:
            block_header * head_{ nullptr };

            std::size_t length_{ 0 };
        };

        struct block_manager
        {
            unsigned int block_size;
            block_list free_list;
            block_list used_list;

            std::size_t get_impl_size() const
            {
                return sizeof (block_header) + block_size + sizeof (std::size_t);
            }
        };

    public:
        mempool(void * beg, void * end) :
            beg_(static_cast<unsigned char *>(beg)),
            end_(static_cast<unsigned char *>(end)),
            cur_(static_cast<unsigned char *>(beg))
        {
            init();
        }

        ~mempool() = default;

        void * alloc(std::size_t size);

        void free(void * ptr);

        std::size_t free_size() const
        {
            return end_ - cur_;
        }

        static double usage_ratio();

    protected:
        void init();

        std::size_t find_block_manager(const std::size_t size) const;

        void * alloc_new(block_manager & manager);

        static void * alloc_exist(block_manager & manager)
        {
            const auto block = manager.free_list.pop();
            manager.used_list.push(block);
            return &block[1];
        }

    private:
        std::array<block_manager, 29> managers_;
        unsigned char * beg_;
        unsigned char * end_;
        unsigned char * cur_;
    };

}
