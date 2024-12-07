#pragma once

#include "a2/macros/Literals.h"
#include "nvl/entity/Entity.h"

namespace a2 {

using namespace nvl;

struct Player final : Entity<3> {
    class_tag(Player, Entity<3>);

    static constexpr I64 kMaxVelocity = 10_mps;
    static constexpr I64 kDigDist = 3_m;
    static constexpr I64 kDigRadius = 5_m;
    static constexpr I64 kViewDistance = 50_m;

    static constexpr U64 kRespawnTicks = 100;
    static constexpr U64 kDigTicks = 5;

    explicit Player(const Pos<3> &loc);

    Pos<3> &x() { return parts_.loc; }
    Pos<3> &a() { return accel_; }
    Pos<3> &v() { return velocity_; }

    pure World<3> *world() const { return world_; }

    Status receive(const Message &message) override;
    void draw(Window *, const Color &) const override;
    Status tick(const List<Message> &messages) override;
    Status broken(const List<Component> &) override;

    U64 last_dig = 0;
    Pos<3> spawn;
};

} // namespace a2
