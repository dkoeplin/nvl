#include "a2/entity/Player.h"

#include "a2/action/PlayerAction.h"
#include "nvl/material/TestMaterial.h"

namespace a2 {

Player::Player(const Pos<3> &loc) : Entity(loc), spawn(loc) {
    const auto material = Material::get<TestMaterial>(Color::kBlue);
    parts_.emplace(Box<3>({-50, 0, -50}, {50, 50, 50}), material);
    parts_.emplace(Box<3>({-50, 50, -50}, {-50, 100, 50}), material);
}

Status Player::receive(const Message &message) {
    if (auto *action = message.dyn_cast<PlayerAction>()) {
        return action->act(*this);
    }
    return Entity::receive(message);
}

void Player::draw(Window *, const Color &) const {
    // const auto box_color = Color::kBlue; //.highlight(color);
    //  const auto edge_color = color.highlight(Color::kDarker);
    // for (const At<3, Part<3>> &part : this->parts()) {
    //  window->fill_cube(box_color, part.bbox());
    //  window->fill_cube(edge_color, part.bbox());
    // }
}

Status Player::tick(const List<Message> &messages) { return Entity::tick(messages); }

Status Player::broken(const List<Component> &) { return Status::kNone; }

} // namespace a2
