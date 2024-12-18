#pragma once

#include "nvl/macros/Assert.h"
#include "nvl/macros/Implicit.h"
#include "nvl/macros/Pure.h"
#include "nvl/macros/ReturnIf.h"
#include "nvl/macros/Unreachable.h"
#include "nvl/reflect/Casting.h"
#include "nvl/reflect/ClassTag.h"

namespace nvl {

enum class View {
    kImmutable, // Cannot mutate the elements of the underlying collection.
    kMutable,   // Can mutate the elements of the underlying collection.
};

/**
 * @struct AbstractIterator
 * @brief Base class for iterator which provide constant references.
 */
template <typename Value>
struct AbstractIterator {
    class_tag(AbstractIterator<Value>);
    using value_type = Value;
    using reference = const Value &;
    using pointer = const Value *;

    virtual ~AbstractIterator() = default;
    virtual void increment() = 0;
    pure virtual std::shared_ptr<AbstractIterator<Value>> copy() const = 0;

    // HACK: These are returned as const pointers, but creation of a mutable Iterator will cast away the const.
    pure virtual const Value *ptr() = 0;

    pure virtual U64 diff(const AbstractIterator &rhs) const = 0;
    pure virtual bool equals(const AbstractIterator &rhs) const = 0;
};

template <typename Concrete, typename Value>
struct AbstractIteratorCRTP : AbstractIterator<Value> {
    pure std::shared_ptr<AbstractIterator<Value>> copy() const final {
        return std::make_shared<Concrete>(*static_cast<const Concrete *>(this));
    }
    pure bool equals(const AbstractIterator<Value> &rhs) const final {
        auto *b = dyn_cast<Concrete>(&rhs);
        return b && *this == *b;
    }
    pure virtual U64 operator-(const Concrete &) const { UNREACHABLE; }
    pure virtual bool operator==(const Concrete &rhs) const = 0;

    pure U64 diff(const AbstractIterator<Value> &rhs) const final {
        auto *b = dyn_cast<Concrete>(&rhs);
        ASSERT(b, "Cannot compute diff between differing iterator types.");
        return *this - *b;
    }
};

/**
 * @struct Iterator
 * @brief
 */
template <typename Value, View Type = View::kImmutable>
struct Iterator {
    using value_type = Value;
    using reference = std::conditional_t<Type == View::kImmutable, const Value &, Value &>;
    using pointer = std::conditional_t<Type == View::kImmutable, const Value *, Value *>;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;

    Iterator() = default;
    Iterator(const Iterator &rhs) : Iterator(rhs.ptr_) {}

    /// Converts a mutable iterator to an immutable one.
    /// Requires C-style casting because the two aren't actually related by inheritance.
    implicit Iterator(const Iterator<Value, View::kMutable> &rhs)
        requires(Type == View::kImmutable)
        : Iterator(*(const Iterator<Value> *)(&rhs)) {}

    explicit Iterator(std::shared_ptr<AbstractIterator<Value>> impl) : ptr_(impl) {}

    Iterator &operator=(const Iterator &rhs) = default;

    Iterator copy() const { return ptr_ ? Iterator(ptr_->copy()) : Iterator(); }

    /// Returns an immutable reference to the current value.
    pure reference operator*() const { return *const_cast<pointer>(ptr_->ptr()); }

    /// Returns an immutable pointer to the current value.
    pure pointer operator->() const { return const_cast<pointer>(ptr_->ptr()); }

    /// Increments this iterator and returns the value of the iterator _after_ incrementing.
    Iterator &operator++() {
        ptr_->increment();
        return *this;
    }

    /// Increments this iterator and returns a copy of the iterator _before_ incrementing.
    Iterator operator++(int) {
        Iterator prev = copy();
        ptr_->increment();
        return prev;
    }

    pure U64 operator-(const Iterator &rhs) const { return ptr_->diff(*rhs.ptr_); }

    pure bool operator==(const Iterator &rhs) const {
        return_if(ptr_ == nullptr || rhs.ptr_ == nullptr, ptr_ == rhs.ptr_);
        return ptr_->equals(*rhs.ptr_);
    }
    pure bool operator!=(const Iterator &rhs) const {
        return_if(ptr_ == nullptr || rhs.ptr_ == nullptr, ptr_ != rhs.ptr_);
        return !ptr_->equals(*rhs.ptr_);
    }

    template <typename T>
    pure const T *dyn_cast() const {
        return nvl::dyn_cast<T>(ptr_.get());
    }

    template <typename T>
    pure T *dyn_cast() {
        return nvl::dyn_cast<T>(ptr_.get());
    }

    template <typename T>
    pure bool isa() const {
        return nvl::isa<T>(ptr_.get());
    }

protected:
    std::shared_ptr<AbstractIterator<Value>> ptr_ = nullptr;
};

template <typename Value>
using MIterator = Iterator<Value, View::kMutable>;

template <typename IterType, View Type = View::kImmutable, typename... Args>
Iterator<typename IterType::value_type, Type> make_iterator(Args &&...args) {
    using Value = typename IterType::value_type;
    std::shared_ptr<AbstractIterator<Value>> ptr = std::make_shared<IterType>(std::forward<Args>(args)...);
    return Iterator<Value, Type>(ptr);
}

template <typename IterType, typename... Args>
MIterator<typename IterType::value_type> make_miterator(Args &&...args) {
    using Value = typename IterType::value_type;
    std::shared_ptr<AbstractIterator<Value>> ptr = std::make_shared<IterType>(std::forward<Args>(args)...);
    return Iterator<Value, View::kMutable>(ptr);
}

} // namespace nvl
