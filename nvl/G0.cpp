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
    explicit Jump(Actor src) : AbstractMessage(std::move(src)) {}
};
struct Move final : AbstractMessage {
    class_tag(Move, AbstractMessage);
    explicit Move(Actor src, const Dir dir) : AbstractMessage(std::move(src)), dir(dir) {}
    Dir dir;
};
struct Brake final : AbstractMessage {
    class_tag(Brake, AbstractMessage);
    explicit Brake(Actor src) : AbstractMessage(std::move(src)) {}
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
            velocity_[1] = -100;
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
        for (const At<2, Part<2>> &part : this->parts()) {
            const auto color = part->material->color.highlight(scale);
            window->fill_rectangle(color, part.bbox());
        }
    }

    Status broken(const List<Component> &) override { return Status::kNone; }
};

struct G0 final : World<2> {
    class_tag(G0, World<2>);

    explicit G0(Window *window) : World<2>(window, {.gravity_accel = 2}) {
        const Material bulwark = Material::get<Bulwark>();
        const Pos<2> min = {0, window->height() - 50};
        const Pos<2> max = {window->width(), window->height()};
        const Box<2> base(min, max);
        spawn<Block<2>>(Pos<2>::zero, base, bulwark);
        prev_tick = Clock::now();
        player = spawn<Player>(Pos<2>{window->center()[0], -30});

        on_key_down[Key::P] = [this] { paused = (paused > 0) ? 0 : 255; };
    }

    void tick() override {
        if (paused) {
            paused += 10 * pause_dir;
            if (paused > 255 || pause_dir < 1) {
                pause_dir = -pause_dir;
                paused = std::clamp<U64>(paused, 1, 255);
            }
            return;
        }
        const auto prev_player_loc = player->loc();
        World<2>::tick();
        const auto diff = player->loc() - prev_player_loc;
        view_ += diff;
    }

    void draw() override {
        World::draw();
        if (paused) {
            const Color color = Color::kBlack.highlight({.a = paused});
            window_->centered_text(color, window_->center(), 50, "PAUSED [P]");
        }
    }

    Player *player;
    Time prev_tick;
    U64 paused = 1;
    Dir pause_dir = Dir::Pos;
};

struct PlayerControls final : Tool<2> {
    explicit PlayerControls(Window *window, G0 *world) : Tool(window, world), g0(world) {}
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
    G0 *g0;
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
    auto *world = window.open<nvl::G0>();
    window.open<nvl::PlayerControls>(world);

    Time prev_tick = Clock::now();
    while (!window.should_close()) {
        if (const Time now = Clock::now(); Duration(now - prev_tick) >= world->kNanosPerTick) {
            prev_tick = now;
            window.tick();
        }
        window.feed();
        window.draw();
    }
}
