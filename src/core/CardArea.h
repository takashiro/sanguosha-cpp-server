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

#include <Json.h>

#include <deque>
#include <set>
#include <functional>

class Player;
class Card;

class CardArea
{
public:
	using Type = CardAreaType;
	using Direction = CardMoveDirection;
	using ChangeSignal = std::function<void()>;

	CardArea(Type type, Player *owner = nullptr, const std::string &name = std::string());
	Type type() const { return m_type; }
	Player *owner() const { return m_owner; }
	std::string name() const { return m_name; }

	void setSignal(ChangeSignal signal) { m_changeSignal = signal; }

	bool keepVirtualCard() const { return m_keepVirtualCard; }
	void setKeepVirtualCard(bool keep) { m_keepVirtualCard = keep; }

	bool add(const Card *card, Direction direction = Direction::Undefined);
	bool add(const std::vector<const Card *> &cards, Direction direction = Direction::Undefined);
	bool remove(const Card *card);
	bool remove(const std::vector<const Card *> &cards);
	void clear() { m_cards.clear(); }

	const Card *findCard(uint id) const;
	const Card *rand() const;

	const Card *first() const { return m_cards.front(); }
	const Card *takeFirst();

	const Card *last() const { return m_cards.back(); }
	const Card *takeLast();

	std::vector<const Card *> first(int n) const;
	std::vector<const Card *> takeFirst(int n);

	std::vector<const Card *> last(int n) const;
	std::vector<const Card *> takeLast(int n);

	bool contains(const Card *card) const;
	bool contains(uint id) const;
	bool contains(const char *name) const;

	void addVirtualCard(const std::string &name) { m_virtualCards.insert(name); }
	void removeVirtualCard(const std::string &name) { m_virtualCards.erase(name); }

	std::deque<const Card *> &cards() { return m_cards; }
	const std::deque<const Card *> &cards() const { return m_cards; }

	int size() const { return static_cast<int>(m_cards.size()); }

	KA_IMPORT Json toJson() const;

private:
	Type m_type;
	Player *m_owner;
	std::string m_name;
	std::deque<const Card *> m_cards;
	ChangeSignal m_changeSignal;
	bool m_keepVirtualCard;
	std::set<std::string> m_virtualCards;
};
