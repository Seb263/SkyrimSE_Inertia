#pragma once

class SettingsIni
{
public:
	// General
	static inline int  iVerboseMode = 1;
	
	// Inertia
	static inline float fDuration = 5.0f;
	static inline bool  bApplyOnHit = false;

	static bool ReadSettings()
	{
		constexpr auto path = L"Data/SKSE/Plugins/Inertia.ini";

		if (!std::filesystem::exists(path)) return false;

		CSimpleIniA ini;
		ini.SetUnicode();
		SI_Error rc = ini.LoadFile(path);

		if (rc < 0) return false;

		// General
		iVerboseMode = ini.GetLongValue("General", "iVerboseMode", 1);

		// Inertia
		fDuration = static_cast<float>(ini.GetDoubleValue("Inertia", "fDuration", 5.0f));
		fDuration = std::clamp(fDuration, 0.0f, 30.0f);

		bApplyOnHit = ini.GetBoolValue("Inertia", "bApplyOnHit", false);

		debugVerboseMode = iVerboseMode;

		return true;
	}
};
