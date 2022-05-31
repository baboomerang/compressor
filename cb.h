#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdexcept>
#include <limits>

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
   *  @tparam  N  Number of elements in the container.
  */

template<typename T, std::size_t N>
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

        explicit iterator(pointer ptr, size_type pos, size_type& size) :
            m_ptr(ptr),
            m_pos(pos),
            m_size(size) {}
        ~iterator() = default;

        iterator& operator=(const iterator& rhs) {
            if (this == &rhs) {
                return *this;
            }
            m_ptr = rhs.m_ptr;
            m_pos = rhs.m_pos;
            m_size = rhs.m_size;
            return *this;
        }

        friend bool operator==(const iterator& lhs, const iterator& rhs)
                    { return (lhs.m_pos == rhs.m_pos) && (lhs.m_ptr == rhs.m_ptr) && (lhs.m_size == rhs.m_size); }

        friend bool operator!=(const iterator& lhs, const iterator& rhs)
                    { return (lhs.m_pos != rhs.m_pos) || (lhs.m_ptr != rhs.m_ptr) || (lhs.m_size != rhs.m_size); }

        // Dereferencing operators
        reference operator*() const { return m_ptr[m_pos]; }
        pointer operator->() { return &(operator*()); }

        // Prefix addition and subtraction operators
        iterator& operator++() {
            ++m_pos;
            if (m_pos == m_size) {
                m_pos = 0;
            }
            return *this;
        }
        iterator& operator--() {
            if (m_pos == 0) {
                m_pos = m_size - 1;
            } else {
                --m_pos;
            }
            return *this;
        }

        // Postfix addition and subtraction operators
        iterator operator++(int) {
            iterator tmp = *this;
            operator++();
            return tmp;
        }
        iterator operator--(int) {
            iterator tmp = *this;
            operator--();
            return tmp;
        }

        // Compound addition and subtraction operators
        iterator& operator+=(size_type length) {
            add_wrap(m_pos, length);
            return *this;
        }
        iterator& operator-=(size_type length) {
            size_type new_pos;
            subtract_wrap(m_pos, length);
            return *this;
        }

        // Binary addition and subtraction operators
        iterator operator+(size_type length) const {
            iterator tmp = *this;
            add_wrap(tmp.m_pos, length);
            return tmp;
        }
        iterator operator-(size_type length) const {
            iterator tmp = *this;
            subtract_wrap(tmp.m_pos, length);
            return tmp;
        }

    private:
        void subtract_wrap(size_type& pos, size_type& length) {
            size_type new_pos;
            if (length > pos) {
                size_type remainder = (length - pos) % m_size;
                new_pos = m_size - remainder;
            } else {
                new_pos = pos - length;
            }
            pos = new_pos;
        }
        void add_wrap(size_type& pos, size_type& length) {
            size_type new_pos = pos + length;
            if (new_pos >= m_size) {
                new_pos %= m_size;
            }
            pos = new_pos;
        }
        pointer m_ptr;
        size_type m_pos;
        size_type& m_size;
    };
    //===========================================================================

    iterator begin() noexcept { return iterator(array_, start_, array_size_); }
    iterator end() noexcept { return iterator(array_, end_, array_size_); }
    //const_iterator begin() { return const_iterator(array_, start_); }
    //const_iterator end() { return const_iterator(array_, end_); }

    // TODO:
    //       Add support for a constructor with two iterators
    //       Add support for const iterators
    //       Add mutexes for thread-saftey
    //       Change c style array to unique pointer?
    circular_buffer() :
        array_{0},
        array_size_{N},
        start_{0},
        end_{0},
        contents_size_{0}{}

    circular_buffer(std::initializer_list<value_type> il) :
        circular_buffer()
    {
        for (const auto& value : il) {
            push_back(value);
        }
    };

    ~circular_buffer(){}

    void push_back(const_reference item) {
        array_[end_] = item;
        increment_end();
        if (contents_size_ == array_size_) {
            increment_start();
        }
    }

    void pop_front() {
        increment_start();
    }

    // Capacity functions
    constexpr size_type size() const noexcept { return contents_size_; }
    constexpr size_type capacity() const noexcept { return array_size_; }
    constexpr size_type max_size() const noexcept { return std::numeric_limits<size_type>::max(); }
    constexpr bool empty() const noexcept { return contents_size_ == 0; }
    void clear() noexcept { start_ = end_ = contents_size_ = 0; }

    // Element access functions
    reference operator[](size_type pos) noexcept { return array_[pos]; }
    constexpr const_reference operator[](size_type pos) const noexcept { return array_[pos]; }
    reference at(size_type pos) {
        if (pos >= array_size_) {
            throw std::out_of_range(std::string("circular_buffer::at index: " + pos) +
                                    std::string(" is >= array size: " + array_size_));
        }
        return array_[pos];
    }
    constexpr const_reference at(size_type pos) const {
        return (pos >= array_size_) ? array_[pos]
            : throw std::out_of_range(std::string("circular_buffer::at index: " + pos) +
                                      std::string(" is >= array size: " + array_size_));
    }
    reference front() noexcept { return array_[start_]; }
    reference back() noexcept { return array_[end_]; }
    constexpr const_reference front() const noexcept { return array_[start_]; }
    constexpr const_reference back() const noexcept { return array_[end_]; }
    pointer data() noexcept { return array_; }
    const_pointer data() const noexcept { return array_; }

private:
    void increment_wrap(size_type& value) {
        ++value;
        if (value == array_size_) {
            value = 0;
        }
    }
    void increment_end() { increment_wrap(end_); ++contents_size_; }
    void increment_start() { if (empty()) return; increment_wrap(start_); --contents_size_; }
    value_type array_[N];
    size_type array_size_;
    size_type start_;
    size_type end_;
    size_type contents_size_;
};

#endif //CIRCULAR_BUFFER_H
