#ifndef SMART_PTR_KIT_UNIQUE_PTR_HPP
#define SMART_PTR_KIT_UNIQUE_PTR_HPP

#include <utility>
#include <type_traits>
#include <memory>

namespace sptr {

template <typename T, typename Deleter = std::default_delete<T>>
// like a box<T>
class unique_ptr {
public:
    using pointer = T*;
    using element_type = T;
    using deleter_type = Deleter;

    constexpr unique_ptr() noexcept : m_ptr(nullptr) {}
    constexpr unique_ptr(std::nullptr_t) noexcept : m_ptr(nullptr) {}
    explicit unique_ptr(pointer p) noexcept : m_ptr(p) {}
    
    ~unique_ptr() {
        reset();
    }

    unique_ptr(unique_ptr&& other) noexcept : m_ptr(other.release()) {}

    unique_ptr& operator=(unique_ptr&& other) noexcept {
        if (this != &other) {
            reset(other.release());
        }
        return *this;
    }

    unique_ptr(const unique_ptr&) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;

    T& operator*() const noexcept {
        return *m_ptr;
    }

    pointer operator->() const noexcept {
        return m_ptr;
    }

    pointer get() const noexcept {
        return m_ptr;
    }

    pointer release() noexcept {
        pointer tmp = m_ptr;
        m_ptr = nullptr;
        return tmp;
    }

    void reset(pointer p = nullptr) noexcept {
        pointer old_ptr = m_ptr;
        m_ptr = p;
        if (old_ptr) {
            m_deleter(old_ptr);
        }
    }

    void swap(unique_ptr& other) noexcept {
        std::swap(m_ptr, other.m_ptr);
        std::swap(m_deleter, other.m_deleter);
    }

    explicit operator bool() const noexcept {
        return m_ptr != nullptr;
    }

private:
    pointer m_ptr;
    deleter_type m_deleter{};
};

template <typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args) {
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

} // namespace sptr

#endif // SMART_PTR_KIT_UNIQUE_PTR_HPP