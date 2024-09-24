#pragma once

#include "nvl/actor/Actor.h"
#include "nvl/data/Map.h"
#include "nvl/data/Set.h"
#include "nvl/geo/Box.h"
#include "nvl/geo/BRTree.h"
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
    using window_iterator = typename EntityTree::window_iterator;

    static bool is_vertical(const U64 dim, const Dir dir) { return dim == 1 && dir == Dir::Neg; }

    static constexpr I64 kMaxY = 10000;

    pure Range<window_iterator> entities(const Pos<N> &pos) { return entities_[pos]; }
    pure Range<window_iterator> entities(const Box<N> &box) { return entities_[box]; }

    template <typename Msg, typename... Args>
    void send(const Actor src, const Actor &dst, Args &&...args) {
        const auto message = Message::get<Msg>(src, std::forward<Args>(args)...);
        messages_[dst].push_back(std::move(message));
    }

    template <typename Msg, typename Iterator, typename... Args>
        requires std::is_same_v<Actor, typename Iterator::value_type>
    void send(const Actor src, const Range<Iterator> &dst, Args &&...args) {
        const auto message = Message::get<Msg>(src, std::forward<Args>(args)...);
        for (const Actor &actor : dst) {
            messages_[actor].push_back(message);
        }
    }

protected:
    struct EntityHash {
        pure U64 operator()(const Ref<Entity<N>> &a) const { return sip_hash(a.ptr()); }
    };

    void tick() {
        Set<Actor> died;
        Set<Ref<Entity<N>>, EntityHash> idled;
        for (Ref<Entity<N>> entity : awake_) {
            const Actor actor = entity->self();
            const Box<N> prev_bbox = entity->bbox();
            const List<Message> &messages = messages_[actor];
            const Status status = entity->tick(messages);
            if (status == Status::kDied) {
                died.insert(actor);
            } else if (status == Status::kIdle) {
                idled.insert(entity);
            } else if (status == Status::kMove) {
                entities_.move(actor, prev_bbox);
            }
        }
        awake_.erase(died.begin(), died.end());
        awake_.erase(idled.begin(), idled.end());
    }

    EntityTree entities_;
    Set<Ref<Entity<N>>, EntityHash> awake_;
    Map<Actor, List<Message>> messages_;
};

} // namespace nvl
