#ifndef SMART_PTR_KIT_SHARED_PTR_HPP
#define SMART_PTR_KIT_SHARED_PTR_HPP

#include <utility>
#include <type_traits>
#include <atomic>
#include <memory>

namespace sptr {

namespace detail {
    class control_block {
    public:
        control_block() : m_use_count(1), m_weak_count(0) {}
        
        void add_reference() noexcept {
            ++m_use_count;
        }
        
        void add_weak_reference() noexcept {
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
    
    template <typename T>
    class inplace_control_block : public control_block {
    public:
        template <typename... Args>
        explicit inplace_control_block(Args&&... args) {
            new(&m_storage) T(std::forward<Args>(args)...);
        }
            
        void dispose() noexcept override {
            // Call destructor without deallocating memory
            get_object()->~T();
        }
        
        void destroy() noexcept override {
            delete this;
        }
        
        T* get() const noexcept {
            return get_object();
        }
        
    private:
        T* get_object() const noexcept {
            return const_cast<T*>(reinterpret_cast<const T*>(&m_storage));
        }
        
        // Aligned storage for T
        mutable typename std::aligned_storage<sizeof(T), alignof(T)>::type m_storage;
    };
}

template <typename T>
class weak_ptr;

// like a Rc<T>, Arc<T>
template <typename T>
class shared_ptr {
    friend class weak_ptr<T>;
    template <typename U, typename... Args>
    friend shared_ptr<U> make_shared(Args&&...);
    
    template <typename U, typename V>
    friend shared_ptr<U> dynamic_pointer_cast(const shared_ptr<V>&) noexcept;
    
    template <typename U, typename V>
    friend shared_ptr<U> static_pointer_cast(const shared_ptr<V>&) noexcept;
    
    template <typename U, typename V>
    friend shared_ptr<U> const_pointer_cast(const shared_ptr<V>&) noexcept;
    
    template <typename U, typename V>
    friend shared_ptr<U> reinterpret_pointer_cast(const shared_ptr<V>&) noexcept;
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
        if (m_ctrl) m_ctrl->add_reference();
    }
    
    template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
    shared_ptr(const shared_ptr<Y>& other) noexcept
        : m_ptr(other.m_ptr), m_ctrl(other.m_ctrl) {
        if (m_ctrl) m_ctrl->add_reference();
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
        if (m_ctrl) m_ctrl->add_reference();
    }
    
    T* m_ptr;
    detail::control_block* m_ctrl;
};

template <typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args) {
    try {
        // Create a control block with the object in-place
        auto cb = new detail::inplace_control_block<T>(std::forward<Args>(args)...);
        shared_ptr<T> result;
        // Use private constructor to set the pointer and control block
        result.m_ptr = cb->get();
        result.m_ctrl = cb;
        return result;
    } catch (...) {
        throw;
    }
}

// Dynamic cast
template <typename T, typename U>
shared_ptr<T> dynamic_pointer_cast(const shared_ptr<U>& other) noexcept {
    if (auto* p = dynamic_cast<T*>(other.get())) {
        shared_ptr<T> result;
        result.m_ptr = p;
        result.m_ctrl = other.m_ctrl;
        if (result.m_ctrl) result.m_ctrl->add_reference();
        return result;
    }
    return shared_ptr<T>();
}

// Static cast
template <typename T, typename U>
shared_ptr<T> static_pointer_cast(const shared_ptr<U>& other) noexcept {
    auto* p = static_cast<T*>(other.get());
    shared_ptr<T> result;
    result.m_ptr = p;
    result.m_ctrl = other.m_ctrl;
    if (result.m_ctrl) result.m_ctrl->add_reference();
    return result;
}

// Const cast
template <typename T, typename U>
shared_ptr<T> const_pointer_cast(const shared_ptr<U>& other) noexcept {
    auto* p = const_cast<T*>(other.get());
    shared_ptr<T> result;
    result.m_ptr = p;
    result.m_ctrl = other.m_ctrl;
    if (result.m_ctrl) result.m_ctrl->add_reference();
    return result;
}

// Reinterpret cast
template <typename T, typename U>
shared_ptr<T> reinterpret_pointer_cast(const shared_ptr<U>& other) noexcept {
    auto* p = reinterpret_cast<T*>(other.get());
    shared_ptr<T> result;
    result.m_ptr = p;
    result.m_ctrl = other.m_ctrl;
    if (result.m_ctrl) result.m_ctrl->add_reference();
    return result;
}

} // namespace sptr

#endif // SMART_PTR_KIT_SHARED_PTR_HPP