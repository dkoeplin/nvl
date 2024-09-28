#pragma once

#include <array>
#include <string_view>

#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"

namespace nvl {

template <typename T>
concept HasClassTag = requires(T inst) { T::_classtag; } && requires(T inst) { inst._get_classtag(); };

/**
 * @struct ClassTag
 * @brief Tracks class inheritance to allow reflection and dynamic casting.
 *
 * ClassTags are annotated on subclasses to allow for dynamic casting from a parent pointer to a subclass pointer.
 * The tag tracks the inheritance tree directly via an array of pointers to other ClassTags.
 * ClassTags are always static fields of their respective class.
 *
 * Typical Usage:
 * struct ParentClass {
 *   class_tag(ParentClass);
 *   virtual ~ParentClass() = default;
 * };
 *
 * struct ChildClass : ParentClass {
 *   class_tag(ChildClass, ParentClass);
 * }
 *
 * ParentClass *instance;
 * ChildClass *child = nvl::dyn_cast<ChildClass>(instance);
 */
struct ClassTag {
    static constexpr U64 kMaxParents = 16;

    template <typename T>
        requires HasClassTag<T>
    static constexpr const ClassTag &get() {
        return T::_classtag;
    }
    template <typename T>
        requires HasClassTag<T>
    static constexpr const ClassTag &get(const T *inst) {
        return inst->_get_classtag();
    }
    template <typename T>
        requires HasClassTag<T>
    static constexpr const ClassTag &get(const T &inst) {
        return inst._get_classtag();
    }

    explicit consteval ClassTag(const std::string_view name) : name(name) {}

    pure constexpr bool operator==(const ClassTag &rhs) const { return this == &rhs; }
    pure constexpr bool operator!=(const ClassTag &rhs) const { return this != &rhs; }
    pure bool operator<=(const ClassTag &rhs) const;
    pure bool operator>=(const ClassTag &rhs) const;
    pure bool operator<(const ClassTag &rhs) const;
    pure bool operator>(const ClassTag &rhs) const;

    template <U64 i>
    pure constexpr ClassTag with_parents() const {
        return *this;
    }

    template <U64 i, typename Arg, typename... Args>
        requires(i < kMaxParents)
    pure constexpr ClassTag with_parents() const {
        ClassTag tag = *this;
        tag.parents[i] = &Arg::_classtag;
        return tag.with_parents<i + 1, Args...>();
    }

    std::string_view name;
    std::array<const ClassTag *, kMaxParents> parents = {nullptr};
};

template <typename T>
pure const ClassTag &reflect() {
    return ClassTag::get<T>();
}

/// Registers a static ClassTag for this class and creates a virtual ClassTag getter for instances of this class.
#define class_tag(Name, ...)                                                                                           \
    [[maybe_unused]] static constexpr auto _classtag =                                                                 \
        ::nvl::ClassTag(#Name).with_parents<0 __VA_OPT__(, ) __VA_ARGS__>();                                           \
    pure virtual const ::nvl::ClassTag &_get_classtag() const __VA_OPT__(override) { return Name::_classtag; }

inline std::ostream &operator<<(std::ostream &os, const ClassTag &tag) { return os << tag.name; }

} // namespace nvl
