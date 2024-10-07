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
#include "nvl/ui/Screen.h"
#include "nvl/ui/Window.h"

namespace nvl {

template <U64 N>
class Entity;

template <U64 N>
class World : public AbstractScreen {
public:
    class_tag(World<N>, AbstractScreen);

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

    explicit World(Window *window, Params params = Params())
        : AbstractScreen(window),
          kGravityAccel(params.gravity_accel * kPixelsPerMeter * kMillisPerTick * kMillisPerTick / 1e6),
          kMaxVelocity(params.terminal_velocity * kMillisPerTick * kPixelsPerMeter / 1e3),
          kGravity(Pos<N>::unit(kVerticalDim, kGravityAccel)), kMaxY(params.maximum_y * kPixelsPerMeter) {

        on_mouse_move[{}] = on_mouse_move[{Mouse::Any}] = [this] {
            propagate_event();               // Don't prevent children from seeing the mouse movement event
            view_ += window_->mouse_delta(); // Change the view delta
        };
    }

    /// Converts the given coordinates from window coordinates to world coordinates.
    pure Pos<N> window_to_world(const Pos<2> &pt) const {
        static_assert(N == 2, "window_to_world implementation only works for 2D world right now");
        return pt + view_;
    }

    pure Box<N> window_to_world(const Box<2> &box) const {
        static_assert(N == 2, "window_to_world implementation only works for 2D world right now");
        return Box<N>(window_to_world(box.min), window_to_world(box.max));
    }

    pure Range<Actor> entities(const Pos<N> &pos) { return entities_[pos]; }
    pure Range<Actor> entities(const Box<N> &box) { return entities_[box]; }

    pure U64 num_awake() const { return awake_.size(); }
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

    /// Inserts a copy of this entity into the world.
    /// Returns a reference to the resulting copy.
    Actor reify(std::unique_ptr<Entity<N>> entity) {
        Actor result = entities_.take(std::move(entity));
        Entity<N> *copy = result.template dyn_cast<Entity<N>>();
        awake_.emplace(copy);
        copy->bind(this);
        return result;
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

    void tick() override;
    void draw() override;

    mutable Random random;

protected:
    using EntityHash = PointerHash<Ref<Entity<N>>>;

    void tick_entity(Set<Actor> &idled, Ref<Entity<N>> entity);

    EntityTree entities_;
    Set<Actor> awake_;
    Set<Actor> died_;
    Map<Actor, List<Message>> messages_;

    /// Marks the offset view. Starts at (0,0), so 2D coordinates == world coordinates
    Pos<N> view_ = Pos<N>::zero;
};

template <U64 N>
void World<N>::tick() {
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
}

template <U64 N>
void World<N>::draw() {
    const Box<2> range = window_to_world(window_->bbox());
    {
        auto offset = Window::Offset::Absolute(window_, view_);
        for (const Actor &actor : entities(range)) {
            actor->draw(window_, {});
        }
    }

    constexpr Color crosshair_color = Color::kBlack;
    const auto c = window_->center();
    const Box<2> hline({c[0] - 10, c[1] - 1}, {c[0] + 10, c[1] + 1});
    const Box<2> vline({c[0] - 1, c[1] - 10}, {c[0] + 1, c[1] + 10});
    window_->line_rectangle(crosshair_color, hline);
    window_->line_rectangle(crosshair_color, vline);

    window_->text(Color::kBlack, {10, 10}, 20, std::to_string(window_->fps()));
    window_->text(Color::kBlack, {10, 40}, 20, window_to_world(range).to_string());
    window_->text(Color::kBlack, {10, 70}, 20, "Alive: " + std::to_string(num_alive()));
    window_->text(Color::kBlack, {10, 100}, 20, "Awake: " + std::to_string(num_awake()));
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
