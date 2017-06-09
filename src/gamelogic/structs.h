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

#include "common.h"

#include "CardAreaType.h"
#include "CardMoveDirection.h"
#include "PlayerPhase.h"
#include "SkillAreaType.h"
#include "CardPattern.h"

#include <vector>
#include <string>
#include <Json.h>

class Card;
class ServerPlayer;
class Skill;

struct CardsMoveStruct
{
	struct Area
	{
		CardAreaType type;
		CardMoveDirection direction;
		ServerPlayer *owner;
		uint ownerId;
		std::string name;

		Area();
		KA_IMPORT Json toJson() const;
	};

	Area from;
	Area to;
	std::vector<Card *> cards;
	bool isOpen;
	bool isLastHandCard;
	CardsMoveStruct *origin;

	CardsMoveStruct();
	~CardsMoveStruct();

	CardsMoveStruct &operator << (Card *card) { cards.push_back(card); return *this; }
	CardsMoveStruct &operator << (std::vector<Card *> &cards);

	bool isRelevant(const ServerPlayer *player) const;
	KA_IMPORT Json toJson(bool open = false) const;
};

struct PhaseChangeStruct
{
	PlayerPhase from;
	PlayerPhase to;
};

struct CardUseStruct
{
	ServerPlayer *from;
	std::vector<ServerPlayer *> to;
	Card *card;
	Card *target;
	std::vector<ServerPlayer *> nullifiedList;
	bool isNullified;
	bool addHistory;
	bool isHandCard;
	KA_IMPORT Json extra;

	CardUseStruct();
};

struct CardEffectStruct
{
	CardUseStruct &use;
	ServerPlayer *&from;
	ServerPlayer *to;

	CardEffectStruct(CardUseStruct &use);
	bool isNullified() const;
};

struct DamageStruct
{
	enum Nature
	{
		Normal,
		Fire,
		Thunder
	};

	ServerPlayer *from;
	ServerPlayer *to;
	Card *card;
	int damage;
	Nature nature;
	bool chain;
	bool transfer;
	bool byUser;
	std::string reason;
	std::string transferReason;
	bool prevented;

	DamageStruct();
};

struct RecoverStruct
{
	ServerPlayer *from;
	ServerPlayer *to;
	int recover;
	Card *card;

	RecoverStruct();
};

struct CardResponseStruct
{
	ServerPlayer *from;
	ServerPlayer *to;
	Card *card;
	Card *target;

	CardResponseStruct();
};

struct JudgeStruct
{
	CardPattern pattern;
	ServerPlayer *who;
	Card *card;
	bool matched;

	JudgeStruct(const std::string &pattern);
};

struct DeathStruct
{
	ServerPlayer *who;
	DamageStruct *damage;

	DeathStruct();
};

struct SkillStruct
{
	ServerPlayer *owner;
	const Skill *skill;
	SkillAreaType area;

	SkillStruct();
};
