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

#include "Player.h"

#include "CardArea.h"
#include "CardAreaType.h"
#include "SkillArea.h"

#include <algorithm>

Player::Player(uint id)
	: m_id(id)
	, m_hp(0)
	, m_maxHp(0)
	, m_alive(true)
	, m_removed(false)
	, m_seat(0)
	, m_next(nullptr)
	, m_phase(Phase::Inactive)
	, m_turnCount(0)
	, m_faceUp(false)
	, m_handcardNum(0)
	, m_drunk(false)
	, m_kingdom("hidden")
	, m_role("hidden")
	, m_attackRange(1)
	, m_chained(false)
	, m_dying(false)
	, m_headGeneral(nullptr)
	, m_deputyGeneral(nullptr)
	, m_headGeneralShown(false)
	, m_deputyGeneralShown(false)
	, m_extraOutDistance(0)
	, m_extraInDistance(0)
	, m_handcardArea(new CardArea(CardAreaType::Hand, this))
	, m_equipArea(new CardArea(CardAreaType::Equip, this))
	, m_delayedTrickArea(new CardArea(CardAreaType::DelayedTrick, this))
	, m_judgeCards(new CardArea(CardAreaType::Judge, this))
	, m_headSkillArea(new SkillArea(SkillAreaType::Head))
	, m_deputySkillArea(new SkillArea(SkillAreaType::Deputy))
	, m_acquiredSkillArea(new SkillArea(SkillAreaType::Acquired))
{

	m_handcardArea->setSignal([this] () {
		m_handcardNum = m_handcardArea->size();
	});
	m_equipArea->setKeepVirtualCard(true);
	m_delayedTrickArea->setKeepVirtualCard(true);
	m_judgeCards->setKeepVirtualCard(true);
}

Player::~Player()
{
	delete m_acquiredSkillArea;
	delete m_deputySkillArea;
	delete m_headSkillArea;
	delete m_handcardArea;
	delete m_equipArea;
	delete m_delayedTrickArea;
	delete m_judgeCards;
}

Player *Player::next(bool ignoreRemoved) const
{
	Player *next = this->next();
	if (ignoreRemoved) {
		while (next->isRemoved())
			next = next->next();
	}
	return next;
}

Player *Player::nextAlive(int step, bool ignoreRemoved) const
{
	Player *cur = next();
	for (;;) {
		if (cur->isAlive() && !(ignoreRemoved && cur->isRemoved())) {
			step--;
			if (step <= 0)
				break;
		}

		cur = cur->next();
		if (cur == this)
			break;
	}
	return cur;
}

void Player::setPhaseString(const std::string &phase)
{
	if (phase == "round_start")
		m_phase = Phase::RoundStart;
	else if (phase == "start")
		m_phase = Phase::Start;
	else if (phase == "judge")
		m_phase = Phase::Judge;
	else if (phase == "draw")
		m_phase = Phase::Draw;
	else if (phase == "play")
		m_phase = Phase::Play;
	else if (phase == "discard")
		m_phase = Phase::Discard;
	else if (phase == "finish")
		m_phase = Phase::Finish;
	else
		m_phase = Phase::Inactive;
}

std::string Player::phaseString() const
{
	switch (m_phase) {
	case Phase::RoundStart:
		return "round_start";
	case Phase::Start:
		return "start";
	case Phase::Judge:
		return "judge";
	case Phase::Draw:
		return "draw";
	case Phase::Play:
		return "play";
	case Phase::Discard:
		return "discard";
	case Phase::Finish:
		return "finish";
	default:
		return "inactive";
	}
}

void Player::setHeadGeneral(const General *general)
{
	m_headGeneral = general;
}

void Player::setDeputyGeneral(const General *general)
{
	m_deputyGeneral = general;
}

void Player::setFaceUp(bool faceUp)
{
	m_faceUp = faceUp;
}

int Player::equipNum() const
{
	return m_equipArea->size();
}

int Player::delayedTrickNum() const
{
	return m_delayedTrickArea->size();
}

int Player::distanceTo(const Player *other) const
{
	if (this == other)
		return 0;

	if (isRemoved() || other->isRemoved() || isDead() || other->isDead())
		return -1;

	std::map<const Player *, int>::const_iterator iter = m_fixedDistance.find(other);
	if (m_fixedDistance.find(other) != m_fixedDistance.end())
		return iter->second;

	const Player *next = this;
	int right = 0;
	while (next != other) {
		next = next->nextAlive();
		if (next == this)
			return -1;
		right++;
	}
	int left = 0;
	while (next != this) {
		next = next->nextAlive();
		left++;
	}

	int distance = std::min(left, right) + extraOutDistance() + other->extraInDistance();

	// keep the distance >=1
	if (distance < 1)
		distance = 1;

	return distance;
}

void Player::addCardHistory(const std::string &name, int times)
{
	std::map<std::string, int>::iterator iter = m_cardHistory.find(name);
	if (iter != m_cardHistory.end())
		iter->second += times;
	else
		m_cardHistory[name] = times;
}

void Player::setDrunk(bool drunk)
{
	m_drunk = drunk;
}

void Player::setKingdom(const std::string &kingdom)
{
	m_kingdom = kingdom;
}

void Player::setRole(const std::string &role)
{
	m_role = role;
}

void Player::setAttackRange(int range)
{
	m_attackRange = range;
}

bool Player::inAttackRangeOf(const Player *attacker) const
{
	return attacker->distanceTo(this) <= attacker->attackRange();
}

void Player::setChained(bool chained)
{
	m_chained = chained;
}

void Player::setDying(bool dying)
{
	m_dying = dying;
}


bool Player::hasSkill(const Skill *skill) const
{
	if (m_headSkillArea->contains(skill)) {
		return true;
	}

	if (m_deputySkillArea->contains(skill)) {
		return true;
	}

	if (m_acquiredSkillArea->contains(skill)) {
		return true;
	}

	return false;
}

bool Player::hasShownSkill(const Skill *skill) const
{
	if (hasShownHeadGeneral()) {
		if (m_headSkillArea->contains(skill)) {
			return true;
		}
	}
	if (hasShownDeputyGeneral()) {
		if (m_deputySkillArea->contains(skill)) {
			return true;
		}
	}

	if (m_acquiredSkillArea->contains(skill)) {
		return true;
	}

	return false;
}

std::vector<const Skill *> Player::getGlobalSkills() const
{
	std::vector<const Skill *> skills = this->skills();
	const Player *current = this->nextAlive(1, false);
	while (current != this) {
		std::vector<const Skill *> add_list = current->skills();
		for (const Skill *skill : add_list) {
			skills.push_back(skill);
		}
		current = current->nextAlive(1, false);
	}
	return skills;
}

void Player::addCardFilter(const CardFilter *filter)
{
	m_cardFilters.push_back(filter);
}

void Player::removeCardFilter(const CardFilter *filter)
{
	auto i = std::find(m_cardFilters.begin(), m_cardFilters.end(), filter);
	if (i != m_cardFilters.end()) {
		m_cardFilters.erase(i);
	}
}

void Player::addSkill(const Skill *skill, SkillAreaType type)
{
	if (type == SkillAreaType::Head) {
		m_headSkillArea->add(skill);
	} else if (type == SkillAreaType::Deputy) {
		m_deputySkillArea->add(skill);
	} else {
		m_acquiredSkillArea->add(skill);
	}
}

void Player::removeSkill(const Skill *skill)
{
	std::vector<const Skill *>::iterator i;

	if (m_acquiredSkillArea->remove(skill)) {
		return;
	}

	if (m_headSkillArea->remove(skill)) {
		return;
	}

	m_deputySkillArea->remove(skill);
}

void Player::removeSkill(const Skill *skill, SkillAreaType type)
{
	if (type == SkillAreaType::Head) {
		m_headSkillArea->remove(skill);
	} else if (type == SkillAreaType::Deputy) {
		m_deputySkillArea->remove(skill);
	} else {
		m_acquiredSkillArea->remove(skill);
	}
}

std::vector<const Skill *> Player::skills() const
{
	std::vector<const Skill *> result;
	result.resize(m_headSkillArea->size() + m_deputySkillArea->size() + m_acquiredSkillArea->size());

	auto from = result.begin();
	const std::vector<const Skill *> &head_skills = m_headSkillArea->skills();
	from = std::copy(head_skills.begin(), head_skills.end(), from);
	const std::vector<const Skill *> &deputy_skills = m_deputySkillArea->skills();
	from = std::copy(deputy_skills.begin(), deputy_skills.end(), from);
	const std::vector<const Skill *> &acquired_skills = m_acquiredSkillArea->skills();
	from = std::copy(acquired_skills.begin(), acquired_skills.end(), from);

	return result;
}
