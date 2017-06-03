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

#include "Card.h"

#include "Player.h"

#include <string.h>

KA_USING(Json)
KA_USING(JsonArray)
KA_USING(JsonObject)

Card::Card(Suit suit, int number)
	: m_id(0)
	, m_suit(suit)
	, m_number(number)
	, m_color(Color::None)
	, m_type(Type::Invalid)
	, m_subtype(0)
	, m_transferable(false)
	, m_canRecast(false)
	, m_useLimit(InfinityNum)
	, m_maxTargetNum(1)
	, m_minTargetNum(1)
	, m_distanceLimit(InfinityNum)
	, m_targetFixed(false)
	, m_skill(nullptr)
{
}

uint Card::effectiveId() const
{
	if (!isVirtual())
		return m_id;

	if (m_subcards.size() == 1)
		return m_subcards.front()->effectiveId();

	return 0;
}

Card::Suit Card::suit() const
{
	if (m_subcards.empty())
		return m_suit;
	else if (m_subcards.size() == 1)
		return m_subcards.front()->suit();
	else
		return Suit::None;
}

void Card::setSuitString(const char *suit)
{
	if (strcmp(suit, "spade") == 0)
		setSuit(Suit::Spade);
	else if (strcmp(suit, "heart") == 0)
		setSuit(Suit::Heart);
	else if (strcmp(suit, "club") == 0)
		setSuit(Suit::Club);
	else if (strcmp(suit, "diamond") == 0)
		setSuit(Suit::Diamond);
	else
		setSuit(Suit::None);
}

const char *Card::suitString() const
{
	if (m_suit == Suit::Spade)
		return "spade";
	else if (m_suit == Suit::Heart)
		return "heart";
	else if (m_suit == Suit::Club)
		return "club";
	else if (m_suit == Suit::Diamond)
		return "diamond";
	else
		return "no_suit";
}

int Card::number() const
{
	if (m_number > 0)
		return m_number;

	int number = 0;
	for (const Card *card : m_subcards)
		number += card->number();
	return number >= 13 ? 13 : number;
}

Card::Color Card::color() const
{
	if (m_suit == Suit::None)
		return m_color;
	return (m_suit == Suit::Spade || m_suit == Suit::Club) ? Color::Black : Color::Red;
}

void Card::setColorString(const char *color)
{
	if (strcmp(color, "black") == 0)
		setColor(Color::Black);
	else if (strcmp(color, "red") == 0)
		setColor(Color::Red);
	else
		setColor(Color::None);
}

const char *Card::colorString() const
{
	Color color = this->color();
	if (color == Color::Black)
		return "black";
	else if (color == Color::Red)
		return "red";
	else
		return "no_color";
}

const char *Card::typeString() const
{
	if (m_type == Type::Basic)
		return "basic";
	else if (m_type == Type::Trick)
		return "trick";
	else if (m_type == Type::Equip)
		return "equip";
	else
		return "skill";
}

void Card::addSubcard(Card *card)
{
	m_subcards.push_back(card);
}

Card *Card::realCard()
{
	if (id() > 0)
		return this;

	if (m_subcards.size() == 1)
		return m_subcards.front()->realCard();

	return nullptr;
}

const Card *Card::realCard() const
{
	if (id() > 0)
		return this;

	if (m_subcards.size() == 1)
		return m_subcards.front()->realCard();

	return nullptr;
}

std::vector<Card *> Card::realCards()
{
	std::vector<Card *> cards;
	if (id() > 0) {
		cards.push_back(this);
	} else {
		for (Card *subcard : m_subcards) {
			std::vector<Card *> subcards = subcard->realCards();
			for (Card *real_card : subcards) {
				cards.push_back(real_card);
			}
		}
	}
	return cards;
}

std::vector<const Card *> Card::realCards() const
{
	std::vector<const Card *> cards;
	if (id() > 0) {
		cards.push_back(this);
	} else {
		for (const Card *subcard : m_subcards) {
			std::vector<const Card *> subcards = subcard->realCards();
			for (const Card *real_card : subcards) {
				cards.push_back(real_card);
			}
		}
	}
	return cards;
}

int Card::useLimit() const
{
	return m_useLimit;
}

int Card::useLimit(const Player *source) const
{
	int limit = useLimit();
	std::vector<const CardFilter *> filters = source->getCardFilters();
	for (const CardFilter *filter : filters) {
		limit += filter->extraUseNum(this, source);
	}
	return limit;
}

int Card::maxTargetNum() const
{
	return m_maxTargetNum;
}

int Card::minTargetNum() const
{
	return m_minTargetNum;
}

int Card::distanceLimit() const
{
	return m_distanceLimit;
}

bool Card::targetFeasible(const std::vector<const Player *> &selected, const Player *source) const
{
	KA_UNUSED(source);
	return minTargetNum() <= selected.size() && selected.size() <= maxTargetNum();
}

bool Card::targetFilter(const std::vector<const Player *> &selected, const Player *toSelect, const Player *source) const
{
	int distanceLimit = this->distanceLimit();
	int maxTargetNum = this->maxTargetNum();
	bool isValid = toSelect->isAlive();

	std::vector<const CardFilter *> filters = toSelect->getCardFilters();
	for (const CardFilter *filter : filters) {
		isValid = filter->targetFilter(this, selected, toSelect, source);
		if (!isValid)
			return false;

		distanceLimit += filter->extraDistanceLimit(this, selected, toSelect, source);
		maxTargetNum += filter->extraMaxTargetNum(this, selected, toSelect, source);
	}

	return isValid && selected.size() < maxTargetNum && (source == nullptr || source->distanceTo(toSelect) <= distanceLimit);
}

bool Card::isAvailable(const Player *source) const
{
	int limit = useLimit(source);
	return source->cardHistory(this->name()) < limit;
}

bool Card::isValid(const std::vector<const Player *> &targets, const Player *source) const
{
	if (isTargetFixed())
		return targets.empty();

	std::vector<const Player *> selected;
	for (const Player *toSelect : targets) {
		if (targetFilter(selected, toSelect, source))
			selected.push_back(toSelect);
		else
			return false;
	}
	return targetFeasible(selected, source);
}

Card *Card::Find(const std::vector<Card *> &cards, uint id)
{
	for (Card *card : cards) {
		if (card->id() == id)
			return card;
	}
	return nullptr;
}

std::vector<Card *> Card::Find(const std::vector<Card *> &cards, const KA_IMPORT Json &data)
{
	std::vector<Card *> result;
	const JsonArray &targets = data.toArray();
	for(const Json &target : targets) {
		for (Card *card : cards) {
			if (card->id() == target.toUInt())
				result.push_back(card);
		}
	}
	return result;
}

KA_IMPORT JsonObject Card::toJson() const
{
	JsonObject data;
	data["name"] = name();
	data["suit"] = suitString();
	data["number"] = number();
	data["color"] = static_cast<int>(color());
	return data;
}
