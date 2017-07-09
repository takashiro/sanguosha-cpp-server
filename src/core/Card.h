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

#include "CardSuit.h"
#include "CardColor.h"
#include "CardType.h"
#include "CardFilter.h"

#include <string>
#include <vector>
#include <set>

#include <Json.h>

class GameLogic;
class Player;
class Skill;

struct CardUseStruct;
struct CardEffectStruct;

class Card
{
	friend class Package;

public:
	using Suit = CardSuit;
	using Color = CardColor;
	using Type = CardType;

	enum Number
	{
		InfinityNum = 1000
	};

	Card(Suit suit = Suit::None, int number = 0);
	virtual ~Card();
	virtual Card *clone() const = 0;

	bool is(const std::string &name) const;

	uint id() const { return m_id; }
	bool isVirtual() const { return id() == 0; }
	uint effectiveId() const;

	const std::string &name() const { return m_name; }
	void setName(const std::string &name) { m_name = name; }

	void setSuit(Suit suit) { m_suit = suit; }
	Suit suit() const;
	void setSuitString(const char *suit);
	const char *suitString() const;

	void setNumber(int number) { m_number = number; }
	int number() const;

	void setColor(Color color) { m_color = color; }
	Color color() const;
	void setColorString(const char *color);
	const char *colorString() const;

	void setType(Type type) { m_type = type; }
	Type type() const { return m_type; }
	int subtype() const { return m_subtype; }
	const char *typeString() const;

	void addSubcard(const Card *card);
	void setSubcards(const std::vector<const Card *> &cards) { m_subcards = cards; }
	std::vector<const Card *> subcards() const { return m_subcards; }

	const Card *realCard() const;
	std::vector<const Card *> realCards() const;

	void setSkill(const Skill *skill) { m_skill = skill; }
	const Skill *skill() const { return m_skill; }

	void addFlag(const std::string &flag) { m_flags.insert(flag); }
	void removeFlag(const std::string &flag) { m_flags.erase(flag); }
	bool hasFlag(const std::string &flag) const { return m_flags.find(flag) != m_flags.end(); }
	void clearFlags() { m_flags.clear(); }

	void setTransferable(bool transferable) { m_transferable = transferable; }
	bool isTransferable() const { return m_transferable; }

	bool canRecast() const { return m_canRecast; }
	int useLimit() const;
	int useLimit(const Player *source) const;
	int maxTargetNum() const;
	int minTargetNum() const;
	int distanceLimit() const;

	bool isTargetFixed() const { return m_targetFixed; }
	virtual bool targetFeasible(const std::vector<const Player *> &selected, const Player *source) const;
	virtual bool targetFilter(const std::vector<const Player *> &selected, const Player *toSelect, const Player *source) const;
	virtual bool isAvailable(const Player *source) const;

	bool isValid(const std::vector<const Player *> &targets, const Player *source) const;

	virtual void onUse(GameLogic *logic, CardUseStruct &use) const {}
	virtual void use(GameLogic *logic, CardUseStruct &use) const {}
	virtual void onEffect(GameLogic *logic, CardEffectStruct &effect) const {}
	virtual void effect(GameLogic *logic, CardEffectStruct &effect) const {}
	virtual void complete(GameLogic *logic) const {}

	static const Card *Find(const std::vector<const Card *> &cards, uint id);
	static std::vector<const Card *> Find(const std::vector<const Card *> &cards, const KA_IMPORT Json &data);

	KA_IMPORT JsonObject toJson() const;

protected:
	virtual bool inherits(const std::string &name) const { KA_UNUSED(name); return false; }

	uint m_id;
	std::string m_name;
	Suit m_suit;
	int m_number;
	Color m_color;
	Type m_type;
	int m_subtype;

	bool m_transferable;
	bool m_canRecast;
	int m_useLimit;
	int m_maxTargetNum;
	int m_minTargetNum;
	int m_distanceLimit;
	bool m_targetFixed;

	const Skill *m_skill;
	std::vector<const Card *> m_subcards;
	std::set<std::string> m_flags;
};

#define SGS_CARD_CLONE(classname) \
public:\
Card *clone() const override\
{\
	return new classname(suit(), number());\
}

#define SGS_CARD_HIERACHY(classname, parent) \
protected:\
bool inherits(const std::string &name) const override\
{\
	return name == #classname || parent::inherits(name);\
}

#define SGS_CARD(classname, parent) \
	SGS_CARD_CLONE(classname)\
	SGS_CARD_HIERACHY(classname, parent)
