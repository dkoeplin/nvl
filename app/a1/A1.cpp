#include <utility>

#include "nvl/entity/Entity.h"
#include "nvl/material/Bulwark.h"
#include "nvl/reflect/Backtrace.h"
#include "nvl/tool/ToolBelt.h"
#include "nvl/ui/RayWindow.h"
#include "nvl/world/World.h"

namespace nvl {

struct Jump final : AbstractMessage {
    class_tag(Jump, AbstractMessage);
    explicit Jump(AbstractActor *src) : AbstractMessage(src) {}
};
struct Move final : AbstractMessage {
    class_tag(Move, AbstractMessage);
    explicit Move(AbstractActor *src, const Dir dir) : AbstractMessage(src), dir(dir) {}
    Dir dir;
};
struct Brake final : AbstractMessage {
    class_tag(Brake, AbstractMessage);
    explicit Brake(AbstractActor *src) : AbstractMessage(src) {}
};

struct Player final : Entity<2> {
    static constexpr I64 kMaxVelocity = 10;
    class_tag(Player, Entity<2>);
    explicit Player(const Pos<2> loc) : Entity(loc) {
        const auto material = Material::get<TestMaterial>(Color::kGreen);
        parts_.emplace(Box<2>({-5, 0}, {5, 10}), material);
        parts_.emplace(Box<2>({-10, 10}, {10, 30}), material);
    }
    Status receive(const Message &message) override {
        if (message.isa<Jump>() && has_below() && velocity_[1] == 0) {
            velocity_[1] = -30;
        } else if (message.isa<Brake>()) {
            const I64 v = velocity_[0];
            velocity_[0] = v + (v < 0 ? 1 : v > 0 ? -1 : 0);
        } else if (const auto move = message.dyn_cast<Move>()) {
            velocity_[0] = std::clamp<I64>(velocity_[0] + 2 * move->dir, -kMaxVelocity, kMaxVelocity);
        } else {
            return Entity::receive(message);
        }
        return Status::kNone;
    }

    void draw(Window *window, const Color &scale) const override {
        for (const At<2, Part> &part : this->parts()) {
            const auto color = part->material->color.highlight(scale);
            window->fill_box(color, part.bbox());
        }
        if (digging) {
            const auto color = Color::kBlue.highlight({.a = 100});
            const auto bbox = this->bbox();
            const Box<2> dig_box{bbox.min - 10, {bbox.max[0] + 10, bbox.max[1]}};
            window->fill_box(color, dig_box);
        }
    }

    Status tick(const List<Message> &messages) override {
        const auto bbox = this->bbox();
        Box<2> dig_box{bbox.min - 10, {bbox.max[0] + 10, bbox.max[1]}};
        if (digging) {
            if (velocity_[1] < 0) {
                dig_box.min[1] += velocity_[1];
            }
            for (const Actor &overlap : world_->entities(dig_box)) {
                if (overlap != self()) {
                    send<Hit<2>>(overlap, dig_box, 1);
                }
            }
        } else if (velocity_ != Pos<2>::zero) {
            for (const Actor &overlap : world_->entities(dig_box)) {
                if (overlap != self()) {
                    send<Notify>(overlap, Notify::kMoved);
                }
            }
        }
        return Entity::tick(messages);
    }

    Status broken(const List<Component> &) override { return Status::kNone; }

    bool digging = false;
};

struct A1 final : World<2> {
    class_tag(A1, World<2>);

    explicit A1(AbstractScreen *window) : World(window, {.gravity_accel = 3}) {
        const Material bulwark = Material::get<Bulwark>();
        const Pos<2> min = {0, window_->height() - 50};
        const Pos<2> max = {window_->width(), window_->height()};
        const Box<2> base(min, max);
        spawn<Block<2>>(Pos<2>::zero, base, bulwark);

        const Pos<2> start{window_->center()[0], min[1] - 100};
        player = spawn<Player>(start);
        view_ = ViewOffset::at(start - window_->shape() / 2);

        on_key_down[Key::P] = [this] { paused = (paused > 0) ? 0 : 255; };
    }

    View2D &view2d() { return *view_.dyn_cast<View2D>(); }

    void tick() override {
        auto &view = view2d();
        if (paused) {
            paused += 10 * pause_dir;
            if (paused > 255 || pause_dir < 1) {
                pause_dir = -pause_dir;
                paused = std::clamp<U64>(paused, 1, 255);
            }
            return;
        }
        const auto prev_player_loc = player->loc();
        World::tick();
        const auto diff = player->loc() - prev_player_loc;
        view.offset += diff;

        if (ticks() - prev_generated >= ticks_per_gen) {
            const auto slots = ceil_div(window_->width(), 50);
            const auto left = random.uniform<I64, I64>(-4, slots);
            const auto width = random.uniform<I64, I64>(1, 5);
            const auto height = random.uniform<I64, I64>(1, 3);
            const auto top = std::min(view.offset[1], entities_.bbox().min[1]) - height * 50 - 200;
            const auto color = random.uniform<Color>(0, 255);
            const auto material = Material::get<TestMaterial>(color);
            const Pos<2> pos{left * 50, top};
            const Box<2> box{{0, 0}, {width * 50, height * 50}};
            spawn<Block<2>>(pos, box, material);
            prev_generated = ticks();
        }
    }

    void draw() override {
        World::draw();
        if (paused) {
            const Color color = Color::kBlack.highlight({.a = paused});
            window_->centered_text(color, window_->center(), 50, "PAUSED [P]");
        }
    }

    Player *player;
    U64 paused = 1;
    Dir pause_dir = Dir::Pos;
    U64 prev_generated = 0;
    U64 ticks_per_gen = 10;
};

struct PlayerControls final : Tool<2> {
    explicit PlayerControls(AbstractScreen *window, A1 *world) : Tool(window, world), g0(world) {
        on_key_down[Key::J] = [this] { g0->player->digging = true; };
        on_key_up[Key::J] = [this] { g0->player->digging = false; };
    }
    void tick() override {
        Actor player(g0->player);
        if (window_->pressed(Key::Space)) {
            world_->send<Jump>(nullptr, player);
        }
        if (!window_->pressed(Key::A) && !window_->pressed(Key::D)) {
            world_->send<Brake>(nullptr, player);
        } else if (window_->pressed(Key::A)) {
            world_->send<Move>(nullptr, player, Dir::Neg);
        } else if (window_->pressed(Key::D)) {
            world_->send<Move>(nullptr, player, Dir::Pos);
        }
    }
    void draw() override {}
    A1 *g0;
};

} // namespace nvl

using nvl::Clock;
using nvl::Duration;
using nvl::RayWindow;
using nvl::Time;
using nvl::ToolBelt;
using nvl::Window;
using nvl::World;

int main() {
    nvl::register_signal_handlers();

    RayWindow window("App", {1000, 1000});
    window.set_mouse_mode(Window::MouseMode::kViewport);
    auto *world = window.open<nvl::A1>();
    window.open<nvl::PlayerControls>(world);

    Time prev_tick = Clock::now();
    while (!window.should_close()) {
        if (const Time now = Clock::now(); Duration(now - prev_tick) >= world->kNanosPerTick) {
            prev_tick = now;
            window.tick();
        }
        window.react();
        window.draw();
    }
}
