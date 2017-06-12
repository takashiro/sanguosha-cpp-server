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

class Crossbow : public Weapon
{
	SGS_CARD(Crossbow, Weapon)
public:
	Crossbow(Suit suit, int number);
};

class DoubleSword : public Weapon
{
	SGS_CARD(Crossbow, Weapon)

public:
	DoubleSword(Suit suit = Suit::Spade, int number = 2);
};

class QinggangSword : public Weapon
{
	SGS_CARD(QinggangSword, Weapon)

public:
	QinggangSword(Suit suit = Suit::Spade, int number = 6);
};

class IceSword : public Weapon
{
	SGS_CARD(IceSword, Weapon)

public:
	IceSword(Suit suit = Suit::Spade, int number = 2);
};

class Spear : public Weapon
{
	SGS_CARD(Spear, Weapon)

public:
	Spear(Suit suit = Suit::Spade, int number = 12);
};

class Axe : public Weapon
{
	SGS_CARD(Axe, Weapon)

public:
	Axe(Suit suit = Suit::Diamond, int number = 5);
};

class Triblade : public Weapon
{
	SGS_CARD(Triblade, Weapon)

public:
	Triblade(Suit suit = Suit::Diamond, int number = 12);
};

class KylinBow : public Weapon
{
	SGS_CARD(KylinBow, Weapon)

public:
	KylinBow(Suit suit = Suit::Heart, int number = 5);
};

class EightDiagram : public Armor
{
	SGS_CARD(EightDiagram, Armor)

public:
	EightDiagram(Suit suit, int number);
};

class RenwangShield : public Armor
{
	SGS_CARD(RenwangShield, Armor)

public:
	RenwangShield(Suit suit = Suit::Club, int number = 2);
};
