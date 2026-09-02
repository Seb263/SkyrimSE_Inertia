#pragma once

#include "SettingsIni.hpp"

#define FRAME_DELAY_MS() std::chrono::milliseconds(static_cast<int>(std::lround(ModUtils::GetFrameDelay() * 1000.0f)))

class ModUtils
{
public:

	template <typename TCallback>
	static void WaitUntilRagdollReady(RE::TESObjectREFR* ref, TCallback&& callback, std::chrono::milliseconds timeout = 3s, const bool secureFrame = true)
	{
		if (!ref) {
			callback(ref, false);
			return;
		}

		std::jthread([formId = ref->formID, callback = std::forward<TCallback>(callback), timeout, secureFrame]() mutable {
			auto start = std::chrono::steady_clock::now();
			while (std::chrono::steady_clock::now() - start < timeout) {
				const std::chrono::milliseconds delay = FRAME_DELAY_MS();
				RE::TESObjectREFR* ref = RE::TESForm::LookupByID<RE::TESObjectREFR>(formId);
				if (ref && ModUtils::IsReferenceRagdollReady(ref)) {
					SKSE::GetTaskInterface()->AddTask([callback, ref, secureFrame, start = std::chrono::steady_clock::now(), delay]() {
						if (secureFrame && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() > 300 + delay.count()) {
							TRACE("WaitUntilRagdollReady: Task was delayed and invalidated due to frame timing (>{}ms)", 300 + delay.count());
							return;
						}
						callback(ref, true);
					});
					return;
				}
				std::this_thread::sleep_for(delay);
			}
        
			SKSE::GetTaskInterface()->AddTask([callback, formId, secureFrame, start = std::chrono::steady_clock::now()]() {
				if (secureFrame && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() > 300) {
					TRACE("WaitUntilRagdollReady: Task was delayed and invalidated due to frame timing (>{}ms)", 300);
					return;
				}
				RE::TESObjectREFR* ref = RE::TESForm::LookupByID<RE::TESObjectREFR>(formId);
				callback(ref, false);
			});
		}).detach();
	}

	static bool IsReferenceRagdollReady(RE::TESObjectREFR* ref)
	{
		if (!ref || !ref->Is3DLoaded()) return false;

		RE::NiAVObject* niAVObject = ref->Get3D(false);
		if (!niAVObject) return false;

		if (RE::NiPointer<RE::NiAVObject> root = RE::NiPointer<RE::NiAVObject>(niAVObject)) {
			if (auto rb = GetRigidBody(root.get())) {
				if (auto hkpRigidBody = static_cast<RE::hkpRigidBody*>(rb->referencedObject.get())) {
					if (hkpRigidBody->world && hkpRigidBody->motion.GetMass() > 0.0f) return true;
				}
			}
		}

		return false;
	}

	static RE::bhkRigidBody* GetRigidBody(RE::NiAVObject* a_object)
	{
		if (auto collisionObject = a_object->GetCollisionObject()) {
			return collisionObject->GetRigidBody();
		}
		return nullptr;
	}

	static float GetFrameDelay()
	{
		RE::BSTimer* bsTimer = RE::BSTimer::GetSingleton();
		if (!bsTimer) return 0.00694444f; // 144Hz

		float frame_delay = bsTimer->realTimeDelta / bsTimer->QGlobalTimeMultiplier();
		if (frame_delay < 0.004f) frame_delay = 0.004f;

		return frame_delay;
	}
};
