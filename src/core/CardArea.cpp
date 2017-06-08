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

#include "CardArea.h"

#include "Card.h"
#include "Player.h"

#include <algorithm>
#include <random>
#include <vector>

CardArea::CardArea(CardArea::Type type, Player *owner, const std::string &name)
    : m_type(type)
    , m_owner(owner)
    , m_name(name)
    , m_keepVirtualCard(false)
{
}

bool CardArea::add(Card *card, Direction direction) {
    if (card) {
		for (Card *current : m_cards) {
			if (current == card) {
				return false;
			}
		}
        if (!keepVirtualCard() && card->isVirtual()) {
            delete card;
            return false;
        }
    }
    if (direction == Top)
        m_cards.push_front(card);
    else
        m_cards.push_back(card);
    if (m_changeSignal)
        m_changeSignal();
    return true;
}

bool CardArea::add(const std::vector<Card *> &cards, Direction direction)
{
    int num = size();
    if (direction == Top) {
		size_t pos = 0;
        for (size_t i = 0; i < cards.size(); i++) {
            Card *card = cards.at(i);
            if (card) {
                if (std::find(m_cards.begin(), m_cards.end(), card) != m_cards.end())
                    continue;
                if (!keepVirtualCard() && card->isVirtual()) {
                    delete card;
                    continue;
                }
            }
            m_cards.insert(m_cards.begin() + pos, card);
			pos++;
        }
    } else {
        for (Card *card : cards) {
            if (card) {
				bool contains = false;
				for (Card *current : m_cards) {
					if (current == card) {
						contains = true;
						break;
					}
				}
                if (contains)
                    continue;
                if (!keepVirtualCard() && card->isVirtual()) {
                    delete card;
                    continue;
                }
            }
            m_cards.push_back(card);
        }
    }

    if (m_changeSignal && num != size())
            m_changeSignal();

    return num + cards.size() == size();
}

bool CardArea::remove(Card *card)
{
	auto i = std::find(m_cards.begin(), m_cards.end(), card);
	if (i != m_cards.end()) {
		m_cards.erase(i);
		if (m_changeSignal) {
			m_changeSignal();
		}
		return true;
	}
    return false;
}

bool CardArea::remove(const std::vector<Card *> &cards)
{
    int num = size();
	for (Card *card : cards) {
		auto i = std::find(m_cards.begin(), m_cards.end(), card);
		if (i != m_cards.end()) {
			m_cards.erase(i);
		}
	}

    if (m_changeSignal && num != size())
            m_changeSignal();

    return num - cards.size() == size();
}

Card *CardArea::findCard(uint id) const
{
    for (Card *card : m_cards) {
        if (card && card->id() == id)
            return card;
    }
    return nullptr;
}

Card *CardArea::rand() const
{
    if (m_cards.empty())
        return nullptr;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, size() - 1);
    return m_cards.at(dis(gen));
}

Card *CardArea::takeFirst()
{
	Card *first = m_cards.front();
	m_cards.pop_front();
	return first;
}

Card *CardArea::takeLast()
{
	Card *last = m_cards.back();
	m_cards.pop_back();
	return last;
}

std::vector<Card *> CardArea::first(int n) const
{
	n = std::min(n, static_cast<int>(m_cards.size()));
	std::vector<Card *> cards;
	cards.resize(n);
	std::copy(m_cards.begin(), m_cards.begin() + n, cards.begin());
	return cards;
}

std::vector<Card *> CardArea::takeFirst(int n)
{
	std::vector<Card *> cards;
	cards.resize(n);
	for (int i = 0; i < n; i++) {
		cards[i] = takeFirst();
	}
    return cards;
}

std::vector<Card *> CardArea::last(int n) const
{
	n = std::min(n, static_cast<int>(m_cards.size()));
	std::vector<Card *> cards;
	cards.resize(n);
	auto from = m_cards.begin() + (m_cards.size() - n);
	auto to = from + n;
	std::copy(from, to, cards.begin());
	return cards;
}

std::vector<Card *> CardArea::takeLast(int n)
{
	std::vector<Card *> cards;
	cards.resize(n);
	for (int i = n - 1; i >= 0; i--) {
		cards[i] = takeLast();
	}
    return cards;
}

bool CardArea::contains(const Card *card) const
{
	for (Card *current : m_cards) {
		if (current == card)
			return true;
	}
    return false;
}

bool CardArea::contains(uint id) const
{
	for (const Card *card : m_cards) {
		if (card && card->id() == id)
			return true;
	}
    return false;
}

KA_IMPORT Json CardArea::toJson() const
{
    KA_IMPORT Json data;
    data["ownerId"] = m_owner ? m_owner->id() : 0;
    data["type"] = static_cast<int>(type());
    data["name"] = name();
    return data;
}
