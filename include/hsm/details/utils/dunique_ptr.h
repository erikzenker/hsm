
#pragma once

#include <memory>

namespace hsm::details::utils {

template <typename T, typename Delete = std::default_delete<T>> struct dunique_ptr;

template <typename T> struct dunique_ptr<T, std::default_delete<T>> {
    dunique_ptr()
        : ptr(nullptr)
    {
    }
    explicit dunique_ptr(T* ptr)
        : ptr(ptr)
    {
    }
    dunique_ptr(const dunique_ptr&) = delete;
    auto operator=(const dunique_ptr&) -> dunique_ptr& = delete;
    dunique_ptr(dunique_ptr&& other) noexcept
        : ptr(other.ptr)
    {
        other.ptr = nullptr;
    }
    auto operator=(dunique_ptr&& other) noexcept -> dunique_ptr&
    {
        swap(other);
        return *this;
    }
    ~dunique_ptr()
    {
        reset();
    }
    void reset(T* value = nullptr)
    {
        T* to_delete = ptr;
        ptr = value;
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete to_delete;
    }
    auto release() -> T*
    {
        T* result = ptr;
        ptr = nullptr;
        return result;
    }
    explicit operator bool() const
    {
        return bool(ptr);
    }
    [[nodiscard]] auto get() const -> T*
    {
        return ptr;
    }
    auto operator*() const -> T&
    {
        return *ptr;
    }
    auto operator->() const -> T*
    {
        return ptr;
    }
    void swap(dunique_ptr& other)
    {
        std::swap(ptr, other.ptr);
    }

    auto operator==(const dunique_ptr& other) const -> bool
    {
        return ptr == other.ptr;
    }
    auto operator!=(const dunique_ptr& other) const -> bool
    {
        return !(*this == other);
    }
    auto operator<(const dunique_ptr& other) const -> bool
    {
        return ptr < other.ptr;
    }
    auto operator<=(const dunique_ptr& other) const -> bool
    {
        return !(other < *this);
    }
    auto operator>(const dunique_ptr& other) const -> bool
    {
        return other < *this;
    }
    auto operator>=(const dunique_ptr& other) const -> bool
    {
        return !(*this < other);
    }

  private:
    T* ptr;
};

template <typename T, typename D> void swap(dunique_ptr<T, D>& lhs, dunique_ptr<T, D>& rhs)
{
    lhs.swap(rhs);
}
}