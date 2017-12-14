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
#include <array>

namespace flyzero
{

    class mempool
    {
        struct chunk;

        struct block_node
        {
            chunk * chunk;
            block_node * prev;
            block_node * next;
            std::size_t checksum;

            std::size_t calc_checksum()
            {
                return checksum = reinterpret_cast<std::size_t>(chunk) ^ reinterpret_cast<std::size_t>(this);
            }

            bool check() const
            {
                return checksum == (reinterpret_cast<std::size_t>(chunk) ^ reinterpret_cast<std::size_t>(this));
            }
        };

        class block_list
        {
        public:
            bool empty() const
            {
                return (head_ == nullptr);
            }

            block_node * pop(void);

            void push(block_node * node);

            void detach(block_node * node);

            std::size_t length() const
            {
                return length_;
            }

        private:
            block_node * head_{ nullptr };

            std::size_t length_{ 0 };
        };

        struct chunk
        {
            unsigned int block_size;
            block_list free_list;
            block_list used_list;

            std::size_t get_impl_size() const
            {
                return sizeof (block_node) + block_size + sizeof (std::size_t);
            }
        };

    public:
        mempool(void * m_beg, void * m_end) :
            beg_(static_cast<unsigned char *>(m_beg)),
            end_(static_cast<unsigned char *>(m_end)),
            cur_(static_cast<unsigned char *>(m_beg))
        {
            init();
        }

        ~mempool()
        {
        }

        void * alloc(std::size_t size);

        void free(void * ptr);

        std::size_t avialable_size() const
        {
            return end_ - cur_;
        }

        static double usage_ratio();

    protected:
        void init();

        std::size_t find_chunk_id(const std::size_t size) const;

    private:
        std::array<chunk, 29> chunks_;
        unsigned char * beg_;
        unsigned char * end_;
        unsigned char * cur_;
    };

}
