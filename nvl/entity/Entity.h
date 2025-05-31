#pragma once

#include "nvl/actor/Actor.h"
#include "nvl/actor/Part.h"
#include "nvl/actor/Status.h"
#include "nvl/geo/BRTree.h"
#include "nvl/geo/Tuple.h"
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
    using Part = nvl::Part<N>;
    using Edge = nvl::Edge<N, I64>;
    using Tree = BRTree<N, Part, Rel<Part>, kMaxEntries, kGridExpMin, kGridExpMax>;
    using Intersect = typename Tree::Intersect;

    explicit Entity(Pos<N> loc, Range<Rel<Part>> parts = {}) : parts_(loc, parts) {}
    explicit Entity(Pos<N> loc, Range<Part> parts) : parts_(loc, parts) {}

    pure Pos<N> loc() const { return parts_.loc; }
    pure Box<N> bbox() const { return parts_.bbox(); }
    pure const Tree &tree() const { return parts_; }
    pure const Pos<N> &velocity() const { return velocity_; }
    pure const Pos<N> &accel() const { return accel_; }

    pure Range<Rel<Edge>> edges() const { return parts_.edges(); }
    pure Range<Rel<Part>> parts() const { return parts_.items(); }

    pure Set<Rel<Part>> parts(const Box<N> &box) const { return parts_[box]; }
    pure Set<Rel<Part>> parts(const Pos<N> &pos) const { return parts_[pos]; }

    /// Returns the first part stored in the given volume, if one exists.
    pure expand Maybe<Rel<Part>> first(const Box<N> &box) const { return parts_.first(box); }
    pure expand Maybe<Rel<Part>> first(const Pos<N> &pos) const { return parts_.first(pos); }

    /// Returns the first part along the given line, if one exists, along with the intersection location and face.
    pure expand Maybe<Intersect> first(const Line<N> &line) const {
        return parts_.first_where(line, [](const Intersect &x) { return x.dist; });
    }

    /// Returns true if there are any items stored in the given volume.
    pure expand bool exists(const Box<N> &box) const { return parts_.exists(box); }
    pure expand bool exists(const Pos<N> &pos) const { return parts_.exists(pos); }

    pure virtual bool falls() const {
        return parts().all([](const Rel<Part> &part) { return part->material->falls; });
    }

    Status tick(const List<Message> &messages) override;

    void bind(World<N> *world) { world_ = world; }

    pure bool has_below() const;

    pure Set<Actor> above() const;

    /// Sends an action to the destination actor with this entity as the sender.
    template <typename Msg, typename... Args>
    void send(const Actor dst, Args &&...args) {
        world_->template send<Msg>(self(), dst, std::forward<Args>(args)...);
    }

    /// Sends an action to the destination actor(s) with this entity as the sender.
    template <typename Msg, typename... Args>
    void send(const Range<Actor> &dst, Args &&...args) {
        world_->template send<Msg>(self(), dst, std::forward<Args>(args)...);
    }

    /// Spawns a new actor in the world with this entity as the creator.
    template <typename Type, typename... Args>
    void spawn(Args &&...args) {
        world_->template spawn_by<Type>(self(), std::forward<Args>(args)...);
    }

protected:
    friend struct Relative;

    virtual Status broken(const List<Set<Rel<Part>>> &components) = 0;

    pure Pos<N> next_velocity() const;

    virtual Status receive(const Message &message);
    Status receive(const List<Message> &messages);

    Status hit(const List<Hit<N>> &hits);

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
    for (const Rel<Edge> &edge : parts_.edges()) {
        if (World<N>::is_up(edge->dim, edge->dir)) {
            const Box<N> box = edge->box + loc();
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
    for (const Rel<Edge> &edge : parts_.edges()) {
        if (World<N>::is_down(edge->dim, edge->dir)) {
            const Box<N> box = edge->box + loc();
            for (const Actor &actor : world_->entities(box)) {
                if (const Entity<N> *entity = actor.dyn_cast<Entity<N>>(); entity && entity != this) {
                    return_if(entity->exists(box), true);
                }
            }
        }
    }
    return false;
}

template <U64 N>
Pos<N> Entity<N>::next_velocity() const {
    const Pos<N> accel = accel_ + (falls() && !has_below() ? world_->kGravity : Pos<N>::zero);
    Pos<N> velocity;
    for (U64 i = 0; i < N; ++i) {
        const I64 v = velocity_[i];
        const I64 a = accel[i];
        I64 v_next = std::clamp(v + a, -world_->kMaxVelocity, world_->kMaxVelocity);
        if (v != 0 || a != 0) {
            for (const Rel<Part> &part : parts()) {
                const Box<N> box = part->box + loc();
                const I64 x = (v >= 0) ? box.end[i] : box.min[i];
                const Box<N> trj = box.with(i, x, x + v_next);
                for (Actor actor : world_->entities(trj)) {
                    if (auto *entity = actor.dyn_cast<Entity<N>>(); entity && entity != this) {
                        const I64 entity_x = entity->loc()[i];
                        for (const Rel<Part> &other : entity->parts(trj)) {
                            v_next = (v >= 0) ? std::clamp<I64>(other->box.min[i] + entity_x - x, 0, v_next)
                                              : std::clamp<I64>(x - entity_x - other->box.end[i], v_next, 0);
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
    if (message.isa<Destroy>()) {
        return Status::kDied;
    }
    return Status::kNone;
}

template <U64 N>
Status Entity<N>::receive(const List<Message> &messages) {
    List<Hit<N>> hits;
    Status status = Status::kNone;
    for (const auto &message : messages) {
        if (const auto *h = message.dyn_cast<Hit<N>>()) {
            hits.push_back(*h);
        } else {
            status = std::max(status, receive(message));
        }
        if (status == Status::kDied) {
            const Set<Actor> neighbors = above();
            send<Notify>(neighbors.values(), Notify::kDied);
            return status; // early exit on death
        }
    }
    if (!hits.empty()) {
        return hit(hits);
    }
    return status;
}

template <U64 N>
Status Entity<N>::hit(const List<Hit<N>> &hits) {
    Set<Actor> neighbors;
    bool was_hit = false;
    for (const Hit<N> &hit : hits) {
        const Box<N> local_box = hit.box - parts_.loc;
        const Set<Rel<Part>> hit_parts = parts(local_box);
        was_hit = was_hit || !hit_parts.empty();
        for (const Rel<Part> &part : hit_parts) {
            const Box<N> area = part->bbox().widened(1);
            neighbors.insert(world_->entities(area).values());
            if (part->health > hit.strength) {
                parts_.emplace(part->bbox().intersect(local_box).value(), part->material, part->health - hit.strength);
            }
            for (auto diff : part->diff(local_box)) {
                parts_.insert(diff);
            }
            parts_.remove(part);
        }
    }

    if (was_hit) {
        const List<Set<Rel<Part>>> components = parts_.components();
        const bool was_broken = components.size() != 1;
        const auto cause = was_broken ? Notify::kBroken : Notify::kChanged;
        send<Notify>(neighbors.values(), cause);
        return was_broken ? broken(components) : Status::kNone;
    }
    return Status::kNone;
}

template <U64 N>
Status Entity<N>::tick(const List<Message> &messages) {
    // Early exit if we aren't attached to a world
    return_if(world_ == nullptr, Status::kNone);

    Status status = receive(messages);
    return_if(status == Status::kDied, status);

    const Pos<N> init_velocity = velocity_;
    velocity_ = next_velocity();

    if (velocity_ != Pos<N>::zero) {
        if (init_velocity == Pos<N>::zero) {
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
