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

#include "Engine.h"
#include "GameMode.h"
#include "General.h"

#include "hegstandardpackage.h"
#include "standard-basiccard.h"
#include "standard-equipcard.h"
#include "standard-trickcard.h"
#include "maneuveringpackage.h"

SixSwords::SixSwords(Suit suit, int number)
	: Weapon(suit, number)
{
}

HegStandardPackage::HegStandardPackage()
	: Package("hegstandard")
{
	std::vector<Card *> cards = {
		//Basic Cards
		new Slash(CardSuit::Spade, 5),
		new Slash(CardSuit::Spade, 7),
		new Slash(CardSuit::Spade, 8),
		new Slash(CardSuit::Spade, 8),
		new Slash(CardSuit::Spade, 9),
		new Slash(CardSuit::Spade, 10),
		new Slash(CardSuit::Spade, 11),

		new Slash(CardSuit::Club, 2),
		new Slash(CardSuit::Club, 3),
		new Slash(CardSuit::Club, 4),
		new Slash(CardSuit::Club, 5),
		new Slash(CardSuit::Club, 8),
		new Slash(CardSuit::Club, 9),
		new Slash(CardSuit::Club, 10),
		new Slash(CardSuit::Club, 11),
		new Slash(CardSuit::Club, 11),

		new Slash(CardSuit::Heart, 10),
		new Slash(CardSuit::Heart, 12),

		new Slash(CardSuit::Diamond, 10),
		new Slash(CardSuit::Diamond, 11),
		new Slash(CardSuit::Diamond, 12),

		new FireSlash(CardSuit::Heart, 4),

		new FireSlash(CardSuit::Diamond, 4),
		new FireSlash(CardSuit::Diamond, 5),

		new ThunderSlash(CardSuit::Spade, 6),
		new ThunderSlash(CardSuit::Spade, 7),

		new ThunderSlash(CardSuit::Club, 6),
		new ThunderSlash(CardSuit::Club, 7),
		new ThunderSlash(CardSuit::Club, 8),

		new Jink(CardSuit::Heart, 2),
		new Jink(CardSuit::Heart, 11),
		new Jink(CardSuit::Heart, 13),

		new Jink(CardSuit::Diamond, 2),
		new Jink(CardSuit::Diamond, 3),
		new Jink(CardSuit::Diamond, 6),
		new Jink(CardSuit::Diamond, 7),
		new Jink(CardSuit::Diamond, 7),
		new Jink(CardSuit::Diamond, 8),
		new Jink(CardSuit::Diamond, 8),
		new Jink(CardSuit::Diamond, 9),
		new Jink(CardSuit::Diamond, 10),
		new Jink(CardSuit::Diamond, 11),
		new Jink(CardSuit::Diamond, 13),

		new Peach(CardSuit::Heart, 4),
		new Peach(CardSuit::Heart, 6),
		new Peach(CardSuit::Heart, 7),
		new Peach(CardSuit::Heart, 8),
		new Peach(CardSuit::Heart, 9),
		new Peach(CardSuit::Heart, 10),
		new Peach(CardSuit::Heart, 12),

		new Peach(CardSuit::Diamond, 2),
		new Analeptic(CardSuit::Spade, 9),
		new Analeptic(CardSuit::Club, 9),
		new Analeptic(CardSuit::Diamond, 9),

		//Equip Cards
		new Crossbow(CardSuit::Diamond, 1),
		new DoubleSword,
		new QinggangSword,
		new IceSword,
		new Spear,
		new Fan,
		new Axe,
		new KylinBow,
		new SixSwords,
		new Triblade,

		new EightDiagram(CardSuit::Spade, 2),
		new RenwangShield,
		new Vine,
		new SilverLion
	};

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

	addQunGenerals();
	addShuGenerals();
	addWeiGenerals();
	addWuGenerals();
}

bool HegStandardPackage::isAvailable(const GameMode *mode) const
{
	return mode->name() == "hegemony";
}

ADD_PACKAGE(HegStandard)
