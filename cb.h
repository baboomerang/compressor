#include <iterator>
#include <utility>
#include <limits>
#include <stdexcept>

  /**
   *  @brief A fixed size container connected end to end
   *
   *  @ingroup sequences
   *
   *  Meets the requirements of a container, a reversible container, and a sequence
   *
   *  Sets support random access iterators.
   *
   *  @tparam  T  Type of element. Required to be a complete type.
   *  @tparam  S  Size of the container (number of elements)
  */

template<typename T, std::size_t S>
class circular_buffer {
public:
    using value_type             = T;
    using pointer                = value_type*;
    using reference              = value_type&;
    using const_pointer          = const value_type*;
    using const_reference        = const value_type&;
    using size_type              = std::size_t;
    using difference_type        = std::ptrdiff_t;

    struct iterator {
        using iterator_category  = std::random_access_iterator_tag;
        using difference_type    = std::ptrdiff_t;
        using value_type         = T;
        using pointer            = value_type*;
        using reference          = value_type&;

        explicit iterator(pointer ptr) : m_pos(ptr - array_), m_ptr(ptr) {
            if (m_pos < 0 || m_pos >= array_size_) {
                throw std::out_of_range(std::string("circular_buffer::circular_buffer: iterator position " + m_pos)
                        + std::string(" is < 0  or >= container size: " + array_size_));
            }
        }
        //iterator(const iterator&);
        ~iterator() { delete[] m_ptr; }

        iterator& operator=(const iterator& rhs) { return rhs; }
        friend bool operator==(const iterator& lhs, const iterator& rhs) { return lhs.m_ptr == rhs.m_ptr; }
        friend bool operator!=(const iterator& lhs, const iterator& rhs) { return lhs.m_ptr != rhs.m_ptr; }
        //bool operator< (const iterator& a, const iterator& b) const { return a.m_ptr <  b.m_ptr; }
        //bool operator> (const iterator& a, const iterator& b) const { return a.m_ptr >  b.m_ptr; }
        //bool operator<=(const iterator& a, const iterator& b) const { return a.m_ptr <= b.m_ptr; }
        //bool operator>=(const iterator& a, const iterator& b) const { return a.m_ptr >= b.m_ptr; }

        // TODO: change the logic of these operators to allow iterators to 'wrap' over the container
        iterator& operator++() { ++m_ptr; return *this; }
        iterator& operator--() { --m_ptr; return *this; }
        iterator operator++(int) { iterator tmp = *this; ++m_ptr; return tmp; }
        iterator operator--(int) { iterator tmp = *this; --m_ptr; return tmp; }

        iterator& operator+=(size_type);
        iterator& operator-=(size_type);
        iterator operator+(size_type) const;
        iterator operator-(size_type) const;

        iterator operator+(size_type, const iterator&);
        difference_type operator-(iterator) const;

        reference operator[](size_type) const { return; }
        reference operator*() const;
        pointer operator->() const;

        private:
            difference_type m_pos;
            pointer m_ptr;
    };

    //using reverse_iterator       = std::reverse_iterator<iterator>;
    //using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // Iterators.
    iterator begin() {};
    //const_iterator begin() const;
    //const_iterator cbegin() const;

    iterator end();
    //const_iterator end() const;
    //const_iterator cend() const;

    //reverse_iterator rbegin();
    //const_reverse_iterator rbegin() const;
    //const_reverse_iterator crbegin() const;

    //reverse_iterator rend();
    //const_reverse_iterator rend() const;
    //const_reverse_iterator crend() const;

    // Class Methods.
    explicit circular_buffer(value_type, size_type capacity = 0) :
        array_(new value_type[capacity]),
        array_size_(capacity),
        start_(0),
        end_(0),
        contents_size_(0) {}

    ~circular_buffer() { delete[] array_; };

    void push_back(const_reference item) {
        inc_end();
        if (contents_size_ == array_size_) {
            inc_start();
        }
        array_[end_] = item;
    }

    void pop_front() {
        inc_start();
    }

    // Capacity.
    constexpr size_type size() const noexcept { return contents_size_; }
    constexpr size_type capacity() const noexcept { return array_size_; }
    constexpr size_type max_size() const noexcept { return std::numeric_limits<size_type>::max(); }
    constexpr bool empty() const noexcept { return contents_size_ == 0; }
    void clear() noexcept { start_ = end_ = contents_size_ = 0; }

    // Element access.
    reference operator[](size_type pos) noexcept { return array_[pos]; }
    constexpr const_reference operator[](size_type pos) const noexcept { return const_cast<reference>(array_[pos]); }
    reference at(size_type pos) {
        if (pos >= array_size_) {
            throw std::out_of_range(std::string("circular_buffer::at: " + pos) + std::string(" is >= " + array_size_));
        }
        return array_[pos];
    }
    constexpr const_reference at(size_type pos) const {
        return (pos < array_size_) ? const_cast<reference>(array_[pos])
            : throw std::out_of_range(std::string("circular_buffer::at: " + pos) + std::string(" is >= " + array_size_));
    }
    reference front() noexcept { return array_[start_]; }
    reference back() noexcept { return array_[end_]; }
    constexpr const_reference front() const noexcept { return const_cast<reference>(array_[start_]); }
    constexpr const_reference back() const noexcept { return const_cast<reference>(array_[end_]); }
    pointer data() noexcept { return array_; }
    const_pointer data() const noexcept { return const_cast<pointer>(array_); }

private:
    pointer array_;
    size_type array_size_;
    size_type start_;
    size_type end_;
    size_type contents_size_;

    void inc_start() {    // head
        if (empty) {
            return;
        }
        ++start_;
        --contents_size_;
        if (start_ == array_size_) {
            start_ = 0;
        }
    }

    void inc_end() {     // tail
        ++end_;
        ++contents_size_;
        if (end_ == array_size_) {
            end_ = 0;
        }
    }
};
