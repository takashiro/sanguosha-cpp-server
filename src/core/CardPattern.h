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

#include <vector>
#include <string>

class Player;
class Card;

class CardPattern
{
public:
	CardPattern(const std::string &pattern);

	bool match(const Player *player, const Card *card) const;

private:
	struct Exp
	{
		std::vector<std::string> types;
		std::vector<std::string> suits;
		std::vector<std::string> numbers;
		std::vector<std::string> places;
	};

	bool matchOne(const Player *player, const Card *card, const Exp &exp) const;

	std::vector<Exp> m_exps;
};
