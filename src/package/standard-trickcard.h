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

#include "cards.h"

class GameLogic;
class ServerPlayer;

class AmazingGrace : public GlobalEffect
{
	SGS_CARD(AmazingGrace, GlobalEffect)

public:
	AmazingGrace(Suit suit, int number);

	void use(GameLogic *logic, CardUseStruct &use) override;
	void effect(GameLogic *logic, CardEffectStruct &effect) override;

private:
	void clearRestCards(GameLogic *logic) const;
};

class GodSalvation : public GlobalEffect
{
	SGS_CARD(GodSalvation, GlobalEffect)

public:
	GodSalvation(Suit suit, int number);

	void effect(GameLogic *logic, CardEffectStruct &effect) override;
	bool isNullifiable(const CardEffectStruct &effect) const override;
};

class SavageAssault :public AreaOfEffect
{
	SGS_CARD(SavageAssault, AreaOfEffect)

public:
	SavageAssault(Suit suit, int number);

	void effect(GameLogic *logic, CardEffectStruct &effect) override;
};

class ArcheryAttack : public AreaOfEffect
{
	SGS_CARD(ArcheryAttack, AreaOfEffect)

public:
	ArcheryAttack(Suit suit, int number);

	void effect(GameLogic *logic, CardEffectStruct &effect) override;
};

class ExNihilo : public SingleTargetTrick
{
	SGS_CARD(ExNihilo, SingleTargetTrick)

public:
	ExNihilo(Suit suit, int number);

	void onUse(GameLogic *logic, CardUseStruct &use) override;
	void effect(GameLogic *, CardEffectStruct &effect) override;
};

class Duel : public SingleTargetTrick
{
	SGS_CARD(Duel, SingleTargetTrick)

public:
	Duel(Suit suit, int number);

	bool targetFilter(const std::vector<const Player *> &targets, const Player *toSelect, const Player *self) const override;
	void effect(GameLogic *logic, CardEffectStruct &effect) override;
};

class Snatch : public SingleTargetTrick
{
	SGS_CARD(Snatch, SingleTargetTrick)

public:
	Snatch(Suit suit, int number);

	bool targetFilter(const std::vector<const Player *> &targets, const Player *toSelect, const Player *self) const override;
	void effect(GameLogic *logic, CardEffectStruct &effect) override;
};

class Dismantlement : public SingleTargetTrick
{
	SGS_CARD(Dismantlement, SingleTargetTrick)

public:
	Dismantlement(Suit suit, int number);

	bool targetFilter(const std::vector<const Player *> &targets, const Player *toSelect, const Player *self) const override;
	void effect(GameLogic *logic, CardEffectStruct &effect) override;
};

class Collateral : public SingleTargetTrick
{
	SGS_CARD(Collateral, SingleTargetTrick)

public:
	Collateral(Suit suit, int number);
	bool isAvailable(const Player *player) const override;
	bool targetFeasible(const std::vector<const Player *> &targets, const Player *self) const override;
	bool targetFilter(const std::vector<const Player *> &targets, const Player *toSelect, const Player *self) const override;
	void onUse(GameLogic *logic, CardUseStruct &use) override;
	void effect(GameLogic *logic, CardEffectStruct &effect) override;

private:
	bool doCollateral(CardEffectStruct &effect) const;

	ServerPlayer *m_victim;
};

class Nullification : public SingleTargetTrick
{
	SGS_CARD(Nullification, SingleTargetTrick)

public:
	Nullification(Suit suit, int number);

	bool isAvailable(const Player *) const override;
	void effect(GameLogic *, CardEffectStruct &effect) override;
};

class Indulgence : public DelayedTrick
{
	SGS_CARD(Indulgence, DelayedTrick)

public:
	Indulgence(Suit suit, int number);

	void takeEffect(GameLogic *, CardEffectStruct &effect) override;
};

class Lightning : public MovableDelayedTrick
{
	SGS_CARD(Lightning, MovableDelayedTrick)

public:
	Lightning(Suit suit, int number);

	void takeEffect(GameLogic *logic, CardEffectStruct &effect) override;
};
