#include "../EntityManager/EntityCreator.h"
#include "../LightManager/LightManager.h"
#include "../ParamParser/ParamParser.h"
#include "SceneManager.h"

namespace
{
	const Atrc::Camera *ParseCamera(const ConfigGroup &params, Atrc::Real aspectRatio, ObjArena<> &arena)
	{
		auto eye = ParamParser::ParseVec3(params["eye"]);
		auto dst = ParamParser::ParseVec3(params["dst"]);
		auto dir = dst - eye;

		auto up = ParamParser::ParseVec3(params["up"]);

		Atrc::Rad rad;
		auto &angle = params["FOVy"].AsArray();
		if(angle.Size() != 2)
			throw std::runtime_error("Scene: invalid angle form for FOVy");
		if(angle[0].AsValue() == "Rad")
			rad = Atrc::Rad(angle[1].AsValue().Parse<Atrc::Real>());
		else if(angle[0].AsValue() == "Deg")
			rad = Atrc::Deg(angle[1].AsValue().Parse<Atrc::Real>());
		else
			throw std::runtime_error("Scene: invalid angle form for FOVy");

		return arena.Create<Atrc::PerspectiveCamera>(
			eye, dir, up, rad, aspectRatio);
	}
}

void SceneManager::Initialize(const ConfigGroup &params)
{
	if(IsAvailable())
		throw std::runtime_error("Scene: reinitialized");

	auto outputWidth  = params["output.width"].AsValue().Parse<uint32_t>();
	auto outputHeight = params["output.height"].AsValue().Parse<uint32_t>();
	auto aspectRatio = Atrc::Real(outputWidth) / outputHeight;

	auto camera = ParseCamera(params["camera"].AsGroup(), aspectRatio, arena_);

	std::vector<Atrc::Entity*> entities;
	std::vector<Atrc::Light*> lights;

	auto &entArr = params["entities"].AsArray();
	for(size_t i = 0; i < entArr.Size(); ++i)
	{
		auto ent = EntityManager::GetInstance().Create(
			entArr[i].AsGroup());
		if(!ent)
			throw std::runtime_error("SceneManager: unknown entity type");
		entities.push_back(ent);
		
		auto light = ent->AsLight();
		if(light)
			lights.push_back(light);
	}

	auto &lgtArr = params["lights"].AsArray();
	for(size_t i = 0; i < lgtArr.Size(); ++i)
	{
		auto light = LightManager::GetInstance().Create(
			lgtArr[i].AsGroup());
		if(!light)
			throw std::runtime_error("SceneManager: unknown light type");
		lights.push_back(light);
	}

	scene_.camera = camera;

	for(auto ent : entities)
		scene_.entities_.push_back(ent);
	for(auto lgt : lights)
		scene_.lights_.push_back(lgt);

	for(auto light : lights)
		light->PreprocessScene(scene_);
}

bool SceneManager::IsAvailable() const
{
	return scene_.camera != nullptr;
}

const Atrc::Scene &SceneManager::GetScene() const
{
	if(!IsAvailable())
		throw std::runtime_error("Scene: uninitialized");
	return scene_;
}
