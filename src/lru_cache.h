#pragma once

#include <algorithm>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/list_hook.hpp>
#include <boost/intrusive/options.hpp>
#include <boost/intrusive/unordered_set.hpp>
#include <boost/intrusive/unordered_set_hook.hpp>
#include <chrono>
#include <concepts>
#include <utility>

namespace flyzero {

/**
 * @brief LRU 结点基类
 */
struct LruNodeBase : boost::intrusive::list_base_hook<>,
                     boost::intrusive::unordered_set_base_hook<> {
    /**
     * @brief 时间点
     */
    using TimePoint = std::chrono::steady_clock::time_point;

    /**
     * @brief 构造 LruNodeBase
     * @param deadline 超时时间
     */
    explicit LruNodeBase(TimePoint deadline);

    TimePoint deadline_;  ///< 超时时间
};

/**
 * @brief LRU 缓存
 *
 * @tparam K 键类型
 * @tparam V 值类型
 * @tparam H 哈希函数类型
 * @tparam E 比较函数类型
 * @tparam A 分配器类型
 */
template <typename K, typename V, typename H = std::hash<K>,
          typename E = std::equal_to<K>,
          typename A = std::allocator<std::pair<K, V>>>
class LruCache {
    using TimePoint = LruNodeBase::TimePoint;

    /**
     * @brief 结点
     */
    struct Node : LruNodeBase {
        /**
         * @brief 构造 Node
         * @param deadline 结点超时时间
         * @param key 结点的键
         * @param value 结点的值
         */
        Node(TimePoint deadline, K key, V value);

        /**
         * @brief 析构 Node
         */
        ~Node() = default;

        /**
         * @brief 禁止拷贝 & 移动
         */
        Node(Node &&) = delete;
        Node(Node const &) = delete;
        void operator=(Node const &) = delete;
        void operator=(Node &&) = delete;

        const K key_;  ///< 键
        V value_;      ///< 值
    };

    /**
     * @brief 分配器
     */
    using Allocator =
        typename std::allocator_traits<A>::template rebind_alloc<Node>;

    /**
     * @brief 哈希函数
     */
    struct Hash : public H {
        using H::operator();
        std::size_t operator()(const Node &node) const;
    };

    /**
     * @brief 比较函数
     */
    struct Equal : public E {
        using E::operator();

        template <typename U>
        bool operator()(const U &lhs, const Node &rhs) const;

        template <typename U>
        bool operator()(const Node &lhs, const U &rhs) const;
    };

    /**
     * @brief 链表
     */
    using List =
        boost::intrusive::list<LruNodeBase,
                               boost::intrusive::constant_time_size<true>>;

    /**
     * @brief 哈希表
     */
    using UnorderedSet = boost::intrusive::unordered_set<
        Node, boost::intrusive::equal<Equal>, boost::intrusive::hash<Hash>,
        boost::intrusive::power_2_buckets<true>,
        boost::intrusive::store_hash<true>,
        boost::intrusive::constant_time_size<false>>;

    /**
     * @brief 哈希表桶萃取器
     */
    using BucketTraits = typename UnorderedSet::bucket_traits;

public:
    /**
     * @brief 超时时间
     */
    using Duration = std::chrono::steady_clock::duration;

    /**
     * @brief 迭代器
     */
    using Iterator = typename List::iterator;

    /**
     * @brief 常量迭代器
     */
    using ConstIterator = typename List::const_iterator;

private:
    /**
     * @brief 配置元组，包含哈希函数、比较函数、分配器、超时时间
     * @note 利用 std::tuple 压缩空类型，减少内存占用
     */
    using ConfigTuple = std::tuple<Hash, Equal, Allocator, Duration>;

public:
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
     * @brief 获取桶数量
     */
    size_t bucket_count() const { return hash_.bucket_count(); }

    /**
     * @brief 判断是否为空
     */
    bool empty() const { return list_.empty(); }

    /**
     * @brief 异构查找元素
     * @tparam U 异构键
     * @param key 键
     * @return Iterator 元素迭代器
     */
    template <typename U>
    Iterator find(U const &key) requires std::regular_invocable<Equal, U, K>;

    /**
     * @brief 异构查找元素
     * @tparam U 异构键
     * @param key 键
     * @return Iterator 元素迭代器
     */
    template <typename U>
    ConstIterator find(
        U const &key) const requires std::regular_invocable<Equal, U, K>;

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

    /**
     * @brief 清理过期元素
     * @tparam F 回调函数类型，需要支持 operator()(K, V)
     * @param now 当前时间
     * @param fn 回调函数
     * @return size_t 清理的元素数量
     */
    template <typename F>
    size_t clear_expired(TimePoint now,
                         F fn) requires std::regular_invocable<F, K, V>;

    /**
     * @brief 清理过期元素
     * @param now 当前时间
     * @return size_t 清理的元素数量
     */
    size_t clear_expired(TimePoint now) {
        return clear_expired(now, [](K, V) {});
    }

protected:
    /**
     * @brief 分配哈希表桶
     * @param n 桶数量
     * @return BucketTraits 桶数组
     */
    static BucketTraits alloc_buckets(std::size_t n);

    /**
     * @brief 获取哈希函数
     * @return H 哈希函数
     */
    Hash &get_hash() { return std::get<0>(config_tuple_); }

    /**
     * @brief 获取比较函数
     * @return E 比较函数
     */
    Equal &get_equal() { return std::get<1>(config_tuple_); }

    /**
     * @brief 获取分配器
     * @return Allocator 分配器
     */
    Allocator &get_allocator() { return std::get<2>(config_tuple_); }

    /**
     * @brief 获取超时时间
     * @return Duration 超时时间
     */
    Duration get_timeout() const { return std::get<3>(config_tuple_); }

private:
    ConfigTuple config_tuple_;              ///< 配置元组
    List list_;                             ///< 链表
    UnorderedSet hash_{alloc_buckets(16)};  ///< 哈希表
};

inline LruNodeBase::LruNodeBase(TimePoint deadline) : deadline_(deadline) {}

template <typename K, typename V, typename H, typename E, typename A>
LruCache<K, V, H, E, A>::Node::Node(TimePoint deadline, K key, V value)
    : LruNodeBase(deadline), key_(std::move(key)), value_(std::move(value)) {}

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
LruCache<K, V, H, E, A>::LruCache(Duration timeout)
    : config_tuple_{{}, {}, {}, timeout} {}

template <typename K, typename V, typename H, typename E, typename A>
LruCache<K, V, H, E, A>::~LruCache() {
    list_.clear();
    hash_.clear_and_dispose([](Node *node) { delete node; });
}

template <typename K, typename V, typename H, typename E, typename A>
template <typename U>
auto LruCache<K, V, H, E, A>::find(const U &key)
    -> Iterator requires std::regular_invocable<Equal, U, K> {
    auto const it = hash_.find(key, get_hash(), get_equal());
    if (it == hash_.end()) return list_.end();
    auto &node = *it;
    auto const list_it = list_.iterator_to(node);
    return list_it;
}

template <typename K, typename V, typename H, typename E, typename A>
template <typename U>
auto LruCache<K, V, H, E, A>::find(const U &key) const
    -> ConstIterator requires std::regular_invocable<Equal, U, K> {
    auto const it = hash_.find(key, get_hash(), get_equal());
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
        hash_.insert_check(key, get_hash(), get_equal(), commit_data);
    if (!inserted) {
        // 结点已存在，返回已存在的结点
        auto const list_it = list_.iterator_to(*it);
        ret = {list_it, false};
    } else {
        // 结点不存在，创建新结点
        auto const ptr = get_allocator().allocate(1);
        auto const node = new (ptr)
            Node(now + get_timeout(), std::move(key), std::move(value));
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
    auto &node = reinterpret_cast<Node &>(*it);
    hash_.erase(hash_.iterator_to(node));
    list_.erase(it);
    node.~Node();
    get_allocator().deallocate(&node, 1);
}

template <typename K, typename V, typename H, typename E, typename A>
void LruCache<K, V, H, E, A>::touch(TimePoint now, Iterator it) {
    auto &node = *it;
    node.deadline_ = now + get_timeout();
    list_.splice(list_.end(), list_, it);
}

template <typename K, typename V, typename H, typename E, typename A>
template <typename F>
size_t LruCache<K, V, H, E, A>::clear_expired(
    TimePoint now, F fn) requires std::regular_invocable<F, K, V> {
    size_t count = 0;
    while (!list_.empty()) {
        // 判断是否已过期
        auto &node = reinterpret_cast<Node &>(list_.front());
        if (node.deadline_ > now) break;

        // 删除之前调用回调函数
        fn(node.key_, node.value_);

        // 删除结点
        hash_.erase(hash_.iterator_to(node));
        list_.pop_front();
        node.~Node();
        get_allocator().deallocate(&node, 1);

        // 增加计数
        ++count;
    }

    return count;
}

template <typename K, typename V, typename H, typename E, typename A>
auto LruCache<K, V, H, E, A>::alloc_buckets(std::size_t n) -> BucketTraits {
    auto const buckets = new typename UnorderedSet::bucket_type[n];
    return {buckets, n};
}

}  // namespace flyzero