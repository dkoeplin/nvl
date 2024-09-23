#pragma once

#include "nvl/actor/Actor.h"
#include "nvl/actor/Part.h"
#include "nvl/actor/Status.h"
#include "nvl/actor/TickResult.h"
#include "nvl/geo/BRTree.h"
#include "nvl/geo/Pos.h"
#include "nvl/macros/Abstract.h"
#include "nvl/macros/Aliases.h"
#include "nvl/message/Destroy.h"
#include "nvl/message/Hit.h"
#include "nvl/message/Message.h"
#include "nvl/message/Notify.h"
#include "nvl/world/World.h"

namespace nvl {

template <U64 N>
abstract class Entity : public AbstractActor {
public:
    class_tag(Entity<N>, AbstractActor);
    static constexpr U64 kMaxEntries = 10;
    static constexpr U64 kGridExpMin = 2;
    static constexpr U64 kGridExpMax = 10;
    using Tree = BRTree<N, Part<N>, kMaxEntries, kGridExpMin, kGridExpMax>;

    struct Debug {
        explicit Debug(Entity &entity) : entity(entity) {}

        pure const Tree &tree() const { return entity.parts_; }

        Entity &entity;
    };

    pure virtual Pos<N> loc() const { return parts_.loc; }
    pure virtual Box<N> bbox() const { return parts_.bbox(); }

    pure Range<typename Tree::edge_iterator> edges() { return parts_.unordered_edges(); }

    pure Range<typename Tree::item_iterator> parts() { return parts_.unordered_items(); }
    pure Range<typename Tree::window_iterator> parts(const Box<N> &box) { return parts_[box]; }
    pure Range<typename Tree::window_iterator> parts(const Pos<N> &pos) { return parts_[pos]; }

    struct View {
        explicit View(Entity &entity) : entity(entity) {}
        pure Range<typename Tree::Viewed::item_iterator> parts() { return entity.parts_.view.unordered_items(); }
        pure Range<typename Tree::Viewed::window_iterator> parts(const Box<N> &box) { return entity.parts_.view[box]; }
        pure Range<typename Tree::Viewed::window_iterator> parts(const Pos<N> &pos) { return entity.parts_.view[pos]; }
        Entity &entity;
    } view = View(*this);

    pure virtual bool falls() {
        return view.parts().all([](const Part<N> &part) { return part.material().falls(); });
    }

    void draw(Draw &draw, const int64_t highlight) override {
        Draw::Offset offset(draw, loc());
        for (const auto &part : parts_.view.unordered_items()) {
            part.draw(draw, highlight);
        }
    }

    void tick(const List<Message> &messages) override;

protected:
    friend struct View;

    void receive(Set<Actor> &neighbors, const Message &message);
    void receive(const List<Message> &messages);

    void hit(Set<Actor> &neighbors, const Hit<N> &hit);

    void destroy();

    template <typename Msg, typename... Args>
    void send(const Actor dst, Args &&...args) {
        world_.template send<Msg>(self(), dst, std::forward<Args>(args)...);
    }

    template <typename Msg, typename Iterator, typename... Args>
        requires std::is_same_v<Actor, typename Iterator::value_type>
    void send(const Range<Iterator> &dst, Args &&...args) {
        world_.template send<Msg>(self(), dst, std::forward<Args>(args)...);
    }

    Pos<N> velocity_ = Pos<N>::fill(0);
    Pos<N> accel_ = Pos<N>::fill(0);
    Tree parts_;
    Ref<World<N>> world_;
};

template <U64 N>
void Entity<N>::receive(Set<Actor> &neighbors, const Message &message) {
    if (auto *h = message.dyn_cast<Hit<N>>()) {
        hit(neighbors, *h);
    } else if (message.isa<Destroy>()) {
        destroy();
    }
}

template <U64 N>
void Entity<N>::receive(const List<Message> &messages) {
    Set<Actor> neighbors;
    for (const auto &message : messages) {
        receive(neighbors, message);
        return_if(status_ & Status::Died); // Early exit on death
    }
}

template <U64 N>
void Entity<N>::hit(Set<Actor> &neighbors, const Hit<N> &hit) {
    const Box<N> local_box = hit.box - parts_.loc;
    const List<Ref<Part<N>>> hit_parts(view.parts(local_box));
    return_if(hit_parts.empty());

    status_ |= Status::Hit;
    for (const Ref<Part<N>> &part : hit_parts) {
        const Box<N> area = part->bbox().widened(1);
        neighbors.insert(world_->entities(area));
        if (part->health() > hit.strength) {
            parts_.emplace(part->bbox().intersect(local_box).value(), part->material(), part->health() - hit.strength);
        }
        parts_.insert(part->diff(local_box));
        parts_.remove(part);
    }

    const auto components = parts_.view.components();
    const bool broken = components.size() != 1;
    const auto cause = broken ? Notify::kBroken : Notify::kChanged;
    send<Notify>(neighbors.unordered(), cause);

    if (broken) {
        status_ |= Status::Died;
    } else {
        status_ |= Status::Woke;
    }
}

template <U64 N>
void Entity<N>::destroy() {
    status_ |= Status::Died;
}

template <U64 N>
void Entity<N>::tick(const List<Message> &messages) {
    receive(messages);
}

} // namespace nvl
