#pragma once

#include <utility>

#include "nvl/entity/Entity.h"
#include "nvl/macros/Aliases.h"
#include "nvl/reflect/ClassTag.h"

namespace nvl {

template <U64 N>
class Block : public Entity<N> {
public:
    class_tag(Block<N>, Entity<N>);

    explicit Block(const Box<N> &box, Material material) : Entity<N>(), material_(std::move(material)) {
        this->parts_.emplace(box, material_);
    }

    explicit Block(Range<Ref<Part<N>>> parts) : Entity<N>(parts) {
        if (!this->relative.parts().empty()) {
            material_ = this->relative.parts().begin()->raw().material();
        }
    }

    void draw(Draw &draw, const I64 highlight) override {
        Draw::Offset offset(draw, this->loc());
        for (const Ref<Part<N>> &part : this->relative.parts()) {
            part->draw(draw, highlight);
        }
        for (const Ref<Edge<N>> &edge : this->relative.edges()) {
            edge->draw(draw, highlight);
        }
    }

    pure bool falls() const override { return material_->falls; }

protected:
    using Component = typename Entity<N>::Component;
    Status broken(const List<Component> &components) override {
        for (const Component &component : components) {
            this->world_->template spawn<Block<N>>(component.values());
        }
        return Status::kDied;
    }

    Material material_;
};

} // namespace nvl
