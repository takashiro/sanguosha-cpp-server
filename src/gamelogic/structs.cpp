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

#include "structs.h"

#include "Card.h"

KA_USING(Json)
KA_USING(JsonObject)
KA_USING(JsonArray)

CardsMoveStruct::Area::Area()
	: type(CardAreaType::Unknown)
	, direction(CardMoveDirection::Undefined)
	, owner(nullptr)
{
}

Json CardsMoveStruct::Area::toJson() const
{
	JsonObject data;
	data["type"] = static_cast<int>(type);
	data["direction"] = static_cast<int>(direction);
	data["ownerId"] = ownerId;
	data["name"] = name;
	return data;
}

CardsMoveStruct::CardsMoveStruct()
	: isOpen(false)
	, isLastHandCard(false)
	, origin(nullptr)
{
}

CardsMoveStruct::~CardsMoveStruct()
{
	if (origin)
		delete origin;
}

CardsMoveStruct &CardsMoveStruct::operator << (const std::vector<const Card *> &cards)
{
	for (const Card *card : cards) {
		this->cards.push_back(card);
	}
	return *this;
}

CardsMoveStruct &CardsMoveStruct::operator << (const std::deque<const Card *> &cards)
{
	for (const Card *card : cards) {
		this->cards.push_back(card);
	}
	return *this;
}

bool CardsMoveStruct::isRelevant(const ServerPlayer *player) const
{
	return player == nullptr || player == from.owner || (player == to.owner && to.type != CardAreaType::Special);
}

Json CardsMoveStruct::toJson(bool open) const
{
	JsonObject data;
	data["from"] = from.toJson();
	data["to"] = to.toJson();

	if (isOpen || open) {
		JsonArray card_data;
		for (const Card *card : cards)
			card_data.push_back(card->id());
		data["cards"] = card_data;
	} else {
		data["cards"] = cards.size();
	}

	data["isOpen"] = isOpen;
	data["isLastHandCard"] = isLastHandCard;
	return data;
}

CardUseStruct::CardUseStruct()
	: from(nullptr)
	, card(nullptr)
	, target(nullptr)
	, isNullified(false)
	, addHistory(true)
	, isHandCard(true)
{
}

CardEffectStruct::CardEffectStruct(CardUseStruct &use)
	: use(use)
	, from(use.from)
	, to(nullptr)
{
}

bool CardEffectStruct::isNullified() const
{
	if (use.isNullified) {
		return true;
	}

	if (to) {
		for (ServerPlayer *player : use.nullifiedList) {
			if (player == to) {
				return true;
			}
		}
	}

	return false;
}

DamageStruct::DamageStruct()
	: from(nullptr)
	, to(nullptr)
	, card(nullptr)
	, damage(1)
	, nature(Normal)
	, chain(false)
	, transfer(false)
	, byUser(true)
	, prevented(false)
{
}

RecoverStruct::RecoverStruct()
	: from(nullptr)
	, to(nullptr)
	, recover(1)
	, card(nullptr)
{
}

CardResponseStruct::CardResponseStruct()
	: from(nullptr)
	, to(nullptr)
	, card(nullptr)
	, target(nullptr)
{
}

JudgeStruct::JudgeStruct(const std::string &pattern)
	: pattern(pattern)
	, who(nullptr)
	, card(nullptr)
	, matched(false)
{
}

DeathStruct::DeathStruct()
	: who(nullptr)
	, damage(nullptr)
{
}

SkillStruct::SkillStruct()
	: owner(nullptr)
	, skill(nullptr)
	, area(SkillAreaType::Unknown)
{
}
