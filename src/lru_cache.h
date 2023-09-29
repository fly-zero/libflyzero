#pragma once

#include <algorithm>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/list_hook.hpp>
#include <boost/intrusive/options.hpp>
#include <boost/intrusive/unordered_set.hpp>
#include <boost/intrusive/unordered_set_hook.hpp>
#include <chrono>
#include <utility>

namespace flyzero {

template <typename K, typename V, typename H = std::hash<K>,
          typename E = std::equal_to<K>,
          typename A = std::allocator<std::pair<K, V>>>
class LruCache {
    using ListHook = boost::intrusive::list_member_hook<>;
    using HashHook = boost::intrusive::unordered_set_member_hook<>;
    using TimePoint = std::chrono::steady_clock::time_point;

    struct Node {
        Node(TimePoint deadline, K key, V value);

        ~Node() = default;

        /**
         * @brief 禁止拷贝 & 移动
         */
        Node(Node &&) = delete;
        Node(Node const &) = delete;
        void operator=(Node const &) = delete;
        void operator=(Node &&) = delete;

        ListHook list_hook_;  ///< 链表节点钩子
        HashHook hash_hook_;  ///< 哈希表节点钩子
        TimePoint deadline_;  ///< 超时时间
        K key_;               ///< 键
        V value_;             ///< 值
    };

    using Allocator =
        typename std::allocator_traits<A>::template rebind_alloc<Node>;

    struct Hash : public H {
        using H::operator();
        std::size_t operator()(const Node &node) const;
    };

    struct Equal : public E {
        using E::operator();

        template <typename U>
        bool operator()(const U &lhs, const Node &rhs) const;

        template <typename U>
        bool operator()(const Node &lhs, const U &rhs) const;
    };

    using List = boost::intrusive::list<
        Node, boost::intrusive::member_hook<Node, ListHook, &Node::list_hook_>,
        boost::intrusive::constant_time_size<true>>;

    using UnorderedSet = boost::intrusive::unordered_set<
        Node, boost::intrusive::member_hook<Node, HashHook, &Node::hash_hook_>,
        boost::intrusive::equal<Equal>, boost::intrusive::hash<Hash>,
        boost::intrusive::power_2_buckets<true>,
        boost::intrusive::store_hash<true>,
        boost::intrusive::constant_time_size<false>>;

    using BucketTraits = typename UnorderedSet::bucket_traits;

public:
    using Duration = std::chrono::steady_clock::duration;
    using Iterator = typename List::iterator;
    using ConstIterator = typename List::const_iterator;

    /**
     * @brief 构造函数
     */
    explicit LruCache(Duration timeout);

    /**
     * @brief 析构函数
     */
    ~LruCache();

    /**
     * @brief 禁止拷贝
     */
    LruCache(LruCache const &) = delete;
    void operator=(LruCache const &) = delete;

    /**
     * @brief 移动构造函数
     */
    LruCache(LruCache &&) = default;

    /**
     * @brief 移动赋值
     */
    LruCache &operator=(LruCache &&) = default;

    /**
     * @brief 获取开始迭代器
     */
    Iterator begin() { return list_.begin(); }

    /**
     * @brief 获取结束迭代器
     */
    Iterator end() { return list_.end(); }

    /**
     * @brief 获取开始迭代器
     */
    ConstIterator begin() const { return list_.cbegin(); }

    /**
     * @brief 获取结束迭代器
     */
    ConstIterator end() const { return list_.cend(); }

    /**
     * @brief 获取元素数量
     */
    std::size_t size() const { return list_.size(); }

    /**
     * @brief 判断是否为空
     */
    bool empty() const { return list_.empty(); }

    /**
     * @brief 查找元素
     * @param key 键
     * @return Iterator 元素迭代器
     */
    Iterator find(K const &key);

    /**
     * @brief 异构查找元素
     * @tparam U 异构键
     * @param key 键
     * @return Iterator 元素迭代器
     */
    template <typename U>
    Iterator find(U const &key);

    /**
     * @brief 查找元素
     * @param key 键
     * @return ConstIterator 元素迭代器
     */
    ConstIterator find(K const &key) const;

    /**
     * @brief 异构查找元素
     * @tparam U 异构键
     * @param key 键
     * @return Iterator 元素迭代器
     */
    template <typename U>
    ConstIterator find(U const &key) const;

    /**
     * @brief 插入元素
     * @param now 当前时间
     * @param key 键
     * @param value 值
     * @return Iterator 元素迭代器
     */
    std::pair<Iterator, bool> insert(TimePoint now, K key, V value);

    /**
     * @brief 移除元素
     * @param it 元素迭代器
     */
    void erase(Iterator it);

    /**
     * @brief 更新元素访问时间
     */
    void touch(TimePoint now, Iterator it);

protected:
    /**
     * @brief 分配哈希表桶
     * @param n 桶数量
     * @return BucketTraits 桶数组
     */
    static BucketTraits alloc_buckets(std::size_t n);

private:
    Duration timeout_;                      ///< 超时时间
    List list_;                             ///< 链表
    UnorderedSet hash_{alloc_buckets(16)};  ///< 哈希表
};

template <typename K, typename V, typename H, typename E, typename A>
LruCache<K, V, H, E, A>::Node::Node(TimePoint deadline, K key, V value)
    : deadline_(deadline), key_(std::move(key)), value_(std::move(value)) {}

template <typename K, typename V, typename H, typename E, typename A>
std::size_t LruCache<K, V, H, E, A>::Hash::operator()(const Node &node) const {
    return this->operator()(node.key_);
}

template <typename K, typename V, typename H, typename E, typename A>
template <typename U>
bool LruCache<K, V, H, E, A>::Equal::operator()(const U &lhs,
                                                const Node &rhs) const {
    return this->operator()(lhs, rhs.key_);
}

template <typename K, typename V, typename H, typename E, typename A>
template <typename U>
bool LruCache<K, V, H, E, A>::Equal::operator()(const Node &lhs,
                                                const U &rhs) const {
    return this->operator()(lhs.key_, rhs);
}

template <typename K, typename V, typename H, typename E, typename A>
LruCache<K, V, H, E, A>::LruCache(Duration timeout) : timeout_(timeout) {}

template <typename K, typename V, typename H, typename E, typename A>
LruCache<K, V, H, E, A>::~LruCache() {
    list_.clear();
    hash_.clear_and_dispose([](Node *node) { delete node; });
}

template <typename K, typename V, typename H, typename E, typename A>
auto LruCache<K, V, H, E, A>::find(const K &key) -> Iterator {
    auto const it = hash_.find(key);
    if (it == hash_.end()) return list_.end();
    auto &node = *it;
    auto const list_it = list_.iterator_to(node);
    return list_it;
}

template <typename K, typename V, typename H, typename E, typename A>
template <typename U>
auto LruCache<K, V, H, E, A>::find(const U &key) -> Iterator {
    auto const it = hash_.find(key, Hash{}, Equal{});
    if (it == hash_.end()) return list_.end();
    auto &node = *it;
    auto const list_it = list_.iterator_to(node);
    return list_it;
}

template <typename K, typename V, typename H, typename E, typename A>
auto LruCache<K, V, H, E, A>::find(const K &key) const -> ConstIterator {
    auto const it = hash_.find(key);
    if (it == hash_.end()) return list_.cend();
    auto &node = *it;
    auto const list_it = list_.iterator_to(node);
    return list_it;
}

template <typename K, typename V, typename H, typename E, typename A>
template <typename U>
auto LruCache<K, V, H, E, A>::find(const U &key) const -> ConstIterator {
    auto const it = hash_.find(key, Hash{}, Equal{});
    if (it == hash_.end()) return list_.end();
    auto &node = *it;
    auto const list_it = list_.iterator_to(node);
    return list_it;
}

template <typename K, typename V, typename H, typename E, typename A>
auto LruCache<K, V, H, E, A>::insert(TimePoint now, K key, V value)
    -> std::pair<Iterator, bool> {
    std::pair<Iterator, bool> ret;
    typename UnorderedSet::insert_commit_data commit_data;
    auto const [it, inserted] =
        hash_.insert_check(key, Hash{}, Equal{}, commit_data);
    if (!inserted) {
        // 结点已存在，返回已存在的结点
        auto const list_it = list_.iterator_to(*it);
        ret = {list_it, false};
    } else {
        // 结点不存在，创建新结点
        auto const ptr = Allocator{}.allocate(1);
        auto const node =
            new (ptr) Node(now + timeout_, std::move(key), std::move(value));
        list_.push_back(*node);
        hash_.insert_commit(*node, commit_data);
        ret = {list_.iterator_to(*node), true};

        // 扩容
        if (size() >= hash_.bucket_count()) [[unlikely]] {
            auto const buckets = hash_.bucket_pointer();
            auto const count = hash_.bucket_count();
            hash_.rehash(alloc_buckets(count * 2));
            delete[] buckets;
        }
    }
    return ret;
}

template <typename K, typename V, typename H, typename E, typename A>
void LruCache<K, V, H, E, A>::erase(Iterator it) {
    auto &node = *it;
    hash_.erase(hash_.iterator_to(node));
    list_.erase(it);
    node.~Node();
    Allocator{}.deallocate(&node, 1);
}

template <typename K, typename V, typename H, typename E, typename A>
void LruCache<K, V, H, E, A>::touch(TimePoint now, Iterator it) {
    auto &node = *it;
    node.deadline_ = now + timeout_;
    list_.splice(list_.end(), list_, it);
}

template <typename K, typename V, typename H, typename E, typename A>
auto LruCache<K, V, H, E, A>::alloc_buckets(std::size_t n) -> BucketTraits {
    auto const buckets = new typename UnorderedSet::bucket_type[n];
    return {buckets, n};
}

}  // namespace flyzero