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

#include "CardPattern.h"

#include "Card.h"
#include "CardArea.h"
#include "Player.h"

#include <sstream>
#include <string.h>

namespace
{
	void split_string(std::vector<std::string> &strs, const std::string &str, size_t offset, size_t end, char needle)
	{
		size_t from = offset;
		size_t to = offset;
		while (to < end) {
			if (str[to] == needle) {
				size_t length = to - from;
				if (length > 0) {
					strs.push_back(str.substr(from, length));
				}
				from = to + 1;
			}
			to++;
		}
		size_t length = to - from;
		if (length > 0) {
			strs.push_back(str.substr(from, length));
		}
	}
}

CardPattern::CardPattern(const std::string &pattern)
{
	size_t pattern_end = pattern.size();

	size_t exp_offset = 0;
	size_t exp_end = 0;
	size_t subexp_offset = 0;
	size_t subexp_end = 0;
	size_t factor_offset = 0;
	size_t factor_end = 0;

	while (exp_offset < pattern_end) {
		exp_end = pattern.find('#', exp_offset);
		if (exp_end == std::string::npos) {
			exp_end = pattern_end;
		}

		Exp exp;
		std::vector<std::string> *exp_arr[] = {
			&exp.types,
			&exp.suits,
			&exp.numbers,
			&exp.places,
		};
		int exp_i = 0;

		size_t subexp_offset = exp_offset;
		size_t subexp_end = exp_offset;
		while (subexp_offset < exp_end) {
			subexp_end = pattern.find('|', subexp_offset);
			if (subexp_end == std::string::npos) {
				subexp_end = exp_end;
			}

			split_string(*(exp_arr[exp_i]), pattern, subexp_offset, subexp_end, ',');
			subexp_offset = subexp_end + 1;
			exp_i++;
		}
		m_exps.push_back(exp);
		exp_offset = exp_end + 1;
	}
}

bool CardPattern::match(const Player *player, const Card *card) const
{
	for (const Exp &exp : m_exps)
		if (matchOne(player, card, exp))
			return true;
	return false;
}

// '|' means 'and', '#' means 'or'.
// the expression splited by '|' has 3 parts,
// 1st part means the card name, and ',' means more than one options.
// 2nd patt means the card suit, and ',' means more than one options.
// 3rd part means the card number, and ',' means more than one options,
// the number uses '~' to make a scale for valid expressions
bool CardPattern::matchOne(const Player *player, const Card *card, const Exp &exp) const
{
	if (exp.types.empty()) {
		return false;
	}

	for (const std::string &type : exp.types) {
		if (type == ".") {
			break;
		}

		if (type.front() != '^') {
			if (!card->is(type.c_str())) {
				return false;
			}
		} else {
			if (card->is(type.c_str() + 1)) {
				return false;
			}
		}
	}

	for (const std::string &suit : exp.suits) {
		if (suit == ".") {
			break;
		}

		bool positive = true;
		const char *_suit = suit.c_str();
		if (suit.front() == '^') {
			positive = false;
			_suit++;
		}
		if (strcmp(card->suitString(), _suit) == 0
			|| (card->color() == CardColor::Black && strcmp(_suit, "black") == 0)
			|| (card->color() == CardColor::Red && strcmp(_suit, "red") == 0)) {
			if (!positive) {
				return false;
			}
		} else {
			if (positive) {
				return false;
			}
		}
	}

	int card_number = card->number();
	for (const std::string &number : exp.numbers) {
		if (number == ".") {
			break;
		}

		bool isInt = false;
		if (number.find('~') > 0) {
			int from = 1;
			int to = 13;
			sscanf(number.c_str(), "%d~%d", &from, &to);

			if (card_number < from || card_number > to) {
				return false;
			}

		} else if ((number == "A" && card_number != 1)
			|| (number == "J" && card_number != 11)
			|| (number == "Q" && card_number != 12)
			|| (number == "K" && card_number != 13)) {
			return false;
		} else {
			std::stringstream ss;
			ss << number;
			int criteria = 0;
			ss >> criteria;
			if (criteria != card_number) {
				return false;
			}
		}
	}

	if (player) {
		for (const std::string &place : exp.places) {
			if (place == ".") {
				break;
			}

			if (place == "equipped") {
				if (!player->equipArea()->contains(card)) {
					return false;
				}
			} else if (place == "hand") {
				if (!player->handcardArea()->contains(card)) {
					return false;
				}
			} else {
				//@to-do: pile cards checking
			}
		}
	}

	return true;
}
