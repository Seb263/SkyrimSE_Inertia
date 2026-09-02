#pragma once

#include <shared_mutex>

#include "SettingsIni.hpp"

#include "Utils\ModUtils.hpp"

namespace Events
{
	class MainEvent
	{
		public:

			static void ProceedActorDeath(RE::Actor* actor)
			{
				if (!actor) return;

				ProceedObjectInertia(actor, SettingsIni::fDuration);
			};

			static void ProceedObjectInertia(RE::TESObjectREFR* object, const float duration = 0.0f)
			{
				if (!object) return;

				TRACE("Applying Intertia on <\"{}\" [REF:{:08X}] [BASE:{:08X}]>",
					object->GetName(), object->formID, (object->GetBaseObject() ? object->GetBaseObject()->formID : 0x0));

				ModUtils::WaitUntilRagdollReady(object, [duration](RE::TESObjectREFR* objectRef, const bool result) {
					if (!result || !objectRef) return;
					
					LoopObjectMotions(objectRef, (duration > 0.0f ? duration : SettingsIni::fDuration));
				});
			}

		private:

			inline static std::unordered_set<RE::FormID> busyObject;
			inline static std::shared_mutex              busyMutex;
			static void LoopObjectMotions(RE::TESObjectREFR* object, const float duration)
			{
				if (!object) return;

				{
					std::shared_lock lock(busyMutex);
					if (busyObject.contains(object->formID)) return;
				}

				{
					std::unique_lock lock(busyMutex);
					busyObject.insert(object->formID);
				}

				std::jthread([objectFormID = object->formID, duration]() {
					const auto start = std::chrono::steady_clock::now();
					const int loopDuration = static_cast<int>(duration * 1000.0f);

					while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() < loopDuration) {
						if (!GetInertiaObject(objectFormID)) break;

						SKSE::GetTaskInterface()->AddTask([objectFormID, start = std::chrono::steady_clock::now()]() {
							if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() > 300) return;
							
							RE::TESObjectREFR* tmpObject = GetInertiaObject(objectFormID);
							if (tmpObject) ProcessObjectMotions(tmpObject);
						});

						std::this_thread::sleep_for(FRAME_DELAY_MS());
					}

					{
						std::unique_lock lock(busyMutex);
						busyObject.erase(objectFormID);
					}
				}).detach();
			}

			static void ProcessObjectMotions(RE::TESObjectREFR* object)
			{
				if (!object || !object->Is3DLoaded()) return;

				RE::NiAVObject* objectRoot = object->Get3D(false);
				if (!objectRoot) return;

				std::function<void(RE::NiAVObject*)> processNode = [&](RE::NiAVObject* node) {
					if (!node || !node->AsNode()) return;
					auto niNode = node->AsNode();

					if (RE::NiPointer<RE::NiAVObject> root = RE::NiPointer<RE::NiAVObject>(node)) {
						if (auto rb = ModUtils::GetRigidBody(root.get())) {
							if (auto hkpRigidBody = static_cast<RE::hkpRigidBody*>(rb->referencedObject.get())) {
								hkpRigidBody->motion.deactivationRefPosition[0].quad = _mm_add_ps(hkpRigidBody->motion.deactivationRefPosition[0].quad, _mm_setr_ps(-1.0f, -1.0f, -1.0f, 0.0f));
								hkpRigidBody->motion.deactivationRefPosition[1].quad = _mm_add_ps(hkpRigidBody->motion.deactivationRefPosition[1].quad, _mm_setr_ps(1.0f, 1.0f, 1.0f, 0.0f));
								hkpRigidBody->motion.deactivationRefOrientation[0] -= 1;
								hkpRigidBody->motion.deactivationRefOrientation[1] += 1;
							}
						}
					}

					for (auto& child : niNode->GetChildren()) {
						if (child && child.get()) {
							processNode(child.get());
						}
					}
				};

				processNode(objectRoot);
			}

			static RE::TESObjectREFR* GetInertiaObject(RE::FormID objectFormID)
			{
				if (auto uiManager = RE::UI::GetSingleton(); !uiManager || uiManager->GameIsPaused()) return nullptr;

				RE::TESObjectREFR* object = RE::TESForm::LookupByID<RE::TESObjectREFR>(objectFormID);
				if (!object || object->IsDisabled() || !object->Is3DLoaded()) return nullptr;

				if (RE::Actor* actor = object->As<RE::Actor>()) {
					if (!actor->IsDead()) return nullptr;
				}

				return object;
			}

	
	};
};
