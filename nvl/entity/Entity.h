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

    void draw(Draw &draw, const int64_t highlight) const override {
        Draw::Offset offset(draw, loc());
        for (const auto &part : parts_.view.unordered_items()) {
            part.draw(draw, highlight);
        }
    }

    TickResult tick(const List<Message> &messages) override;

protected:
    friend struct View;

    void receive(TickResult &result, Set<Actor> &neighbors, const Message &message);
    void receive(TickResult &result, const List<Message> &messages);

    void hit(TickResult &result, Set<Actor> &neighbors, const Hit<N> &hit);

    void destroy(TickResult &result);

    Pos<N> velocity_ = Pos<N>::fill(0);
    Pos<N> accel_ = Pos<N>::fill(0);
    Tree parts_;
    World<N> *world_ = nullptr;
    bool alive_ = true;
};

template <U64 N>
void Entity<N>::receive(TickResult &result, Set<Actor> &neighbors, const Message &message) {
    if (auto *h = message.dyn_cast<Hit<N>>()) {
        hit(result, neighbors, *h);
    } else if (message.isa<Destroy>()) {
        destroy(result);
    }
}

template <U64 N>
void Entity<N>::receive(TickResult &result, const List<Message> &messages) {
    Set<Actor> neighbors;
    for (const auto &message : messages) {
        receive(result, neighbors, message);
        if (result.status & Status::Died) {
            return; // Early exit on death
        }
    }
}

template <U64 N>
void Entity<N>::hit(TickResult &result, Set<Actor> &neighbors, const Hit<N> &hit) {}

template <U64 N>
void Entity<N>::destroy(TickResult &result) {
    alive_ = false;
    result.status |= Status::Died;
}

template <U64 N>
TickResult Entity<N>::tick(const List<Message> &messages) {
    TickResult result;
    receive(result, messages);
    return result;
}

} // namespace nvl
