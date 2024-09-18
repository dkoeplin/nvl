#include "nvl/material/Material.h"

namespace nvl {

pure std::string Material::name() const { return impl_->name; }
pure Color Material::color() const { return impl_->color; }
pure bool Material::falls() const { return impl_->falls; }
pure bool Material::liquid() const { return impl_->liquid; }
pure U64 Material::durability() const { return impl_->durability; }

} // namespace nvl