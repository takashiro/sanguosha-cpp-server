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

#include "standard-basiccard.h"
#include "Package.h"

class FireSlash : public Slash
{
	SGS_CARD(FireSlash, Slash)

public:
	FireSlash(Suit suit, int number);
};

class ThunderSlash : public Slash
{
	SGS_CARD(ThunderSlash, Slash)

public:
	ThunderSlash(Suit suit, int number);
};

class Analeptic : public BasicCard
{
	SGS_CARD(Analeptic, BasicCard)

public:
	Analeptic(Suit suit, int number);

	void onUse(GameLogic *logic, CardUseStruct &use) const override;
	void effect(GameLogic *logic, CardEffectStruct &effect) const override;
};

class Fan : public Weapon
{
	SGS_CARD(Fan, Weapon)

public:
	Fan(Suit suit = Suit::Diamond, int number = 1);
};

class GudingBlade : public Weapon
{
	SGS_CARD(GudingBlade, Weapon)

public:
	GudingBlade(Suit = Suit::Spade, int number = 1);
};

class Vine : public Armor
{
	SGS_CARD(Vine, Armor)

public:
	Vine(Suit suit = Suit::Club, int number = 2);
};

class SilverLion : public Armor
{
	SGS_CARD(SilverLion, Armor)

public:
	SilverLion(Suit suit = Suit::Club, int number = 1);
};

class SupplyShortage : public DelayedTrick
{
	SGS_CARD(SupplyShortage, DelayedTrick)

public:
	SupplyShortage(Suit suit, int number);

	void takeEffect(GameLogic *, CardEffectStruct &effect) const override;
};

class IronChain : public TrickCard
{
	SGS_CARD(IronChain, TrickCard)

public:
	IronChain(Suit suit, int number);

	void effect(GameLogic *, CardEffectStruct &effect) const override;
};

class FireAttack : public SingleTargetTrick
{
	SGS_CARD(FireAttack, SingleTargetTrick)

public:
	FireAttack(Suit suit, int number);

	bool targetFilter(const std::vector<const Player *> &targets, const Player *toSelect, const Player *self) const override;
	void effect(GameLogic *logic, CardEffectStruct &effect) const override;
};

class ManeuveringPackage : public Package
{
public:
	ManeuveringPackage();
};
