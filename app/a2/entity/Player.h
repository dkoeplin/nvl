#pragma once

#include "a2/macros/Literals.h"
#include "nvl/entity/Entity.h"

namespace a2 {

using namespace nvl;

struct Player final : Entity<3> {
    class_tag(Player, Entity<3>);

    explicit Player(const Pos<3> &loc);

    Pos<3> &x() { return parts_.loc; }
    Pos<3> &a() { return accel_; }
    Pos<3> &v() { return velocity_; }

    pure World<3> *world() const { return world_; }

    Status receive(const Message &message) override;
    void draw(Window *, const Color &) const override;
    Status tick(const List<Message> &messages) override;
    Status broken(const List<Set<Rel<Part>>> &) override;

    pure I64 dig_ticks() const { return 5; }
    pure I64 dig_reach() const { return 3_m; }
    pure I64 dig_radius() const { return 1_m; }
    pure I64 walk_accel() const { return 30_mps2; }
    pure I64 walk_max_velocity() const { return 10_mps; }

    U64 last_dig = 0;
    Pos<3> spawn;
};

} // namespace a2
