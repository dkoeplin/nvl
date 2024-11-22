#include <utility>

#include "math/Trig.h"
#include "nvl/entity/Entity.h"
#include "nvl/material/Bulwark.h"
#include "nvl/reflect/Backtrace.h"
#include "nvl/tool/ToolBelt.h"
#include "nvl/ui/GlowEffect.h"
#include "nvl/ui/RayWindow.h"
#include "nvl/world/World.h"

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

struct Teleport final : AbstractMessage {
    class_tag(Teleport, AbstractMessage);
    explicit Teleport(Actor src, const Pos<3> &dst) : AbstractMessage(std::move(src)), dst(dst) {}
    Pos<3> dst;
};

struct Player final : Entity<3> {
    class_tag(Player, Entity<3>);

    static constexpr I64 kMaxVelocity = 10;
    static constexpr I64 kDigTicks = 5;
    static constexpr I64 kDigDist = 100;
    static constexpr I64 kDigRadius = 5;

    explicit Player(const Pos<3> &loc) : Entity(loc) {
        const auto material = Material::get<TestMaterial>(Color::kBlue);
        parts_.emplace(Box<3>({-5, 0, -5}, {5, 10, 5}), material);
        parts_.emplace(Box<3>({-10, 10, -10}, {10, 30, 10}), material);
    }

    Status receive(const Message &message) override {
        if (message.isa<Jump>() && has_below() && velocity_[1] == 0) {
            velocity_[1] = -30;
            return Status::kMove;
        }
        if (message.isa<Brake>()) {
            const I64 v0 = velocity_[0];
            const I64 v2 = velocity_[2];
            velocity_[0] = v0 > 0 ? std::max<I64>(v0 - 2, 0) : std::min<I64>(v0 + 2, 0);
            velocity_[2] = v2 > 0 ? std::max<I64>(v2 - 2, 0) : std::min<I64>(v2 + 2, 0);
            return Status::kMove;
        }
        const auto *view = world_->view().dyn_cast<View3D>();

        if (const auto strafe = message.dyn_cast<Strafe>()) {
            const float delta_x = std::round(strafe->dir * 2 * std::cos((view->angle - 90) * kDeg2Rad));
            const float delta_z = std::round(strafe->dir * 2 * std::sin((view->angle - 90) * kDeg2Rad));
            velocity_[0] = std::clamp<I64>(velocity_[0] + delta_x, -kMaxVelocity, kMaxVelocity);
            velocity_[2] = std::clamp<I64>(velocity_[2] + delta_z, -kMaxVelocity, kMaxVelocity);
            return Status::kMove;
        }
        if (const auto move = message.dyn_cast<Move>()) {
            const float delta_x = std::round(move->dir * 2 * std::cos(view->angle * kDeg2Rad));
            const float delta_z = std::round(move->dir * 2 * std::sin(view->angle * kDeg2Rad));
            velocity_[0] = std::clamp<I64>(velocity_[0] + delta_x, -kMaxVelocity, kMaxVelocity);
            velocity_[2] = std::clamp<I64>(velocity_[2] + delta_z, -kMaxVelocity, kMaxVelocity);
            return Status::kMove;
        }
        if (const auto *teleport = message.dyn_cast<Teleport>()) {
            parts_.loc = teleport->dst;
            return Status::kMove;
        }

        return Entity::receive(message);
    }

    void draw(Window *, const Color &) const override {
        // const auto box_color = Color::kBlue; //.highlight(color);
        //  const auto edge_color = color.highlight(Color::kDarker);
        // for (const At<3, Part<3>> &part : this->parts()) {
        //  window->fill_cube(box_color, part.bbox());
        //  window->fill_cube(edge_color, part.bbox());
        // }
    }

    Status tick(const List<Message> &messages) override {
        if (digging) {
            const auto now = world_->ticks();
            const auto time = last_dig.has_value() ? now - *last_dig : kDigTicks;
            if (time >= kDigTicks) {
                const auto *view = world_->view().dyn_cast<View3D>();
                const Pos<3> &start = view->offset;
                const Pos<3> end = view->project(kDigDist);
                const Line sight(start, end);
                if (auto itx = world_->first_except(sight, self())) {
                    const Pos<3> pt = Pos<3>::round(itx->pt);
                    const Box<3> dig_box(pt - 5, pt + 5);
                    send<Hit<3>>(itx->actor, dig_box, 1);
                    last_dig = Some(now);
                }
            }
        }
        return Entity::tick(messages);
    }

    Status broken(const List<Component> &) override { return Status::kNone; }

    Maybe<I64> last_dig = None;
    bool digging = false;
};

static const std::vector kColors = {
    Color::kLightGray,  Color::kGray,    Color::kDarkGray, Color::kYellow,    Color::kGold,   Color::kOrange,
    Color::kPink,       Color::kRed,     Color::kMaroon,   Color::kMagenta,   Color::kGreen,  Color::kLime,
    Color::kDarkGreen,  Color::kSkyBlue, Color::kBlue,     Color::kDarkBlue,  Color::kPurple, Color::kViolet,
    Color::kDarkPurple, Color::kBeige,   Color::kBrown,    Color::kDarkBrown,
};

struct G1 final : World<3> {
    class_tag(G1, World<3>);

    std::vector<Material> materials;

    explicit G1(Window *window) : World(window, {.gravity_accel = 3, .maximum_y = 3000}) {
        paused.enabled = true;
        window_->set_background(Color::kSkyBlue);

        for (const auto &color : kColors) {
            materials.push_back(Material::get<TestMaterial>(color));
        }

        const Material bulwark = Material::get<Bulwark>(Color::kDarkGreen);
        constexpr Box<3> base({0, 0, 0}, {2000, 50, 2000});
        spawn<Block<3>>(Pos<3>{0, 1000, 0}, base, bulwark);

        constexpr Pos<3> start{500, 950, 300};
        player = spawn<Player>(start);
        view3d().offset = start;
        view3d().angle = 90;
        view3d().pitch = 10;

        const std::vector colors{Color::kRed, Color::kGreen, Color::kBlue};
        I64 x = 500;
        constexpr Box<3> box{{0, 0, 0}, {49, 49, 49}};
        for (const auto &color : colors) {
            const Pos<3> loc{x, 950, 500};
            spawn<Block<3>>(loc, box, Material::get<TestMaterial>(color));
            x += 100;
        }
        const std::vector colors2{Color::kYellow, Color::kOrange, Color::kPurple};
        x = 500;
        for (const auto &color : colors2) {
            const Pos<3> loc{x, 850, 500};
            spawn<Block<3>>(loc, box, Material::get<TestMaterial>(color));
            x += 100;
        }

        on_key_down[Key::P] = [this] { paused.enabled = !paused.enabled; };
        on_key_down[Key::L] = [this] {
            for (const Actor &actor : entities_) {
                if (const auto *block = actor.dyn_cast<Block<3>>()) {
                    std::cout << block->bbox() << std::endl;
                } else if (const auto *p = actor.dyn_cast<Player>()) {
                    std::cout << "PLAYER: " << std::endl;
                    for (const At<3, Part<3>> &part : p->parts()) {
                        std::cout << "  " << part.bbox() << std::endl;
                    }
                }
            }
            std::cout << "VIEW: " << view3d().offset << std::endl;
        };
        on_key_down[Key::N] = [this] { spawn_random_cube(); };

        on_key_down[Key::Any] = [this, start] {
            if (dead && dead_count > kMinDeadTicks) {
                dead.enabled = false;
                dead_count = 0;
                send<Teleport>(nullptr, Actor(player), start);
                view3d().angle = 90;
                view3d().pitch = 10;
            }
        };
    }

    void remove(const Actor &actor) override {
        if (auto *p = actor.dyn_cast<Player>()) {
            dead.enabled = true;
        } else {
            World::remove(actor);
        }
    }

    void spawn_random_cube() {
        const auto slots = ceil_div(1000, 50);
        const auto left = random.uniform<I64, I64>(-4, slots + 4);
        const auto back = random.uniform<I64, I64>(-4, slots + 4);
        const auto width = random.uniform<I64, I64>(1, 5);
        const auto height = random.uniform<I64, I64>(1, 5);
        const auto depth = random.uniform<I64, I64>(1, 5);
        const auto top = std::min<I64>(0, entities_.bbox().min[1]) - height * 50 - 200;
        const auto color_idx = random.uniform<U64, U64>(0, materials.size() - 1);
        const auto material = materials.at(color_idx);
        const Pos<3> pos{left * 50, top, back * 50};
        const Box<3> box{{0, 0, 0}, {width * 50, height * 50, depth * 50}};
        spawn<Block<3>>(pos, box, material);
        prev_generated = ticks();
    }

    void tick() override {
        dead.advance();
        paused.advance();
        if (dead) {
            dead_count += 1;
        }
        if (dead || paused) {
            return;
        }
        const auto prev_player_loc = player->loc();
        World::tick();
        const auto diff = player->loc() - prev_player_loc;
        view3d().offset += diff;

#if 0
        if (window_->ticks() - prev_generated >= ticks_per_gen) {
            spawn_random_cube();
        }
#endif
    }

    void draw() override {
        World::draw();
        const auto center = window_->center();
        auto pos = center;
        pos[1] += 80;

        if (dead) {
            const Color color = Color::kRed.highlight(dead);
            window_->fill_box(color, Box<2>({0, 0}, window_->shape()));
            window_->centered_text(Color::kRed, window_->center(), 50, "OH NO YOU DIED");
            if (dead_count > kMinDeadTicks) {
                window_->centered_text(Color::kBlack, pos, 30, "Press Any Key to Respawn");
            }
        } else if (paused) {
            const Color color = Color::kBlack.highlight(paused);
            window_->fill_box(color, Box<2>({0, 0}, window_->shape()));
            window_->centered_text(Color::kBlack, window_->center(), 50, "PAUSED");
            window_->centered_text(Color::kBlack, pos, 30, "Press [P] to Resume");
        }
    }

    View3D &view3d() { return *view_.dyn_cast<View3D>(); }

    static constexpr U64 kMinDeadTicks = 100;

    Player *player;
    GlowEffect paused = GlowEffect(3, 512, 756);
    GlowEffect dead = GlowEffect(10, 512, 900);
    U64 dead_count = 0;
    U64 prev_generated = 0;
    U64 ticks_per_gen = 50;
};

struct PlayerControls final : Tool<3> {
    explicit PlayerControls(Window *window, G1 *world) : Tool(window, world), g1(world) {
        on_mouse_down[Mouse::Left] = [this] { g1->player->digging = true; };
        on_mouse_up[Mouse::Left] = [this] { g1->player->digging = false; };
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

struct DebugScreen final : AbstractScreen {
    DebugScreen(Window *window, G1 *world) : AbstractScreen(window), world_(world) {}

    void draw() override {
        const auto *view3d = world_->view().dyn_cast<View3D>();
        const Pos<3> offset = view3d->offset;
        const Pos<3> target = view3d->project(100);
        const Line<3> line{offset, target};
        const Actor player(world_->player);
        const auto itx = world_->first_except(line, player);
        std::string at = "N/A";
        if (itx.has_value()) {
            at = itx->actor.dyn_cast<Entity<3>>()->bbox().to_string();
        }

        window_->text(Color::kBlack, {10, 10}, 20, "FPS:  " + std::to_string(window_->fps()));
        window_->text(Color::kBlack, {10, 40}, 20, "Offset: " + offset.to_string());
        window_->text(Color::kBlack, {10, 70}, 20, "Target: " + target.to_string());
        window_->text(Color::kBlack, {10, 100}, 20, "At:    " + at);
        window_->text(Color::kBlack, {10, 130}, 20, "Angle: " + std::to_string(view3d->angle));
        window_->text(Color::kBlack, {10, 160}, 20,
                      "Alive: " + std::to_string(world_->num_awake()) + "/" + std::to_string(world_->num_alive()));
    }
    void tick() override {}

    G1 *world_;
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
    window.open<nvl::DebugScreen>(world);

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
