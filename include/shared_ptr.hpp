#ifndef SMART_PTR_KIT_SHARED_PTR_HPP
#define SMART_PTR_KIT_SHARED_PTR_HPP

#include <utility>
#include <type_traits>
#include <atomic>

namespace sptr {

namespace detail {
    class control_block {
    public:
        control_block() : m_use_count(1), m_weak_count(0) {}
        
        void add_ref() noexcept {
            ++m_use_count;
        }
        
        void add_weak_ref() noexcept {
            ++m_weak_count;
        }
        
        bool release() noexcept {
            if (--m_use_count == 0) {
                dispose();
                if (m_weak_count == 0) {
                    destroy();
                    return true;
                }
            }
            return false;
        }
        
        void weak_release() noexcept {
            if (--m_weak_count == 0 && m_use_count == 0) {
                destroy();
            }
        }
        
        long use_count() const noexcept {
            return m_use_count;
        }
        
        virtual void dispose() noexcept = 0;
        virtual void destroy() noexcept = 0;
        virtual ~control_block() = default;
        
    private:
        std::atomic<long> m_use_count;
        std::atomic<long> m_weak_count;
    };
    
    template <typename T, typename Deleter = std::default_delete<T>>
    class ptr_control_block : public control_block {
    public:
        explicit ptr_control_block(T* ptr, Deleter d = Deleter())
            : m_ptr(ptr), m_deleter(std::move(d)) {}
            
        void dispose() noexcept override {
            if (m_ptr) {
                m_deleter(m_ptr);
                m_ptr = nullptr;
            }
        }
        
        void destroy() noexcept override {
            delete this;
        }
        
        T* get() const noexcept {
            return m_ptr;
        }
        
    private:
        T* m_ptr;
        Deleter m_deleter;
    };
}

template <typename T>
class weak_ptr;

template <typename T>
class shared_ptr {
    friend class weak_ptr<T>;
public:
    using element_type = T;
    
    constexpr shared_ptr() noexcept : m_ptr(nullptr), m_ctrl(nullptr) {}
    constexpr shared_ptr(std::nullptr_t) noexcept : m_ptr(nullptr), m_ctrl(nullptr) {}
    
    template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
    explicit shared_ptr(Y* ptr) {
        try {
            m_ctrl = new detail::ptr_control_block<Y>(ptr);
            m_ptr = ptr;
        } catch (...) {
            delete ptr;
            throw;
        }
    }
    
    shared_ptr(const shared_ptr& other) noexcept
        : m_ptr(other.m_ptr), m_ctrl(other.m_ctrl) {
        if (m_ctrl) m_ctrl->add_ref();
    }
    
    template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
    shared_ptr(const shared_ptr<Y>& other) noexcept
        : m_ptr(other.m_ptr), m_ctrl(other.m_ctrl) {
        if (m_ctrl) m_ctrl->add_ref();
    }
    
    shared_ptr(shared_ptr&& other) noexcept
        : m_ptr(other.m_ptr), m_ctrl(other.m_ctrl) {
        other.m_ptr = nullptr;
        other.m_ctrl = nullptr;
    }
    
    template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
    shared_ptr(shared_ptr<Y>&& other) noexcept
        : m_ptr(other.m_ptr), m_ctrl(other.m_ctrl) {
        other.m_ptr = nullptr;
        other.m_ctrl = nullptr;
    }
    
    ~shared_ptr() {
        if (m_ctrl) m_ctrl->release();
    }
    
    shared_ptr& operator=(const shared_ptr& other) noexcept {
        shared_ptr(other).swap(*this);
        return *this;
    }
    
    shared_ptr& operator=(shared_ptr&& other) noexcept {
        shared_ptr(std::move(other)).swap(*this);
        return *this;
    }
    
    void reset() noexcept {
        shared_ptr().swap(*this);
    }
    
    template <typename Y>
    void reset(Y* ptr) {
        shared_ptr(ptr).swap(*this);
    }
    
    void swap(shared_ptr& other) noexcept {
        std::swap(m_ptr, other.m_ptr);
        std::swap(m_ctrl, other.m_ctrl);
    }
    
    T* get() const noexcept {
        return m_ptr;
    }
    
    T& operator*() const noexcept {
        return *m_ptr;
    }
    
    T* operator->() const noexcept {
        return m_ptr;
    }
    
    long use_count() const noexcept {
        return m_ctrl ? m_ctrl->use_count() : 0;
    }
    
    explicit operator bool() const noexcept {
        return m_ptr != nullptr;
    }
    
private:
    template <typename Y>
    shared_ptr(Y* ptr, detail::control_block* ctrl) noexcept
        : m_ptr(ptr), m_ctrl(ctrl) {
        if (m_ctrl) m_ctrl->add_ref();
    }
    
    T* m_ptr;
    detail::control_block* m_ctrl;
};

template <typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args) {
    // This is a simplified version - a real implementation would use allocate_shared
    // with a custom allocator to allocate control block and object in one allocation
    return shared_ptr<T>(new T(std::forward<Args>(args)...));
}

} // namespace sptr

#endif // SMART_PTR_KIT_SHARED_PTR_HPP