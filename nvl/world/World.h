#pragma once

#include <utility>

#include "nvl/actor/Actor.h"
#include "nvl/data/Map.h"
#include "nvl/data/Set.h"
#include "nvl/geo/RTree.h"
#include "nvl/geo/Volume.h"
#include "nvl/math/Random.h"
#include "nvl/message/Created.h"
#include "nvl/message/Destroy.h"
#include "nvl/message/Message.h"
#include "nvl/time/Clock.h"
#include "nvl/time/Duration.h"
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
        U64 terminal_velocity = 53;  // meters/sec (default is about 120mph)
        U64 gravity_accel = 10;      // meters/sec^2
        I64 maximum_y = 1e3;         // pixels -- down is positive
        U64 pixels_per_meter = 1000; // pixels / meter
        U64 ms_per_tick = 30;        // milliseconds / tick
    };

    static constexpr I64 kMaxEntries = 10;
    static constexpr I64 kGridExpMin = 2;
    static constexpr I64 kGridExpMax = 10;
    static constexpr U64 kVerticalDim = 1;
    using EntityTree = RTree<N, Entity<N>, Actor, kMaxEntries, kGridExpMin, kGridExpMax>;

    struct Intersect : nvl::Intersect<N> {
        Intersect(const nvl::Intersect<N> &init, Actor actor, Ref<Part<N>> part)
            : nvl::Intersect<N>(init), actor(std::move(actor)), part(part) {}
        Actor actor;
        Ref<Part<N>> part;
    };

    const I64 kMillisPerTick;  // ms / tick
    const I64 kNanosPerTick;   // ns / tick
    const I64 kPixelsPerMeter; // pixels / meter

    const I64 kGravityAccel; // pixels / tick^2
    const I64 kMaxVelocity;  // pixels / tick
    const Pos<N> kGravity;   // pixels / tick^2
    const I64 kMaxY;         // pixels

    pure static bool is_up(const U64 dim, const Dir dir) { return dim == kVerticalDim && dir == Dir::Neg; }
    pure static bool is_down(const U64 dim, const Dir dir) { return dim == kVerticalDim && dir == Dir::Pos; }

    explicit World(AbstractScreen *parent, Params params = {})
        : AbstractScreen(parent), kMillisPerTick(params.ms_per_tick), kNanosPerTick(params.ms_per_tick * 1e6),
          kPixelsPerMeter(params.pixels_per_meter),
          kGravityAccel(params.gravity_accel * kPixelsPerMeter * kMillisPerTick * kMillisPerTick / 1e6),
          kMaxVelocity(params.terminal_velocity * kMillisPerTick * kPixelsPerMeter / 1e3),
          kGravity(Pos<N>::unit(kVerticalDim, kGravityAccel)), // Gravity as a vector
          kMaxY(params.maximum_y) {

        on_mouse_move[{}] = on_mouse_move[{Mouse::Any}] = [this] {
            propagate_event(); // Don't prevent children from seeing the mouse movement event
            if constexpr (N == 2) {
                auto *view2d = view_.dyn_cast<View2D>();
                view2d->offset += window_->mouse_delta();
            } else if constexpr (N == 3) {
                auto *view3d = view_.dyn_cast<View3D>();
                view3d->rotate(window_->mouse_delta(), window_->shape());
            }
        };
        on_key_down[Key::Slash] = [this] { debug_ = !debug_; };
    }

    pure Range<Actor> entities() const { return entities_.items(); }
    pure Range<Actor> entities(const Box<N> &box) const { return entities_[box]; }
    pure Range<Actor> entities(const Pos<N> &pos) const { return entities_[pos]; }

    pure Maybe<Intersect> first_except(const Line<N> &line, const Actor &actor) const;
    pure Maybe<Intersect> first(const Line<N> &line) const { return first_except(line, nullptr); }

    pure ViewOffset view() const { return view_; }
    void set_hud(const bool enable) { hud_ = enable; }

    pure U64 num_awake() const { return awake_.size(); }
    pure U64 num_alive() const { return entities_.size(); }

    /// Converts the given coordinates from window coordinates to world coordinates.
    pure Pos<N> window_to_world(const Pos<2> &pos) const {
        if constexpr (N == 2) {
            auto *view2d = view_.dyn_cast<View2D>();
            return pos + view2d->offset;
        }
        UNREACHABLE;
    }
    pure Box<N> window_to_world(const Box<2> &box) const {
        return {window_to_world(box.min), window_to_world(box.max)};
    }

    template <typename Msg, typename... Args>
    void send(Actor src, const Actor &dst, Args &&...args) {
        const auto message = Message::get<Msg>(src.ptr(), std::forward<Args>(args)...);
        if (entities_.has(dst)) {
            messages_[dst].push_back(std::move(message));
        }
    }

    template <typename Msg, typename... Args>
    void send(Actor src, const Range<Actor> &dst, Args &&...args) {
        const auto message = Message::get<Msg>(src.ptr(), std::forward<Args>(args)...);
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
    T *spawn(Args &&...args) {
        Actor actor = entities_.template emplace<T>(std::forward<Args>(args)...);
        if (Entity<N> *entity = actor.template dyn_cast<Entity<N>>()) {
            awake_.emplace(entity);
            entity->bind(this);
        }
        return actor.dyn_cast<T>();
    }

    template <typename T, typename... Args>
    T *spawn_by(const Actor src, Args &&...args) {
        Actor actor = entities_.template emplace<T>(std::forward<Args>(args)...);
        if (Entity<N> *entity = actor.template dyn_cast<Entity<N>>()) {
            awake_.emplace(entity);
            entity->bind(this);
        }
        if (src != nullptr) {
            send<Created>(src, actor);
        }
        return actor.dyn_cast<T>();
    }

    void tick() override;
    void draw() override;

    virtual void remove(const Actor &actor);

    void set_view(const ViewOffset &view) { view_ = view; }

    pure const Map<Actor, List<Message>> &messages() const { return messages_; }

    pure U64 ticks() const { return ticks_; }

    mutable Random random;

protected:
    using EntityHash = PointerHash<Ref<Entity<N>>>;

    void tick_entity(Set<Actor> &idled, Ref<Entity<N>> entity);

    EntityTree entities_;
    Set<Actor> awake_;
    Set<Actor> died_;
    Map<Actor, List<Message>> messages_;

    ViewOffset view_ = ViewOffset::zero<N>(); // Location of the camera in world coordinates
    U64 msgs_last_ = 0, msgs_max_ = 0;        // Message queue sizes (previous tick and max)
    bool hud_ = true;                         // True if HUD should be drawn over world view
    bool debug_ = true;                       // True if debug should be drawn over world view
    U64 ticks_ = 0;
};

template <U64 N>
pure Maybe<typename World<N>::Intersect> World<N>::first_except(const Line<N> &line, const Actor &act) const {
    Maybe<Intersect> closest = None;
    for (Actor actor : entities({floor(line.a), ceil(line.b)})) {
        if (auto *entity = actor.dyn_cast<Entity<N>>(); entity && actor != act) {
            if (auto int0 = line.intersect(entity->bbox())) {
                if (auto int1 = entity->first(line)) {
                    if (!closest.has_value() || int1->dist < closest->dist) {
                        closest = Intersect(*int1, actor, int1->item);
                    }
                }
            }
        }
    }
    return closest;
}

template <U64 N>
void World<N>::tick() {
    msgs_last_ = 0;
    ticks_ += 1;

    // Wake any entities with pending messages
    for (auto &[actor, _] : messages_) {
        if (entities_.has(actor)) {
            awake_.emplace(actor);
        } else {
            died_.insert(actor);
        }
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
    messages_.remove(died_.values());
    died_.clear();
    msgs_max_ = std::max(msgs_max_, msgs_last_);
}

template <U64 N>
void World<N>::draw() {
    window_->push_view(view_);
    if constexpr (N == 2) {
        const auto range = window_to_world(window_->bbox());
        for (const Actor &actor : entities(range)) {
            actor->draw(window_, Color::kNormal);
        }
    } else if constexpr (N == 3) {
        // TODO: Restrict to only visible entities
        for (const Actor &actor : entities_) {
            actor->draw(window_, Color::kNormal);
        }
    }
    window_->pop_view();

    if (hud_) {
        constexpr Color crosshair_color = Color::kBlack;
        const auto c = window_->center();
        const Box<2> hline({c[0] - 10, c[1] - 1}, {c[0] + 10, c[1] + 1});
        const Box<2> vline({c[0] - 1, c[1] - 10}, {c[0] + 1, c[1] + 10});
        window_->line_box(crosshair_color, hline);
        window_->line_box(crosshair_color, vline);
    }
}

template <U64 N>
void World<N>::tick_entity(Set<Actor> &idled, Ref<Entity<N>> entity) {
    static const List<Message> kNoMessages = {};

    const Actor actor = entity->self();
    const Box<N> prev_bbox = entity->bbox();
    const auto messages_iter = messages_.find(actor);
    // Making a copy here to allow clearing the action queue early (prior to running tick)
    const List<Message> messages = messages_iter == messages_.end() ? kNoMessages : messages_iter->second;
    if (messages_iter != messages_.end()) {
        messages_.erase(messages_iter);
    }
    msgs_last_ += messages.size();
    const Status status = entity->tick(messages);
    if (status == Status::kDied) {
        remove(actor);
    } else if (status == Status::kIdle) {
        idled.insert(actor);
    } else if (status == Status::kMove) {
        entities_.move(actor, prev_bbox);
    }

    // Check if the entity is now above the maximum Y limits (down is positive)
    if (status != Status::kDied && entity->bbox().min[kVerticalDim] > kMaxY) {
        send<Destroy>(nullptr, actor, Destroy::kOutOfBounds);
    }
}

template <U64 N>
void World<N>::remove(const Actor &actor) {
    died_.insert(actor);
}

} // namespace nvl
