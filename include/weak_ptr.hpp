#ifndef SMART_PTR_KIT_WEAK_PTR_HPP
#define SMART_PTR_KIT_WEAK_PTR_HPP

#include "shared_ptr.hpp"

namespace sptr {

template <typename T>
class weak_ptr {
public:
    constexpr weak_ptr() noexcept : m_ptr(nullptr), m_ctrl(nullptr) {}
    
    weak_ptr(const weak_ptr& other) noexcept
        : m_ptr(other.m_ptr), m_ctrl(other.m_ctrl) {
        if (m_ctrl) m_ctrl->add_weak_ref();
    }
    
    template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
    weak_ptr(const weak_ptr<Y>& other) noexcept
        : m_ptr(other.m_ptr), m_ctrl(other.m_ctrl) {
        if (m_ctrl) m_ctrl->add_weak_ref();
    }
    
    template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
    weak_ptr(const shared_ptr<Y>& other) noexcept
        : m_ptr(other.m_ptr), m_ctrl(other.m_ctrl) {
        if (m_ctrl) m_ctrl->add_weak_ref();
    }
    
    weak_ptr(weak_ptr&& other) noexcept
        : m_ptr(other.m_ptr), m_ctrl(other.m_ctrl) {
        other.m_ptr = nullptr;
        other.m_ctrl = nullptr;
    }
    
    ~weak_ptr() {
        if (m_ctrl) m_ctrl->weak_release();
    }
    
    weak_ptr& operator=(const weak_ptr& other) noexcept {
        weak_ptr(other).swap(*this);
        return *this;
    }
    
    weak_ptr& operator=(weak_ptr&& other) noexcept {
        weak_ptr(std::move(other)).swap(*this);
        return *this;
    }
    
    template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
    weak_ptr& operator=(const shared_ptr<Y>& other) noexcept {
        weak_ptr(other).swap(*this);
        return *this;
    }
    
    void reset() noexcept {
        weak_ptr().swap(*this);
    }
    
    void swap(weak_ptr& other) noexcept {
        std::swap(m_ptr, other.m_ptr);
        std::swap(m_ctrl, other.m_ctrl);
    }
    
    long use_count() const noexcept {
        return m_ctrl ? m_ctrl->use_count() : 0;
    }
    
    bool expired() const noexcept {
        return use_count() == 0;
    }
    
    shared_ptr<T> lock() const noexcept {
        if (expired()) {
            return shared_ptr<T>();
        }
        return shared_ptr<T>(m_ptr, m_ctrl);
    }
    
private:
    T* m_ptr;
    detail::control_block* m_ctrl;
};

} // namespace sptr

#endif // SMART_PTR_KIT_WEAK_PTR_HPP