#include <Atrc/Core/Scene.h>

AGZ_NS_BEG(Atrc)

const Camera* Scene::GetCamera() const
{
    AGZ_ASSERT(camera);
    return camera;
}

bool Scene::HasIntersection(const Ray &r) const
{
    for(auto ent : entities_)
    {
        if(ent->HasIntersection(r))
            return true;
    }
    return false;
}

bool Scene::FindCloestIntersection(const Ray &r, SurfacePoint *sp) const
{
    bool ret = false;
    for(auto ent : entities_)
    {
        SurfacePoint newSp;
        if(ent->FindIntersection(r, &newSp) && (!ret || newSp.t < sp->t))
        {
            ret = true;
            *sp = newSp;
        }
    }
    return ret;
}

AGZ_NS_END(Atrc)
