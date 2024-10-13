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
        Params() {}
        U64 terminal_velocity = 53;  // meters/sec (default is about 120mph)
        U64 gravity_accel = 2;       // meters/sec^2
        I64 maximum_y = 1e3;         // meters -- down is positive
        U64 pixels_per_meter = 1000; // pixels / meter
        U64 ms_per_tick = 30;        // milliseconds / tick
    };

    static constexpr I64 kMaxEntries = 10;
    static constexpr I64 kGridExpMin = 2;
    static constexpr I64 kGridExpMax = 10;
    static constexpr U64 kVerticalDim = 1;
    using EntityTree = RTree<N, Entity<N>, Actor, kMaxEntries, kGridExpMin, kGridExpMax>;

    const I64 kMillisPerTick;  // ms / tick
    const I64 kNanosPerTick;   // ns / tick
    const I64 kPixelsPerMeter; // pixels / meter

    const I64 kGravityAccel; // pixels / tick^2
    const I64 kMaxVelocity;  // pixels / tick
    const Pos<N> kGravity;   // pixels / tick^2
    const I64 kMaxY;         // pixels

    pure static bool is_up(const U64 dim, const Dir dir) { return dim == kVerticalDim && dir == Dir::Neg; }
    pure static bool is_down(const U64 dim, const Dir dir) { return dim == kVerticalDim && dir == Dir::Pos; }

    explicit World(Window *window, Params params = Params())
        : AbstractScreen(window), kMillisPerTick(params.ms_per_tick), kNanosPerTick(params.ms_per_tick * 1e6),
          kPixelsPerMeter(params.pixels_per_meter),
          kGravityAccel(params.gravity_accel * kPixelsPerMeter * kMillisPerTick * kMillisPerTick / 1e6),
          kMaxVelocity(params.terminal_velocity * kMillisPerTick * kPixelsPerMeter / 1e3),
          kGravity(Pos<N>::unit(kVerticalDim, kGravityAccel)), // Gravity as a vector
          kMaxY(params.maximum_y * kPixelsPerMeter) {

        on_mouse_move[{}] = on_mouse_move[{Mouse::Any}] = [this] {
            propagate_event(); // Don't prevent children from seeing the mouse movement event
            view_ += window_->mouse_delta();
        };
        on_key_down[Key::D] = [this] { debug_ = !debug_; };
    }

    pure Range<Actor> entities() const { return entities_.items(); }
    pure Range<Actor> entities(const Pos<N> &pos) const { return entities_[pos]; }
    pure Range<Actor> entities(const Box<N> &box) const { return entities_[box]; }

    pure Pos<2> view() const { return view_; }
    void set_hud(const bool enable) { hud_ = enable; }

    pure U64 num_awake() const { return awake_.size(); }
    pure U64 num_alive() const { return entities_.size(); }

    /// Converts the given coordinates from window coordinates to world coordinates.
    pure Pos<2> window_to_world(const Pos<2> &pos) const { return pos + view_; }
    pure Box<2> window_to_world(const Box<2> &box) const {
        return {window_to_world(box.min), window_to_world(box.max)};
    }

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

    void set_view(const Pos<2> view) { view_ = view; }

    pure const Map<Actor, List<Message>> &messages() const { return messages_; }

    mutable Random random;

protected:
    using EntityHash = PointerHash<Ref<Entity<N>>>;

    void tick_entity(Set<Actor> &idled, Ref<Entity<N>> entity);

    EntityTree entities_;
    Set<Actor> awake_;
    Set<Actor> died_;
    Map<Actor, List<Message>> messages_;

    Pos<2> view_ = Pos<2>::zero;
    bool hud_ = true;
    Duration draw_last_, draw_max_;
    Duration tick_last_, tick_max_;
    U64 msgs_last_ = 0, msgs_max_ = 0;
    bool debug_ = false;
};

template <U64 N>
void World<N>::tick() {
    const Time start = Clock::now();
    msgs_last_ = 0;

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

    tick_last_ = Clock::now() - start;
    tick_max_ = max(tick_max_, tick_last_);
    msgs_max_ = std::max(msgs_max_, msgs_last_);
}

template <U64 N>
void World<N>::draw() {
    const Time start = Clock::now();

    const Box<2> range = window_to_world(window_->bbox());
    const Pos<2> center = window_to_world(window_->center());
    {
        const auto offset = Window::Offset(window_, view_);
        for (const Actor &actor : entities(range)) {
            actor->draw(window_, Color::kNormal);
        }
    }

    if (hud_) {
        constexpr Color crosshair_color = Color::kBlack;
        const auto c = window_->center();
        const Box<2> hline({c[0] - 10, c[1] - 1}, {c[0] + 10, c[1] + 1});
        const Box<2> vline({c[0] - 1, c[1] - 10}, {c[0] + 1, c[1] + 10});
        window_->line_rectangle(crosshair_color, hline);
        window_->line_rectangle(crosshair_color, vline);
    }

    std::string hover = "None";
    if (auto over = this->entities(center); !over.empty()) {
        if (auto *entity = over.begin()->template dyn_cast<Entity<N>>()) {
            hover = entity->bbox().to_string();
        }
    }

    if (debug_) {
        window_->text(Color::kBlack, {10, 10}, 20, "FPS: " + std::to_string(window_->fps()));
        window_->text(Color::kBlack, {10, 40}, 20, "VRange: " + range.to_string());
        window_->text(Color::kBlack, {10, 70}, 20, "Center: " + center.to_string());
        window_->text(Color::kBlack, {10, 100}, 20, "Alive: " + std::to_string(num_alive()));
        window_->text(Color::kBlack, {10, 130}, 20, "Awake: " + std::to_string(num_awake()));
        window_->text(Color::kBlack, {10, 160}, 20, "Hover: " + hover);
        window_->text(Color::kBlack, {10, 190}, 20, "Tick(last): " + tick_last_.to_string());
        window_->text(Color::kBlack, {10, 220}, 20, "Tick(max): " + tick_max_.to_string());
        window_->text(Color::kBlack, {10, 250}, 20, "Draw(last): " + draw_last_.to_string());
        window_->text(Color::kBlack, {10, 280}, 20, "Draw(max): " + draw_max_.to_string());
        window_->text(Color::kBlack, {10, 310}, 20, "Msgs(last): " + std::to_string(msgs_last_));
        window_->text(Color::kBlack, {10, 340}, 20, "Msgs(max): " + std::to_string(msgs_max_));

        const auto offset = Window::Offset(window_, view_);
        {
            for (const auto &[node, pos] : entities_.points_in(range)) {
                const Box<N> box(pos, pos + node->grid - 1);
                window_->line_rectangle(Color::kRed, box);
            }
        }
    }

    draw_last_ = Clock::now() - start;
    draw_max_ = max(draw_max_, draw_last_);
}

template <U64 N>
void World<N>::tick_entity(Set<Actor> &idled, Ref<Entity<N>> entity) {
    static const List<Message> kNoMessages = {};

    const Actor actor = entity->self();
    const Box<N> prev_bbox = entity->bbox();
    const auto messages_iter = messages_.find(actor);
    // Making a copy here to allow clearing the message queue early (prior to running tick)
    const List<Message> messages = messages_iter == messages_.end() ? kNoMessages : messages_iter->second;
    if (messages_iter != messages_.end()) {
        messages_.erase(messages_iter);
    }
    msgs_last_ += messages.size();
    const Status status = entity->tick(messages);
    if (status == Status::kDied) {
        died_.insert(actor);
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

} // namespace nvl
