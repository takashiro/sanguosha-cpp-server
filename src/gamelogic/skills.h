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

#pragma once

#include "Skill.h"

#include "CardFilter.h"
#include "EventHandler.h"

#include "structs.h"

#include <string>
#include <vector>

class TriggerSkill : public Skill, public EventHandler
{
public:
	TriggerSkill(const std::string &name);

	std::string name() const { return Skill::name(); }

	bool triggerable(ServerPlayer *owner) const override;
	bool onCost(GameLogic *logic, EventType event, ServerPlayer *target, void *data, ServerPlayer *invoker) const final override;

	virtual bool cost(GameLogic *logic, EventType event, ServerPlayer *target, void *data, ServerPlayer *invoker) const;

protected:
	void setFrequency(Frequency frequency);
};

class StatusSkill : public TriggerSkill
{
public:
	StatusSkill(const std::string &name);

	virtual void validate(ServerPlayer *target) const = 0;
	virtual void invalidate(ServerPlayer *target) const = 0;
	virtual bool isValid(ServerPlayer *target) const;

	bool effect(GameLogic *, EventType event, ServerPlayer *target, void *, ServerPlayer *) const final override;
};

class MasochismSkill : public TriggerSkill
{
public:
	MasochismSkill(const std::string &name);

	virtual int triggerable(GameLogic *logic, ServerPlayer *target, DamageStruct &damage) const = 0;
	EventList triggerable(GameLogic *logic, EventType, ServerPlayer *target, void *data, ServerPlayer *) const final override;

	virtual bool effect(GameLogic *logic, ServerPlayer *target, DamageStruct &damage) const = 0;
	bool effect(GameLogic *, EventType event, ServerPlayer *target, void *, ServerPlayer *) const final override;
};

class Card;

template<typename T>
static bool CheckAvailability(const Player *self)
{
	T *card = new T(T::NoSuit, 0);
	bool result = card->isAvailable(self);
	delete card;
	return result;
}

class ViewAsSkill : public Skill
{
public:
	ViewAsSkill(const std::string &name);

	//An empty pattern means it's the playing phase
	virtual bool isAvailable(const Player *self, const std::string &pattern) const;

	//Check if the card can be selected
	virtual bool viewFilter(const std::vector<const Card *> &selected, const Card *card, const Player *self, const std::string &pattern) const = 0;

	//Returns the generated new card
	virtual Card *viewAs(const std::vector<Card *> &cards, const Player *self) const = 0;

	//Check if selected cards are valid
	bool isValid(const std::vector<Card *> &cards, const Player *self, const std::string &pattern) const;
};

class OneCardViewAsSkill : public ViewAsSkill
{
public:
	OneCardViewAsSkill(const std::string &name);

	bool viewFilter(const std::vector<const Card *> &selected, const Card *card, const Player *self, const std::string &pattern) const final override;
	Card *viewAs(const std::vector<Card *> &cards, const Player *self) const final override;

	virtual bool viewFilter(const Card *card, const Player *self, const std::string &pattern) const = 0;
	virtual Card *viewAs(Card *card, const Player *self) const = 0;
};

class ProactiveSkill : public Skill
{
public:
	ProactiveSkill(const std::string &name);

	//An empty pattern means it's the playing phase
	virtual bool isAvailable(const Player *self, const std::string &pattern) const;

	//Check if cards are feasible
	virtual bool cardFeasible(const std::vector<const Card *> &selected, const Player *source) const;

	//Check if the card can be selected
	virtual bool cardFilter(const std::vector<const Card *> &selected, const Card *card, const Player *source, const std::string &pattern) const;

	//Check if selected cards are valid
	bool isValid(const std::vector<Card *> &cards, const Player *source, const std::string &pattern) const;

	//Check if the target players are feasible
	virtual bool playerFeasible(const std::vector<const Player *> &selected, const Player *source) const;

	//Check if a player can be selected
	virtual bool playerFilter(const std::vector<const Player *> &selected, const Player *toSelect, const Player *source) const;

	//Check if selected players are valid
	bool isValid(const std::vector<ServerPlayer *> &targets, ServerPlayer *source) const;
	bool isValid(const std::vector<const Player *> &targets, const Player *source) const;

	virtual bool cost(GameLogic *logic, ServerPlayer *from, const std::vector<ServerPlayer *> &to, const std::vector<Card *> &cards) const;
	virtual void effect(GameLogic *logic, ServerPlayer *from, const std::vector<ServerPlayer *> &to, const std::vector<Card *> &cards) const;
};

class CardModSkill : public TriggerSkill, public CardFilter
{
public:
	CardModSkill(const std::string &name);

	bool effect(GameLogic *, EventType event, ServerPlayer *target, void *data, ServerPlayer *invoker) const final override;
};
