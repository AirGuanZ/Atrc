#include <Atrc/Core/Entity/TwoSideEntity.h>

namespace Atrc
{
    
TwoSideEntity::TwoSideEntity(Entity *internalEntity) noexcept
    : internal_(internalEntity)
{
    
}

bool TwoSideEntity::HasIntersection(const Ray &r) const noexcept
{
    return internal_->HasIntersection(r);
}

bool TwoSideEntity::FindIntersection(const Ray &r, Intersection *inct) const noexcept
{
    if(!internal_->FindIntersection(r, inct))
        return false;
    if(Dot(inct->coordSys.ez, inct->wr) < 0)
    {
        inct->coordSys = -inct->coordSys;
        inct->usr.coordSys = -inct->usr.coordSys;
        std::swap(inct->mediumInterface.in, inct->mediumInterface.out);
    }
    return true;
}

AABB TwoSideEntity::GetWorldBound() const
{
    return internal_->GetWorldBound();
}

Light *TwoSideEntity::AsLight() noexcept
{
    return internal_->AsLight();
}

const Light *TwoSideEntity::AsLight() const noexcept
{
    return internal_->AsLight();
}

} // namespace Atrc
