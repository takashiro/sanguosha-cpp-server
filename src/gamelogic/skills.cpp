/***********************************************************************
H5Sanguosha Server
Copyright (C) 2016-2017  Kazuichi Takashiro

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

takashiro@qq.com
************************************************************************/

#include "skills.h"

#include "Card.h"
#include "General.h"
#include "ServerPlayer.h"

TriggerSkill::TriggerSkill(const std::string &name)
	: Skill(name)
{
	EventHandler::m_name = name;
	m_type = TriggerType;
	m_defaultPriority = 1;
}

bool TriggerSkill::triggerable(ServerPlayer *owner) const
{
	return owner != nullptr && owner->isAlive() && owner->hasSkill(this);
}

bool TriggerSkill::onCost(GameLogic *logic, EventType event, ServerPlayer *target, void *data, ServerPlayer *invoker) const
{
	const General *headGeneral = target->headGeneral();
	if (headGeneral->hasSkill(this)) {
		if (!target->hasShownHeadGeneral()) {
			target->setHeadGeneralShown(true);
			target->broadcastProperty("headGeneralId", target->headGeneral()->id());
		}
	} else {
		const General *deputyGeneral = target->deputyGeneral();
		if (deputyGeneral && deputyGeneral->hasSkill(this) && !target->hasShownDeputyGeneral()) {
			target->setDeputyGeneralShown(true);
			target->broadcastProperty("deputyGeneralId", target->deputyGeneral()->id());
		}
	}

	bool takeEffect = cost(logic, event, target, data, invoker);
	if (takeEffect) {
		if (invoker == target) {
			invoker->addSkillHistory(this);
		} else {
			std::vector<ServerPlayer *> targets;
			targets.push_back(target);
			invoker->addSkillHistory(this, targets);
		}
	}
	return takeEffect;
}

bool TriggerSkill::cost(GameLogic *logic, EventType event, ServerPlayer *target, void *data, ServerPlayer *invoker) const
{
	KA_UNUSED(logic, event, target, data, invoker);
	return true;
}

void TriggerSkill::setFrequency(Skill::Frequency frequency)
{
	m_frequency = frequency;
	m_compulsory = (m_frequency == Compulsory || m_frequency == Wake);
}

StatusSkill::StatusSkill(const std::string &name)
	: TriggerSkill(name)
{
	m_events.insert(SkillAdded);
	m_events.insert(SkillRemoved);
	setFrequency(Compulsory);
}

bool StatusSkill::isValid(ServerPlayer *target) const
{
	KA_UNUSED(target);
	return true;
}

bool StatusSkill::effect(GameLogic *, EventType event, ServerPlayer *target, void *, ServerPlayer *) const
{
	if (event == SkillAdded) {
		validate(target);
	} else if (event == SkillRemoved) {
		invalidate(target);
	} else {
		if (isValid(target))
			validate(target);
		else
			invalidate(target);
	}
	return false;
}

MasochismSkill::MasochismSkill(const std::string &name)
	: TriggerSkill(name)
{
	m_events.insert(AfterDamaged);
}

EventList MasochismSkill::triggerable(GameLogic *logic, EventType, ServerPlayer *target, void *data, ServerPlayer *) const
{
	if (!TriggerSkill::triggerable(target))
		return EventList();

	DamageStruct *damage = static_cast<DamageStruct *>(data);
	int times = triggerable(logic, target, *damage);
	if (times <= 0)
		return EventList();
	else if (times == 1)
		return Event(this, target);

	EventList events;
	for (int i = 0; i < times; i++)
		events.push_back(Event(this, target));
	return events;
}

bool MasochismSkill::effect(GameLogic *logic, EventType, ServerPlayer *target, void *data, ServerPlayer *) const
{
	DamageStruct *damage = static_cast<DamageStruct *>(data);
	return effect(logic, target, *damage);
}

ViewAsSkill::ViewAsSkill(const std::string &name)
	: Skill(name)
{
	m_type = ViewAsType;
}

bool ViewAsSkill::isAvailable(const Player *self, const std::string &pattern) const
{
	KA_UNUSED(self);
	return pattern.empty();
}

bool ViewAsSkill::isValid(const std::vector<Card *> &cards, const Player *self, const std::string &pattern) const
{
	std::vector<const Card *> selected;
	for (const Card *toSelect : cards) {
		if (viewFilter(selected, toSelect, self, pattern))
			selected.push_back(toSelect);
		else
			return false;
	}

	return true;
}

OneCardViewAsSkill::OneCardViewAsSkill(const std::string &name)
	: ViewAsSkill(name)
{
}

bool OneCardViewAsSkill::viewFilter(const std::vector<const Card *> &selected, const Card *card, const Player *self, const std::string &pattern) const
{
	return selected.empty() && viewFilter(card, self, pattern);
}

Card *OneCardViewAsSkill::viewAs(const std::vector<Card *> &cards, const Player *self) const
{
	if (cards.size() == 1)
		return viewAs(cards.front(), self);
	return nullptr;
}

ProactiveSkill::ProactiveSkill(const std::string &name)
	: Skill(name)
{
	m_type = ProactiveType;
}

bool ProactiveSkill::isAvailable(const Player *self, const std::string &pattern) const
{
	KA_UNUSED(self);
	return pattern.empty();
}

bool ProactiveSkill::cardFeasible(const std::vector<const Card *> &selected, const Player *source) const
{
	KA_UNUSED(selected, source);
	return true;
}

bool ProactiveSkill::cardFilter(const std::vector<const Card *> &selected, const Card *card, const Player *source, const std::string &pattern) const
{
	KA_UNUSED(selected, card, source, pattern);
	return false;
}

bool ProactiveSkill::isValid(const std::vector<Card *> &cards, const Player *source, const std::string &pattern) const
{
	std::vector<const Card *> selected;
	for (const Card *toSelect : cards) {
		if (cardFilter(selected, toSelect, source, pattern))
			selected.push_back(toSelect);
		else
			return false;
	}
	return cardFeasible(selected, source);
}

bool ProactiveSkill::playerFeasible(const std::vector<const Player *> &selected, const Player *source) const
{
	KA_UNUSED(selected, source);
	return true;
}

bool ProactiveSkill::playerFilter(const std::vector<const Player *> &selected, const Player *toSelect, const Player *source) const
{
	KA_UNUSED(selected, toSelect, source);
	return false;
}

bool ProactiveSkill::isValid(const std::vector<ServerPlayer *> &targets, ServerPlayer *source) const
{
	std::vector<const Player *> selected;
	for (ServerPlayer *target : targets)
		selected.push_back(target);
	return isValid(selected, source);
}

bool ProactiveSkill::isValid(const std::vector<const Player *> &targets, const Player *source) const
{
	std::vector<const Player *> selected;
	for (const Player *toSelect : targets) {
		if (playerFilter(selected, toSelect, source))
			selected.push_back(toSelect);
		else
			return false;
	}

	return playerFeasible(targets, source);
}

bool ProactiveSkill::cost(GameLogic *, ServerPlayer *, const std::vector<ServerPlayer *> &, const std::vector<Card *> &) const
{
	return true;
}

void ProactiveSkill::effect(GameLogic *, ServerPlayer *, const std::vector<ServerPlayer *> &, const std::vector<Card *> &) const
{
}

CardModSkill::CardModSkill(const std::string &name)
	: TriggerSkill(name)
{
	m_events.insert(SkillAdded);
	m_events.insert(SkillRemoved);
}

bool CardModSkill::effect(GameLogic *, EventType event, ServerPlayer *target, void *data, ServerPlayer *invoker) const
{
	if (event == SkillAdded) {
		target->addCardFilter(this);
	} else if (event == SkillRemoved) {
		target->removeCardFilter(this);
	}
	return false;
}
