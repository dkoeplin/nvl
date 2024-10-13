#pragma once

#include <utility>

#include "nvl/entity/Entity.h"
#include "nvl/macros/Aliases.h"
#include "nvl/reflect/ClassTag.h"
#include "nvl/ui/Window.h"

namespace nvl {

template <U64 N>
class Block : public Entity<N> {
public:
    class_tag(Block<N>, Entity<N>);

    explicit Block(Pos<N> loc, const Box<N> &box, Material material) : Entity<N>(loc), material_(std::move(material)) {
        this->parts_.emplace(box, material_);
    }

    explicit Block(Pos<N> loc, Range<Ref<Part<N>>> parts) : Entity<N>(loc, parts) {
        if (!this->relative.parts().empty()) {
            material_ = this->relative.parts().begin()->raw().material;
        }
    }
    explicit Block(Pos<N> loc, Range<Part<N>> parts) : Entity<N>(loc, parts) {
        if (!this->relative.parts().empty()) {
            material_ = this->relative.parts().begin()->raw().material;
        }
    }

    void draw(Window *window, const Color &scale) const override {
        const auto color = material_->color.highlight(scale);
        for (const At<N, Part<N>> &part : this->parts()) {
            window->fill_rectangle(color, part.bbox());
        }
        if (material_->outline) {
            const auto edge_color = color.highlight(Color::kDarker);
            for (const At<N, Edge<N>> &edge : this->edges()) {
                window->line_rectangle(edge_color, edge.bbox());
            }
        }
    }

    pure bool falls() const override { return material_->falls; }

    pure Material material() const { return material_; }

protected:
    using Component = typename Entity<N>::Component;

    Status broken(const List<Component> &components) override {
        const Pos<N> loc = this->loc();
        for (const Component &component : components) {
            this->world_->template spawn<Block<N>>(loc, component.values());
        }
        return Status::kDied;
    }

    Material material_;
};

} // namespace nvl
