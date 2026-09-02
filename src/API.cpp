#include "API.h"
#include "Main.hpp"

size_t InertiaAPI::InertiaAPI::GetAPIVersion() const
{
	return INER_API_VERSION;
}

std::vector<uint32_t> InertiaAPI::InertiaAPI::GetVersion() const
{
	using namespace SKSE;
	const auto* plugin = PluginDeclaration::GetSingleton();
	auto        version = plugin->GetVersion();

	uint32_t versionMajor = plugin->GetVersion().major();
	uint32_t versionMinor = plugin->GetVersion().minor();
	uint32_t versionPatch = plugin->GetVersion().patch();

	std::vector<uint32_t> versionVector;
	versionVector.push_back(versionMajor);
	versionVector.push_back(versionMinor);
	versionVector.push_back(versionPatch);

	return versionVector;
}

void InertiaAPI::InertiaAPI::ApplyInertia(RE::TESObjectREFR* object, const float duration) const
{
	using namespace Events;

	if (!object) return;
	Events::MainEvent::ProceedObjectInertia(object, duration);
}
