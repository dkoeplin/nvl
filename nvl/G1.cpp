#include <utility>

#include "nvl/entity/Entity.h"
#include "nvl/material/Bulwark.h"
#include "nvl/reflect/Backtrace.h"
#include "nvl/tool/ToolBelt.h"
#include "nvl/ui/RayWindow.h"
#include "nvl/world/World.h"
#include "raylib.h"

namespace nvl {

struct Jump final : AbstractMessage {
    class_tag(Jump, AbstractMessage);
    explicit Jump(Actor src) : AbstractMessage(std::move(src)) {}
};
struct Strafe final : AbstractMessage {
    class_tag(Strafe, AbstractMessage);
    explicit Strafe(Actor src, const Dir dir) : AbstractMessage(std::move(src)), dir(dir) {}
    Dir dir;
};
struct Move final : AbstractMessage {
    class_tag(Move, AbstractMessage) explicit Move(Actor src, const Dir dir)
        : AbstractMessage(std::move(src)), dir(dir) {}
    Dir dir;
};
struct Brake final : AbstractMessage {
    class_tag(Brake, AbstractMessage);
    explicit Brake(Actor src) : AbstractMessage(std::move(src)) {}
};

struct Player final : Entity<3> {
    static constexpr I64 kMaxVelocity = 10;
    class_tag(Player, Entity<2>);
    explicit Player(const Pos<3> &loc) : Entity(loc) {
        const auto material = Material::get<TestMaterial>(Color::kBlue);
        parts_.emplace(Box<3>({-5, 0, -5}, {5, 10, 5}), material);
        parts_.emplace(Box<3>({-10, 10, -10}, {10, 30, 10}), material);
    }
    Status receive(const Message &message) override {
        if (message.isa<Jump>() && has_below() && velocity_[1] == 0) {
            std::cout << "Jump" << std::endl;
            velocity_[1] = -30;
        } else if (message.isa<Brake>()) {
            const I64 v0 = velocity_[0];
            const I64 v2 = velocity_[2];
            std::cout << "Brake" << std::endl;
            velocity_[0] = v0 + (v0 < 0 ? 1 : v0 > 0 ? -1 : 0);
            velocity_[2] = v2 + (v2 < 0 ? 1 : v2 > 0 ? -1 : 0);
        } else if (const auto strafe = message.dyn_cast<Strafe>()) {
            std::cout << "Strafe" << std::endl;
            velocity_[0] = std::clamp<I64>(velocity_[0] + 2 * strafe->dir, -kMaxVelocity, kMaxVelocity);
        } else if (const auto move = message.dyn_cast<Move>()) {
            std::cout << "Move" << std::endl;
            velocity_[2] = std::clamp<I64>(velocity_[2] + 2 * strafe->dir, -kMaxVelocity, kMaxVelocity);
        } else {
            return Entity::receive(message);
        }
        return Status::kNone;
    }

    void draw(Window *, const Color &) const override {
        /*for (const At<3, Part<3>> &part : this->parts()) {
            const auto color = part->material->color.highlight(scale);
            window->fill_box(color, part.bbox());
        }
        if (digging) {
            const auto color = Color::kBlue.highlight({.a = 100});
            const auto bbox = this->bbox();
            const Box<2> dig_box{bbox.min - 10, {bbox.max[0] + 10, bbox.max[1]}};
            window->fill_box(color, dig_box);
        }*/
    }

    Status tick(const List<Message> &messages) override {
        /*const auto bbox = this->bbox();
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
                    send<Notify>(overlap, Notify::kStrafed);
                }
            }
        }*/
        return Entity::tick(messages);
    }

    Status broken(const List<Component> &) override { return Status::kNone; }

    bool digging = false;
};

struct G1 final : World<3> {
    class_tag(G1, World<3>);

    explicit G1(Window *window) : World(window, {.gravity_accel = 3}) {
        const Material bulwark = Material::get<Bulwark>();
        const Pos<3> min = {0, 1000, 0};
        const Pos<3> max = {2000, 1050, 2000};
        const Box<3> base(min, max);
        spawn<Block<3>>(Pos<3>::zero, base, bulwark);

        const Pos<3> start{500, 950, 500};
        player = spawn<Player>(start);
        view3d().offset = start;

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
        World::tick();
        const auto diff = player->loc() - prev_player_loc;
        view3d().offset += diff;

        if (window_->ticks() - prev_generated >= ticks_per_gen) {
            const auto slots = ceil_div(1000, 50);
            const auto left = random.uniform<I64, I64>(-4, slots + 4);
            const auto back = random.uniform<I64, I64>(-4, slots + 4);
            const auto width = random.uniform<I64, I64>(1, 5);
            const auto height = random.uniform<I64, I64>(1, 5);
            const auto depth = random.uniform<I64, I64>(1, 5);
            const auto top = std::min<I64>(0, entities_.bbox().min[1]) - height * 50 - 200;
            const auto color = random.uniform<Color>(0, 255);
            const auto material = Material::get<TestMaterial>(color);
            const Pos<3> pos{left * 50, top, back};
            const Box<3> box{{0, 0, 0}, {width * 50, height * 50, depth * 50}};
            spawn<Block<3>>(pos, box, material);
            prev_generated = window_->ticks();
        }
    }

    void draw() override {
        World::draw();
        if (paused) {
            const Color color = Color::kBlack.highlight({.a = paused});
            window_->centered_text(color, window_->center(), 50, "PAUSED [P]");
        }
    }

    View3D &view3d() { return *view_.dyn_cast<View3D>(); }

    Player *player;
    U64 paused = 1;
    Dir pause_dir = Dir::Pos;
    U64 prev_generated = 0;
    U64 ticks_per_gen = 50;
};

struct PlayerControls final : Tool<3> {
    explicit PlayerControls(Window *window, G1 *world) : Tool(window, world), g1(world) {
        on_key_down[Key::J] = [this] { g1->player->digging = true; };
        on_key_up[Key::J] = [this] { g1->player->digging = false; };
    }
    void tick() override {
        Actor player(g1->player);
        if (window_->pressed(Key::Space)) {
            world_->send<Jump>(nullptr, player);
        }
        bool no_strafe = false, no_move = false;
        if (window_->pressed(Key::A)) {
            world_->send<Strafe>(nullptr, player, Dir::Neg);
        } else if (window_->pressed(Key::D)) {
            world_->send<Strafe>(nullptr, player, Dir::Pos);
        } else {
            no_strafe = true;
        }
        if (window_->pressed(Key::W)) {
            world_->send<Move>(nullptr, player, Dir::Pos);
        } else if (window_->pressed(Key::S)) {
            world_->send<Move>(nullptr, player, Dir::Neg);
        } else {
            no_move = true;
        }
        if (no_strafe && no_move) {
            world_->send<Brake>(nullptr, player);
        }
    }
    void draw() override {}
    G1 *g1;
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
    auto *world = window.open<nvl::G1>();
    window.open<nvl::PlayerControls>(world);

    Time prev_tick = Clock::now();
    while (!window.should_close()) {
        if (const Time now = Clock::now(); Duration(now - prev_tick) >= world->kNanosPerTick) {
            prev_tick = now;
            window.tick_all();
        }
        window.feed();
        window.draw();
    }
}
