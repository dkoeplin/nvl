#pragma once

#include "nvl/actor/Actor.h"
#include "nvl/data/Map.h"
#include "nvl/geo/Box.h"
#include "nvl/geo/BRTree.h"
#include "nvl/message/Message.h"

namespace nvl {

template <U64 N>
class World {
public:
    static constexpr U64 kMaxEntries = 10;
    static constexpr U64 kGridExpMin = 2;
    static constexpr U64 kGridExpMax = 10;
    using EntityTree = BRTree<N, Entity<N>, Actor, kMaxEntries, kGridExpMin, kGridExpMax>;
    using window_iterator = typename EntityTree::window_iterator;

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

private:
    EntityTree entities_;
    Map<Actor, List<Message>> messages_;
};

} // namespace nvl
