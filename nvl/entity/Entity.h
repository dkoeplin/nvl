#pragma once

#include "nvl/actor/Actor.h"
#include "nvl/actor/Part.h"
#include "nvl/actor/Status.h"
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
    using Tree = BRTree<N, Part<N>, Ref<Part<N>>, kMaxEntries, kGridExpMin, kGridExpMax>;

    explicit Entity(Pos<N> loc, Range<Ref<Part<N>>> parts = {}) : parts_(loc, parts) {}

    pure Pos<N> loc() const { return parts_.loc; }
    pure Box<N> bbox() const { return parts_.bbox(); }
    pure const Tree &tree() const { return parts_; }
    pure const Pos<N> &velocity() const { return velocity_; }
    pure const Pos<N> &accel() const { return accel_; }

    pure Range<At<N, Edge<N>>> edges() const { return parts_.edges(); }
    pure Range<At<N, Part<N>>> parts() const { return parts_.items(); }
    pure Range<At<N, Part<N>>> parts(const Box<N> &box) const { return parts_[box]; }
    pure Range<At<N, Part<N>>> parts(const Pos<N> &pos) const { return parts_[pos]; }

    struct Relative {
        explicit Relative(Entity &entity) : entity(entity) {}
        pure Range<Ref<Part<N>>> parts() const { return entity.parts_.relative.items(); }
        pure Range<Ref<Part<N>>> parts(const Box<N> &box) const { return entity.parts_.relative[box]; }
        pure Range<Ref<Part<N>>> parts(const Pos<N> &pos) const { return entity.parts_.relative[pos]; }
        pure Range<Ref<Edge<N>>> edges() const { return entity.parts_.relative.edges(); }
        Entity &entity;
    } relative = Relative(*this);

    pure virtual bool falls() const {
        return relative.parts().all([](const Ref<Part<N>> part) { return part->material->falls; });
    }

    void draw(Draw &draw, const I64 highlight) override {
        Draw::Offset offset(draw, loc());
        for (Ref<Part<N>> part : parts_.relative.items()) {
            part->draw(draw, highlight);
        }
    }

    Status tick(const List<Message> &messages) override;

    void bind(World<N> *world) { world_ = world; }

protected:
    friend struct Relative;

    using Component = typename Tree::ItemTree::Component;
    virtual Status broken(const List<Component> &components) = 0;

    pure Set<Actor> above() const;

    pure bool has_below() const;

    pure Pos<N> next_velocity() const;

    Status receive(const Message &message);
    Status receive(const List<Message> &messages);

    Status hit(const Hit<N> &hit);

    template <typename Msg, typename... Args>
    void send(const Actor dst, Args &&...args) {
        world_->template send<Msg>(self(), dst, std::forward<Args>(args)...);
    }

    template <typename Msg, typename... Args>
    void send(const Range<Actor> &dst, Args &&...args) {
        world_->template send<Msg>(self(), dst, std::forward<Args>(args)...);
    }

    template <typename Type, typename... Args>
    void spawn(Args &&...args) {
        world_->template spawn_by<Type>(self(), std::forward<Args>(args)...);
    }

    /// State
    Tree parts_;
    Pos<N> velocity_ = Pos<N>::zero;
    Pos<N> accel_ = Pos<N>::zero;

    /// Binds
    World<N> *world_ = nullptr;
};

template <U64 N>
Set<Actor> Entity<N>::above() const {
    Set<Actor> above;
    for (const At<N, Edge<N>> &edge : parts_.edges()) {
        if (World<N>::is_up(edge->dim, edge->dir)) {
            const Box<N> box = edge.bbox();
            for (const Actor &actor : world_->entities(box)) {
                if (auto *entity = actor.dyn_cast<Entity<N>>(); entity && entity != this) {
                    auto overlapping = entity->parts(box);
                    if (!overlapping.empty()) {
                        above.insert(actor);
                    }
                }
            }
        }
    }
    return above;
}

template <U64 N>
bool Entity<N>::has_below() const {
    for (const At<N, Edge<N>> &edge : parts_.edges()) {
        if (World<N>::is_down(edge->dim, edge->dir)) {
            const Box<N> box = edge.bbox();
            for (const Actor &actor : world_->entities(box)) {
                const Entity<N> &entity = *actor.dyn_cast<Entity<N>>();
                return_if(entity.parts(box).empty(), true);
            }
        }
    }
    return false;
}

template <U64 N>
Pos<N> Entity<N>::next_velocity() const {
    Pos<N> velocity;
    for (U64 i = 0; i < N; ++i) {
        const I64 v = velocity_[i];
        const I64 a = accel_[i];
        I64 v_next = std::clamp(v + a, -World<N>::kMaxVelocity, World<N>::kMaxVelocity);
        if (v != 0 || a != 0) {
            for (const auto &part : parts()) {
                const Box<N> box = part.bbox();
                const U64 x = (v >= 0) ? box.min[i] : box.max[i];
                const Box<N> trj = box.with(i, x, x + v_next);
                for (Actor actor : world_->entities(trj)) {
                    if (auto *entity = actor.dyn_cast<Entity<N>>(); entity && entity != this) {
                        for (const auto &other : entity->parts(trj)) {
                            const I64 bound = (v >= 0) ? other.bbox().min[i] - 1 : other.bbox().max[i] + 1;
                            v_next = (v >= 0) ? std::min(v_next, bound) : std::max(v_next, bound);
                        }
                    }
                }
            }
        }
        velocity[i] = v_next;
    }
    return velocity;
}

template <U64 N>
Status Entity<N>::receive(const Message &message) {
    if (const auto *h = message.dyn_cast<Hit<N>>()) {
        return hit(*h);
    } else if (message.isa<Destroy>()) {
        return Status::kDied;
    }
    return Status::kNone;
}

template <U64 N>
Status Entity<N>::receive(const List<Message> &messages) {
    Status status = Status::kNone;
    Set<Actor> neighbors;
    for (const auto &message : messages) {
        status = std::max(status, receive(message));
        return_if(status == Status::kDied, status); // Early exit on death
    }
    return status;
}

template <U64 N>
Status Entity<N>::hit(const Hit<N> &hit) {
    const Box<N> local_box = hit.box - parts_.loc;
    const List<Ref<Part<N>>> hit_parts(relative.parts(local_box));
    return_if(hit_parts.empty(), Status::kNone);

    Set<Actor> neighbors;
    for (const Ref<Part<N>> &part : hit_parts) {
        const Box<N> area = part->bbox().widened(1);
        neighbors.insert(world_->entities(area));
        if (part->health > hit.strength) {
            parts_.emplace(part->bbox().intersect(local_box).value(), part->material, part->health - hit.strength);
        }
        for (auto diff : part->diff(local_box)) {
            parts_.insert(diff);
        }
        parts_.remove(part);
    }

    const List<Component> components = parts_.relative.components();
    const bool was_broken = components.size() != 1;
    const auto cause = was_broken ? Notify::kBroken : Notify::kChanged;
    send<Notify>(neighbors.values(), cause);
    return was_broken ? broken(components) : Status::kNone;
}

template <U64 N>
Status Entity<N>::tick(const List<Message> &messages) {
    Status status = receive(messages);
    return_if(status == Status::kDied, status);

    if (!has_below()) {
        accel_ += World<N>::kGravity;
    }

    const Pos<N> init_velocity = velocity_;
    velocity_ = next_velocity();

    if (velocity_ != Pos<N>::zero) {
        if (init_velocity != Pos<N>::zero) {
            // When starting to move, notify anything above
            const Set<Actor> neighbors = above();
            send<Notify>(neighbors.values(), Notify::kMoved);
        }
        // Move by velocity per tick
        parts_.loc += velocity_;
        status = Status::kMove;
    } else {
        status = Status::kIdle;
    }

    return status;
}

} // namespace nvl
