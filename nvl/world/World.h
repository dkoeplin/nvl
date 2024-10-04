#pragma once

#include "nvl/actor/Actor.h"
#include "nvl/data/Map.h"
#include "nvl/data/Set.h"
#include "nvl/geo/Box.h"
#include "nvl/geo/RTree.h"
#include "nvl/math/Random.h"
#include "nvl/message/Created.h"
#include "nvl/message/Destroy.h"
#include "nvl/message/Message.h"
#include "nvl/tool/Tool.h"

namespace nvl {

template <U64 N>
class Entity;

template <U64 N>
class World : public AbstractActor {
public:
    class_tag(World<N>, AbstractActor);

    struct Params {
        Params() {}
        U64 terminal_velocity = 53; // meters/sec (default is about 120mph)
        U64 gravity_accel = 10;     // meters/sec^2
        I64 maximum_y = 1e3;        // meters -- down is positive
    };

    static constexpr U64 kMaxEntries = 10;
    static constexpr U64 kGridExpMin = 2;
    static constexpr U64 kGridExpMax = 10;
    using EntityTree = RTree<N, Entity<N>, Actor, kMaxEntries, kGridExpMin, kGridExpMax>;

    static constexpr U64 kPixelsPerMeter = 1000; // px / m
    static constexpr U64 kMillisPerTick = 30;    // ms / tick
    static constexpr U64 kVerticalDim = 1;

    const U64 kGravityAccel; // px / tick^2
    const I64 kMaxVelocity;  // px / tick
    const Pos<N> kGravity;
    const I64 kMaxY; // px

    pure static bool is_up(const U64 dim, const Dir dir) { return dim == kVerticalDim && dir == Dir::Neg; }
    pure static bool is_down(const U64 dim, const Dir dir) { return dim == kVerticalDim && dir == Dir::Pos; }

    explicit World(Params params = Params())
        : kGravityAccel(params.gravity_accel * kPixelsPerMeter * kMillisPerTick * kMillisPerTick / 1e6),
          kMaxVelocity(params.terminal_velocity * kMillisPerTick * kPixelsPerMeter / 1e3),
          kGravity(Pos<N>::unit(kVerticalDim, kGravityAccel)), kMaxY(params.maximum_y * kPixelsPerMeter) {}

    pure Range<Actor> entities(const Pos<N> &pos) { return entities_[pos]; }
    pure Range<Actor> entities(const Box<N> &box) { return entities_[box]; }

    pure U64 num_active() const { return awake_.size(); }
    pure U64 num_alive() const { return entities_.size(); }

    template <typename Msg, typename... Args>
    void send(const Actor src, const Actor &dst, Args &&...args) {
        const auto message = Message::get<Msg>(src, std::forward<Args>(args)...);
        if (entities_.has(dst)) {
            messages_[dst].push_back(std::move(message));
        }
    }

    template <typename Msg, typename... Args>
    void send(const Actor src, const Range<Actor> &dst, Args &&...args) {
        const auto message = Message::get<Msg>(src, std::forward<Args>(args)...);
        for (const Actor &actor : dst) {
            if (entities_.has(actor)) {
                messages_[actor].push_back(message);
            }
        }
    }

    template <typename T, typename... Args>
    Actor spawn(Args &&...args) {
        Actor actor = entities_.template emplace<T>(std::forward<Args>(args)...);
        if (Entity<N> *entity = actor.template dyn_cast<Entity<N>>()) {
            awake_.emplace(entity);
            entity->bind(this);
        }
        return actor;
    }

    template <typename T, typename... Args>
    Actor spawn_by(const Actor src, Args &&...args) {
        Actor actor = entities_.template emplace<T>(std::forward<Args>(args)...);
        if (Entity<N> *entity = actor.template dyn_cast<Entity<N>>()) {
            awake_.emplace(entity);
            entity->bind(this);
        }
        if (src != nullptr) {
            send<Created>(src, actor);
        }
        return actor;
    }

    Status tick() { return tick({}); }
    Status tick(const List<Message> &messages) override;
    void draw(Window &, const U64) const override {}

    mutable Random random;

protected:
    using EntityHash = PointerHash<Ref<Entity<N>>>;

    void tick_entity(Set<Actor> &idled, Ref<Entity<N>> entity);

    EntityTree entities_;
    Set<Actor> awake_;
    Set<Actor> died_;
    Map<Actor, List<Message>> messages_;
};

template <U64 N>
Status World<N>::tick(const List<Message> &) {
    // Wake any entities with pending messages
    for (auto &[actor, _] : messages_) {
        awake_.emplace(actor);
    }

    Set<Actor> idled;
    for (Actor actor : awake_) {
        if (auto *entity = actor.dyn_cast<Entity<N>>()) {
            tick_entity(idled, Ref(entity));
        }
    }
    awake_.remove(died_.values());
    awake_.remove(idled.values());
    entities_.remove(died_.values());
    died_.clear();
    return Status::kNone;
}

template <U64 N>
void World<N>::tick_entity(Set<Actor> &idled, Ref<Entity<N>> entity) {
    static const List<Message> kNoMessages = {};

    const Actor actor = entity->self();
    const Box<N> prev_bbox = entity->bbox();
    const auto messages_iter = messages_.find(actor);
    const List<Message> &messages = messages_iter == messages_.end() ? kNoMessages : messages_iter->second;
    const Status status = entity->tick(messages);
    if (status == Status::kDied) {
        died_.insert(actor);
    } else if (status == Status::kIdle) {
        idled.insert(actor);
    } else if (status == Status::kMove) {
        entities_.move(actor, prev_bbox);
    }
    if (messages_iter != messages_.end()) {
        messages_.erase(messages_iter);
    }

    // Check if the entity is now above the maximum Y limits (down is positive)
    if (status != Status::kDied && entity->bbox().min[kVerticalDim] > kMaxY) {
        send<Destroy>(nullptr, actor, Destroy::kOutOfBounds);
    }
}

} // namespace nvl
