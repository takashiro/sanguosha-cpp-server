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

#include <string>
#include <vector>

class Card;
class General;
class GameMode;

class Package
{
public:
	Package(const std::string &name);
	virtual ~Package();

	virtual bool isAvailable(const GameMode *mode) const;

	std::string name() const { return m_name; }
	std::vector<const General *> generals(bool includeHidden = false) const;
	const General *getGeneral(const std::string &name) const;
	std::vector<const Card *> cards() const;

protected:
	void addGeneral(General *general);
	void addGenerals(const std::vector<General *> &generals);

	void addCard(Card *card);
	void addCards(const std::vector<Card *> &cards);

	std::string m_name;
	std::vector<General *> m_generals;
	std::vector<Card *> m_cards;
};
