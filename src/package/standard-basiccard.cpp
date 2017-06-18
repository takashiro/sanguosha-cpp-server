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

#include "standard-basiccard.h"
#include "standardpackage.h"

#include "GameLogic.h"
#include "ServerPlayer.h"

KA_USING_NAMESPACE

SlashEffectStruct::SlashEffectStruct()
	: from(nullptr)
	, to(nullptr)
	, slash(nullptr)
	, nature(DamageStruct::Normal)
	, drunk(false)
	, jinkNum(1)
{
}

Slash::Slash(Card::Suit suit, int number)
	: BasicCard(suit, number)
	, m_nature(DamageStruct::Normal)
{
	setName("slash");
	m_useLimit = 1;
}

bool Slash::targetFilter(const std::vector<const Player *> &targets, const Player *toSelect, const Player *self) const
{
	return toSelect->inAttackRangeOf(self) && BasicCard::targetFilter(targets, toSelect, self);
}

void Slash::effect(GameLogic *logic, CardEffectStruct &effect) const
{
	DamageStruct damage;
	damage.from = effect.from;
	damage.to = effect.to;
	damage.nature = m_nature;
	damage.card = this;
	logic->damage(damage);
}

bool Slash::isAvailable(const Player *player) const
{
	//@to-do: Find a better solution for slash use limit
	int times = player->cardHistory("slash");
	times += player->cardHistory("thunder_slash");
	times += player->cardHistory("fire_slash");
	return times < useLimit(player) && BasicCard::isAvailable(player);
}

Jink::Jink(Card::Suit suit, int number)
	: BasicCard(suit, number)
{
	setName("jink");
	m_targetFixed = true;
}

void Jink::onUse(GameLogic *logic, CardUseStruct &use) const
{
	SlashEffectStruct *effect = static_cast<SlashEffectStruct *>(use.extra);
	if (effect)
		use.target = effect->slash;
	BasicCard::onUse(logic, use);
}

void Jink::effect(GameLogic *, CardEffectStruct &effect) const
{
	SlashEffectStruct *slash_effect = static_cast<SlashEffectStruct *>(effect.use.extra);
	if (slash_effect) {
		slash_effect->jink.push_back(this);
	}
}

bool Jink::isAvailable(const Player *) const
{
	return false;
}

Peach::Peach(Card::Suit suit, int number)
	: BasicCard(suit, number)
{
	setName("peach");
	m_targetFixed = true;
}

void Peach::onUse(GameLogic *logic, CardUseStruct &use) const
{
	if (use.to.empty())
		use.to.push_back(use.from);
	BasicCard::onUse(logic, use);
}

void Peach::effect(GameLogic *logic, CardEffectStruct &effect) const
{
	RecoverStruct recover;
	recover.card = this;
	recover.from = effect.from;
	recover.to = effect.to;
	logic->recover(recover);
}

bool Peach::isAvailable(const Player *player) const
{
	return player->isWounded() && BasicCard::isAvailable(player);
}

void StandardPackage::addBasicCards()
{
	std::vector<Card *> cards = {
		new Slash(CardSuit::Spade, 7),
		new Slash(CardSuit::Spade, 8),
		new Slash(CardSuit::Spade, 8),
		new Slash(CardSuit::Spade, 9),
		new Slash(CardSuit::Spade, 9),
		new Slash(CardSuit::Spade, 10),
		new Slash(CardSuit::Spade, 10),

		new Slash(CardSuit::Club, 2),
		new Slash(CardSuit::Club, 3),
		new Slash(CardSuit::Club, 4),
		new Slash(CardSuit::Club, 5),
		new Slash(CardSuit::Club, 6),
		new Slash(CardSuit::Club, 7),
		new Slash(CardSuit::Club, 8),
		new Slash(CardSuit::Club, 8),
		new Slash(CardSuit::Club, 9),
		new Slash(CardSuit::Club, 9),
		new Slash(CardSuit::Club, 10),
		new Slash(CardSuit::Club, 10),
		new Slash(CardSuit::Club, 11),
		new Slash(CardSuit::Club, 11),

		new Slash(CardSuit::Heart, 10),
		new Slash(CardSuit::Heart, 10),
		new Slash(CardSuit::Heart, 11),

		new Slash(CardSuit::Diamond, 6),
		new Slash(CardSuit::Diamond, 7),
		new Slash(CardSuit::Diamond, 8),
		new Slash(CardSuit::Diamond, 9),
		new Slash(CardSuit::Diamond, 10),
		new Slash(CardSuit::Diamond, 13),

		new Jink(CardSuit::Heart, 2),
		new Jink(CardSuit::Heart, 2),
		new Jink(CardSuit::Heart, 13),

		new Jink(CardSuit::Diamond, 2),
		new Jink(CardSuit::Diamond, 2),
		new Jink(CardSuit::Diamond, 3),
		new Jink(CardSuit::Diamond, 4),
		new Jink(CardSuit::Diamond, 5),
		new Jink(CardSuit::Diamond, 6),
		new Jink(CardSuit::Diamond, 7),
		new Jink(CardSuit::Diamond, 8),
		new Jink(CardSuit::Diamond, 9),
		new Jink(CardSuit::Diamond, 10),
		new Jink(CardSuit::Diamond, 11),
		new Jink(CardSuit::Diamond, 11),

		new Peach(CardSuit::Heart, 3),
		new Peach(CardSuit::Heart, 4),
		new Peach(CardSuit::Heart, 6),
		new Peach(CardSuit::Heart, 7),
		new Peach(CardSuit::Heart, 8),
		new Peach(CardSuit::Heart, 9),
		new Peach(CardSuit::Heart, 12),

		new Peach(CardSuit::Diamond, 12)
	};

	addCards(cards);
}
