#pragma once

#include "nvl/macros/Pure.h"
#include "nvl/macros/ReturnIf.h"
#include "nvl/reflect/Castable.h"

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
    pure virtual std::unique_ptr<AbstractIterator<Value>> copy() const = 0;

    // HACK: These are returned as const pointers, but creation of a mutable Iterator will cast away the const.
    pure virtual const Value *ptr() = 0;

    pure virtual bool equals(const AbstractIterator &rhs) const = 0;
};

/**
 * @struct Iterator
 * @brief
 */
template <typename Value, View Type = View::kImmutable>
struct Iterator {
    using value_type = Value;
    using reference = std::conditional_t<Type == View::kImmutable, const Value &, Value &>;
    using pointer = std::conditional_t<Type == View::kImmutable, const Value *, Value &>;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;

    Iterator() = default;
    Iterator(const Iterator &rhs) : Iterator(rhs.ptr_ ? std::move(rhs.ptr_->copy()) : nullptr) {}

    implicit Iterator(const Iterator<Value, View::kMutable> &rhs)
        requires(Type == View::kImmutable)
        : Iterator(*(const Iterator<Value> *)(&rhs)) {}

    explicit Iterator(std::unique_ptr<AbstractIterator<Value>> impl) : ptr_(std::move(impl)) {}

    Iterator &operator=(const Iterator &rhs) {
        ptr_ = std::move(rhs.ptr_->copy());
        return *this;
    }

    /// Returns an immutable reference to the current value.
    pure const Value &operator*() const
        requires(Type == View::kImmutable)
    {
        return *ptr_->ptr();
    }
    /// Returns an immutable pointer to the current value.
    pure const Value *operator->() const
        requires(Type == View::kImmutable)
    {
        return ptr_->ptr();
    }

    /// Returns a mutable reference to the current value.
    pure Value &operator*() const
        requires(Type == View::kMutable)
    {
        return *const_cast<Value *>(ptr_->ptr());
    }
    /// Returns a mutable pointer to the current value.
    pure Value *operator->() const
        requires(Type == View::kMutable)
    {
        return const_cast<Value *>(ptr_->ptr());
    }

    /// Increments this iterator and returns the value of the iterator _after_ incrementing.
    Iterator &operator++() {
        ptr_->increment();
        return *this;
    }

    /// Increments this iterator and returns a copy of the iterator _before_ incrementing.
    Iterator operator++(int) {
        Iterator prev = ptr_->copy();
        ptr_->increment();
        return prev;
    }

    pure bool operator==(const Iterator &rhs) const {
        return_if(ptr_ == nullptr || rhs.ptr_ == nullptr, ptr_ == rhs.ptr_);
        return ptr_->equals(*rhs.ptr_);
    }
    pure bool operator!=(const Iterator &rhs) const {
        return_if(ptr_ == nullptr || rhs.ptr_ == nullptr, ptr_ != rhs.ptr_);
        return !ptr_->equals(*rhs.ptr_);
    }

protected:
    std::unique_ptr<AbstractIterator<Value>> ptr_ = nullptr;
};

template <typename Value>
using MutableIterator = Iterator<Value>;

template <typename IterType, View Type = View::kImmutable, typename... Args>
Iterator<typename IterType::value_type, Type> make_iterator(Args &&...args) {
    using Value = typename IterType::value_type;
    std::unique_ptr<IterType> inst = std::make_unique<IterType>(std::forward<Args>(args)...);
    Iterator<Value, Type> iter(std::move(inst));
    return iter;
}

} // namespace nvl
