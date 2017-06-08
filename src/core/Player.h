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
#include "CardFilter.h"
#include "PlayerPhase.h"
#include "SkillAreaType.h"

#include <Json.h>

#include <vector>
#include <set>
#include <map>

class Card;
class CardArea;
class EventHandler;
class General;
class Skill;
class SkillArea;

class Player
{
public:
	using Phase = PlayerPhase;

	Player(uint id);
	~Player();

	uint id() const { return m_id; }

	int hp() const { return m_hp; }
	void setHp(int hp) { m_hp = hp; }

	int maxHp() const { return m_maxHp; }
	void setMaxHp(int maxHp) { m_maxHp = maxHp; }

	int lostHp() const { return m_maxHp - m_hp; }
	bool isWounded() const { return hp() < 0 || hp() < maxHp(); }

	void setAlive(bool alive) { m_alive = alive; }
	bool isAlive() const { return m_alive; }
	void setDead(bool dead) { setAlive(!dead); }
	bool isDead() const { return !m_alive; }

	void setRemoved(bool removed) { m_removed = removed; }
	bool isRemoved() const { return m_removed; }

	void setSeat(int seat) { m_seat = seat; }
	int seat() const { return m_seat; }

	void setNext(Player *next) { m_next = next; }
	Player *next() const { return m_next; }
	Player *next(bool ignoreRemoved) const;
	Player *nextAlive(int step = 1, bool ignoreRemoved = true) const;

	void setPhase(Phase phase) { m_phase = phase; }
	Phase phase() const { return m_phase; }
	void setPhaseString(const std::string &phase);
	std::string phaseString() const;

	int turnCount() const { return m_turnCount; }
	void setTurnCount(int count) { m_turnCount = count; }

	bool faceUp() const { return m_faceUp; }
	void setFaceUp(bool faceUp);

	void setDrunk(bool drunk);
	bool isDrunk() const { return m_drunk; }

	std::string kingdom() const { return m_kingdom; }
	void setKingdom(const std::string &kingdom);

	std::string role() const { return m_role; }
	void setRole(const std::string &role);

	int attackRange() const { return m_attackRange; }
	void setAttackRange(int range);
	bool inAttackRangeOf(const Player *attacker) const;

	bool isChained() const { return m_chained; }
	void setChained(bool chained);

	bool isDying() const { return m_dying; }
	void setDying(bool dying);

	//Alias of head general.
	const General *general() const { return headGeneral(); }
	void setGeneral(const General *general) { setHeadGeneral(general); }

	const General *headGeneral() const { return m_headGeneral; }
	void setHeadGeneral(const General *general);

	const General *deputyGeneral() const { return m_deputyGeneral; }
	void setDeputyGeneral(const General *general);

	bool hasShownHeadGeneral() const { return m_headGeneralShown; }
	void setHeadGeneralShown(bool shown) { m_headGeneralShown = shown; }

	bool hasShownDeputyGeneral() const { return m_deputyGeneralShown; }
	void setDeputyGeneralShown(bool shown) { m_deputyGeneralShown = shown; }

	bool hasShownGeneral() const { return hasShownHeadGeneral() || hasShownDeputyGeneral(); }
	bool hasShownBothGenerals() const { return hasShownHeadGeneral() && hasShownDeputyGeneral(); }

	int cardHistory(const std::string &name) const { return m_cardHistory.at(name); }
	void addCardHistory(const std::string &name, int times = 1);
	void clearCardHistory() { m_cardHistory.clear(); }

	int distanceTo(const Player *other) const;
	void setFixedDistance(const Player *other, int distance) { m_fixedDistance[other] = distance; }
	void unsetFixedDistance(const Player *other) { m_fixedDistance.erase(other); }

	//Extra distance from you to other players
	int extraOutDistance() const { return m_extraOutDistance; }
	void setExtraOutDistance(int extra) { m_extraOutDistance = extra; }

	//Extra distance from other players to you
	int extraInDistance() const { return m_extraInDistance; }
	void setExtraInDistance(int extra) { m_extraInDistance = extra; }

	CardArea *handcardArea() { return m_handcardArea; }
	const CardArea *handcardArea() const { return m_handcardArea; }
	int handcardNum() const { return m_handcardNum; }
	bool hasHandcard() const { return handcardNum() > 0; }

	const CardArea *equipArea() const { return m_equipArea; }
	CardArea *equipArea() { return m_equipArea; }
	int equipNum() const;
	bool hasEquip() const { return equipNum() > 0; }

	CardArea *delayedTrickArea() { return m_delayedTrickArea; }
	const CardArea *delayedTrickArea() const { return m_delayedTrickArea; }
	int delayedTrickNum() const;

	bool isNude() const { return !hasHandcard() && !hasEquip(); }
	bool isAllNude() const { return isNude() && delayedTrickNum() <= 0; }

	CardArea *judgeCards() { return m_judgeCards; }
	const CardArea *judgeCards() const { return m_judgeCards; }

	bool hasSkill(const Skill *skill) const;
	bool hasShownSkill(const Skill *skill) const;

	std::vector<const Skill *> getGlobalSkills() const;

	void addCardFilter(const CardFilter *filter);
	void removeCardFilter(const CardFilter *filter);
	const std::vector<const CardFilter *> &getCardFilters() const { return m_cardFilters; }

	void addSkill(const Skill *skill, SkillAreaType area = SkillAreaType::Acquired);
	void removeSkill(const Skill *skill);
	void removeSkill(const Skill *skill, SkillAreaType area);
	std::vector<const Skill *> skills() const;
	const SkillArea *headSkillArea() const { return m_headSkillArea; }
	const SkillArea *deputySkillArea() const { return m_deputySkillArea; }
	const SkillArea *acquiredSkillArea() const { return m_acquiredSkillArea; }

	const Skill *getSkill(uint id) const;

	const std::map<const Skill *, int> &skillHistory() const { return m_skillHistory; }
	void clearSkillHistory() { m_skillHistory.clear(); }
	void addSkillHistory(const Skill *skill) { m_skillHistory[skill]++; }
	int skillHistory(const Skill *skill) const { return m_skillHistory.at(skill); }

	std::map<std::string, KA_IMPORT Json> tag;

protected:
	uint m_id;
	int m_hp;
	int m_maxHp;
	bool m_alive;
	bool m_removed;
	int m_seat;
	Player *m_next;
	Phase m_phase;
	int m_turnCount;
	bool m_faceUp;
	int m_handcardNum;
	bool m_drunk;
	std::string m_kingdom;
	std::string m_role;
	int m_attackRange;
	bool m_chained;
	bool m_dying;

	const General *m_headGeneral;
	const General *m_deputyGeneral;
	bool m_headGeneralShown;
	bool m_deputyGeneralShown;

	std::map<const Player *, int> m_fixedDistance;
	int m_extraOutDistance;
	int m_extraInDistance;

	std::map<std::string, int> m_cardHistory;
	CardArea *m_handcardArea;
	CardArea *m_equipArea;
	CardArea *m_delayedTrickArea;
	CardArea *m_judgeCards;

	SkillArea *m_headSkillArea;
	SkillArea *m_deputySkillArea;
	SkillArea *m_acquiredSkillArea;
	std::vector<const CardFilter *> m_cardFilters;

	std::map<const Skill *, int> m_skillHistory;
};
