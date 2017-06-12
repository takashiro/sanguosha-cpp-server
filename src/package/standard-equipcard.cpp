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

#include "standardpackage.h"
#include "standard-equipcard.h"

Crossbow::Crossbow(Card::Suit suit, int number)
	: Weapon(suit, number)
{
	setName("crossbow");
	m_attackRange = 1;
}

DoubleSword::DoubleSword(Card::Suit suit, int number)
	: Weapon(suit, number)
{
	setName("double_sword");
	m_attackRange = 2;
}

QinggangSword::QinggangSword(Card::Suit suit, int number)
	: Weapon(suit, number)
{
	setName("qinggang_sword");
	m_attackRange = 2;
}

IceSword::IceSword(Card::Suit suit, int number)
	: Weapon(suit, number)
{
	setName("ice_sword");
	m_attackRange = 2;
}

Spear::Spear(Card::Suit suit, int number)
	: Weapon(suit, number)
{
	setName("spear");
	m_attackRange = 3;
}

Axe::Axe(Card::Suit suit, int number)
	: Weapon(suit, number)
{
	setName("axe");
	m_attackRange = 3;
}

Triblade::Triblade(Card::Suit suit, int number)
	: Weapon(suit, number)
{
	setName("triblade");
	m_attackRange = 3;
}

KylinBow::KylinBow(Card::Suit suit, int number)
	: Weapon(suit, number)
{
	setName("kylin_bow");
	m_attackRange = 5;
}

EightDiagram::EightDiagram(Card::Suit suit, int number)
	: Armor(suit, number)
{
	setName("eight_diagram");
}

RenwangShield::RenwangShield(Card::Suit suit, int number)
	: Armor(suit, number)
{
	setName("renwang_shield");
}

void StandardPackage::addEquipCards()
{
	std::vector<Card *> cards = {
		new Crossbow(CardSuit::Club, 1),
		new Crossbow(CardSuit::Diamond, 1),
		new DoubleSword,
		new QinggangSword,
		//@to-do: add Blade
		new Spear,
		 new Axe,
		//@to-do: add Halberd
		new KylinBow,

		new EightDiagram(CardSuit::Spade, 2),
		new EightDiagram(CardSuit::Club, 2),
	};
	addCards(cards);

	Card *jueying = new DefensiveHorse(CardSuit::Spade, 5);
	jueying->setName("jueying");
	addCard(jueying);

	Card *dilu = new DefensiveHorse(CardSuit::Club, 5);
	dilu->setName("dilu");
	addCard(dilu);

	Card *zhuahuangfeidian = new DefensiveHorse(CardSuit::Heart, 13);
	zhuahuangfeidian->setName("zhuahuangfeidian");
	addCard(zhuahuangfeidian);

	Card *chitu = new OffensiveHorse(CardSuit::Heart, 5);
	chitu->setName("chitu");
	addCard(chitu);

	Card *dayuan = new OffensiveHorse(CardSuit::Spade, 13);
	dayuan->setName("dayuan");
	addCard(dayuan);

	Card *zixing = new OffensiveHorse(CardSuit::Diamond, 13);
	zixing->setName("zixing");
	addCard(zixing);
}
