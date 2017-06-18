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
#include "structs.h"

#include <vector>

class ServerPlayer;

struct SlashEffectStruct
{
	ServerPlayer *from;
	ServerPlayer *to;

	Card *slash;
	std::vector<const Card *> jink;

	DamageStruct::Nature nature;
	bool drunk;
	int jinkNum;

	SlashEffectStruct();
};

class Slash : public BasicCard
{
	SGS_CARD(Slash, BasicCard)
public:
	Slash(Suit suit, int number);

	bool targetFilter(const std::vector<const Player *> &targets, const Player *toSelect, const Player *self) const override;
	void effect(GameLogic *logic, CardEffectStruct &cardEffect) const override;
	bool isAvailable(const Player *player) const override;

protected:
	DamageStruct::Nature m_nature;
};

class Jink : public BasicCard
{
	SGS_CARD(Jink, BasicCard)
public:
	Jink(Suit suit, int number);

	void onUse(GameLogic *logic, CardUseStruct &use) const override;
	void effect(GameLogic *, CardEffectStruct &effect) const override;
	bool isAvailable(const Player *) const override;

protected:
};

class Peach : public BasicCard
{
	SGS_CARD(Peach, BasicCard)
public:
	Peach(Suit suit, int number);

	void onUse(GameLogic *logic, CardUseStruct &use) const override;
	void effect(GameLogic *logic, CardEffectStruct &effect) const override;
	bool isAvailable(const Player *player) const override;

protected:
};
