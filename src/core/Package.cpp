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

#include "Package.h"

#include "Card.h"
#include "General.h"

namespace{

template<typename T> uint GenerateId()
{
	static uint id = 0;
	id++;
	return id;
}

}

Package::Package(const std::string &name)
	: m_name(name)
{
}

Package::~Package()
{
	for (Card *card : m_cards)
		delete card;

	for (General *general : m_generals)
		delete general;
}

bool Package::isAvailable(const GameMode *) const
{
	return true;
}

std::vector<const General *> Package::generals(bool includeHidden) const
{
	std::vector<const General *> generals;
	generals.reserve(m_generals.size());
	for (const General *general : m_generals) {
		if (includeHidden || !general->isHidden())
			generals.push_back(general);
	}
	return generals;
}

std::vector<const Card *> Package::cards() const
{
	std::vector<const Card *> cards;
	cards.reserve(m_cards.size());
	for (const Card *card : m_cards)
		cards.push_back(card);
	return cards;
}

void Package::addGeneral(General *general)
{
	general->m_id = GenerateId<General>();
	m_generals.push_back(general);
}

void Package::addGenerals(const std::vector<General *> &generals)
{
	for (General *general : generals) {
		addGeneral(general);
	}
}

const General *Package::getGeneral(const std::string &name) const
{
	for (General *general : m_generals) {
		if (general->name() == name)
			return general;
	}
	return nullptr;
}

void Package::addCard(Card *card)
{
	card->m_id = GenerateId<Card>();
	m_cards.push_back(card);
}

void Package::addCards(const std::vector<Card *> &cards)
{
	for (Card *card : cards) {
		addCard(card);
	}
}
