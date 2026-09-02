#include "Events.h"

namespace Events
{
	RE::BSEventNotifyControl ModEventSink::ProcessEvent(const RE::TESHitEvent* event, RE::BSTEventSource<RE::TESHitEvent>*)
	{
		if (!SettingsIni::bApplyOnHit) return continueEvent;

		RE::Actor* victim = event->target && event->target.get() ? event->target->As<RE::Actor>() : nullptr;
		if (!victim || !victim->IsDead()) return continueEvent;

		Events::MainEvent::ProceedActorDeath(victim);

		return continueEvent;
	}

	RE::BSEventNotifyControl ModEventSink::ProcessEvent(const RE::TESDeathEvent* event, RE::BSTEventSource<RE::TESDeathEvent>*)
	{
		if (event->dead) return continueEvent;

		RE::Actor* victim = event->actorDying && event->actorDying.get() ? event->actorDying->As<RE::Actor>() : nullptr;
		if (!victim) return continueEvent;

		Events::MainEvent::ProceedActorDeath(victim);

		return continueEvent;
	}
}
