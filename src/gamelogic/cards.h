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

#include "Card.h"

class DefaultCard : public Card
{
public:
	DefaultCard(Suit suit, int number);

	void onUse(GameLogic *logic, CardUseStruct &use) const override;
	void use(GameLogic *logic, CardUseStruct &use) const override;
	void complete(GameLogic *logic) const override;
};

class BasicCard : public DefaultCard
{
	SGS_CARD_HIERACHY(BasicCard, Card)
public:
	BasicCard(Suit suit, int number);
};

class TrickCard : public DefaultCard
{
	SGS_CARD_HIERACHY(TrickCard, Card)
public:
	enum SubType
	{
		//Immediate Types
		GlobalEffectType,
		AreaOfEffectType,
		SingleTargetType,
		DamageSpreadType,

		//Delayed Types
		DelayedType
	};

	TrickCard(Suit suit, int number);

	void onEffect(GameLogic *logic, CardEffectStruct &effect) const override;
	virtual bool isNullifiable(const CardEffectStruct &effect) const;
};

class Skill;

class EquipCard : public DefaultCard
{
	SGS_CARD_HIERACHY(EquipCard, Card)
public:
	enum SubType
	{
		WeaponType,
		ArmorType,
		DefensiveHorseType,
		OffensiveHorseType,
		TreasureType
	};

	EquipCard(Suit suit, int number);

	void onUse(GameLogic *logic, CardUseStruct &use) const override;
	void use(GameLogic *logic, CardUseStruct &use) const override;
	void complete(GameLogic *) const override;

	Skill *skill() const { return m_skill; }

protected:
	Skill *m_skill;
};

class GlobalEffect : public TrickCard
{
	SGS_CARD_HIERACHY(GlobalEffect, TrickCard)
public:
	GlobalEffect(Card::Suit suit, int number);

	void onUse(GameLogic *logic, CardUseStruct &use) const override;
};

class AreaOfEffect : public TrickCard
{
	SGS_CARD_HIERACHY(AreaOfEffect, TrickCard)
public:
	AreaOfEffect(Suit suit, int number);

	void onUse(GameLogic *logic, CardUseStruct &use) const override;
};

class SingleTargetTrick : public TrickCard
{
	SGS_CARD_HIERACHY(SingleTargetTrick, TrickCard)
public:
	SingleTargetTrick(Suit suit, int number);
};

class DelayedTrick : public TrickCard
{
	SGS_CARD_HIERACHY(DelayedTrick, TrickCard)
public:
	DelayedTrick(Suit suit, int number);

	bool targetFeasible(const std::vector<const Player *> &selected, const Player *) const override;
	bool targetFilter(const std::vector<const Player *> &selected, const Player *toSelect, const Player *source) const override;
	void onUse(GameLogic *logic, CardUseStruct &use) const override;
	void use(GameLogic *logic, CardUseStruct &use) const override;
	void onEffect(GameLogic *logic, CardEffectStruct &effect) const override;
	void effect(GameLogic *logic, CardEffectStruct &effect) const override;

	virtual void takeEffect(GameLogic *logic, CardEffectStruct &effect) const = 0;

protected:
	std::string m_judgePattern;
};

class MovableDelayedTrick : public DelayedTrick
{
	SGS_CARD_HIERACHY(MovableDelayedTrick, DelayedTrick)
public:
	MovableDelayedTrick(Suit suit, int number);

	void onUse(GameLogic *logic, CardUseStruct &use) const override;
	void effect(GameLogic *logic, CardEffectStruct &effect) const override;
	void complete(GameLogic *logic) const override;
	bool isAvailable(const Player *player) const override;
};

class Weapon : public EquipCard
{
	SGS_CARD_HIERACHY(Weapon, EquipCard)
public:
	Weapon(Suit suit, int number);

	int attackRange() const;

protected:
	int m_attackRange;
};

class Armor : public EquipCard
{
	SGS_CARD_HIERACHY(Armor, EquipCard)
public:
	Armor(Suit suit, int number);
};

class Horse : public EquipCard
{
	SGS_CARD_HIERACHY(Horse, EquipCard)
public:
	Horse(Suit suit, int number);
};

class OffensiveHorse : public Horse
{
	SGS_CARD(OffensiveHorse, Horse)
public:
	OffensiveHorse(Suit suit, int number);

	int extraOutDistance() const;

protected:
	int m_extraOutDistance;
};

class DefensiveHorse : public Horse
{
	SGS_CARD(DefensiveHorse, Horse)
public:
	DefensiveHorse(Suit suit, int number);

	int extraInDistance() const;

protected:
	int m_extraInDistance;
};

class Treasure : public EquipCard
{
	SGS_CARD(Treasure, EquipCard)
public:
	Treasure(Suit suit, int number);
};
