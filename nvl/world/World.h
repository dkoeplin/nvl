#pragma once

#include "nvl/actor/Actor.h"
#include "nvl/data/Map.h"
#include "nvl/data/Set.h"
#include "nvl/geo/Box.h"
#include "nvl/geo/RTree.h"
#include "nvl/math/Random.h"
#include "nvl/message/Created.h"
#include "nvl/message/Message.h"

namespace nvl {

template <U64 N>
class Entity;

template <U64 N>
class World {
public:
    static constexpr U64 kMaxEntries = 10;
    static constexpr U64 kGridExpMin = 2;
    static constexpr U64 kGridExpMax = 10;
    using EntityTree = RTree<N, Entity<N>, Actor, kMaxEntries, kGridExpMin, kGridExpMax>;

    // px / tick^2: (10 m/s^2) * (1000 px/m) * (1 ms /1000 s)^2 * (50 ms / tick)^2
    static constexpr I64 kVerticalDim = 1;
    static constexpr Pos<N> kGravity = Pos<N>::unit(kVerticalDim, 3);

    // px / tick    (50 m/s) * (1 s / 1000 ms) * (30 ms / tick) * (1000 px/m)
    static constexpr I64 kMaxVelocity = 1500;

    static constexpr I64 kMaxY = 10000;

    pure static bool is_up(const U64 dim, const Dir dir) { return dim == kVerticalDim && dir == Dir::Neg; }
    pure static bool is_down(const U64 dim, const Dir dir) { return dim == kVerticalDim && dir == Dir::Pos; }

    World() = default;

    pure Range<Actor> entities(const Pos<N> &pos) { return entities_[pos]; }
    pure Range<Actor> entities(const Box<N> &box) { return entities_[box]; }

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
        return spawn_by<T>(nullptr, std::forward<Args>(args)...);
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

    mutable Random random;

protected:
    using EntityHash = PointerHash<Ref<Entity<N>>>;
    using EntitySet = Set<Ref<Entity<N>>, EntityHash>;

    void tick_entity(Set<Actor> &died, Set<Actor> &idled, Ref<Entity<N>> entity) {
        static const List<Message> kNoMessages = {};

        const Actor actor = entity->self();
        const Box<N> prev_bbox = entity->bbox();
        const auto messages_iter = messages_.find(actor);
        const List<Message> &messages = messages_iter == messages_.end() ? kNoMessages : messages_iter->second;
        const Status status = entity->tick(messages);
        if (status == Status::kDied) {
            died.insert(actor);
        } else if (status == Status::kIdle) {
            idled.insert(actor);
        } else if (status == Status::kMove) {
            entities_.move(actor, prev_bbox);
        }
        if (messages_iter != messages_.end()) {
            messages_.erase(messages_iter);
        }
    }

    void tick() {
        // Wake any entities with pending messages
        for (auto &[actor, _] : messages_) {
            if (auto *entity = actor.template dyn_cast<Entity<N>>()) {
                awake_.emplace(entity);
            }
        }

        Set<Actor> died;
        Set<Actor> idled;
        for (Ref<Entity<N>> entity : awake_) {
            tick_entity(died, idled, entity);
        }
        awake_.remove(died.values());
        awake_.remove(idled.values());
    }

    EntityTree entities_;
    Set<Ref<Entity<N>>, EntityHash> awake_;
    Map<Actor, List<Message>> messages_;
};

} // namespace nvl
