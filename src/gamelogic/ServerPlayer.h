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

#include "cmd.h"
#include "Event.h"
#include "Player.h"

#include <string>
#include <vector>
#include <set>

class Skill;
class GameLogic;

KA_NAMESPACE_BEGIN

class User;
class Json;

KA_NAMESPACE_END

class ServerPlayer : public Player
{
public:
	ServerPlayer(GameLogic *logic, KA_IMPORT User *user);
	~ServerPlayer();

	KA_IMPORT User *user() const { return m_user; }

	void drawCards(int n);
	void recastCard(Card *card);

	void showCard(Card *card);
	void showCards(const std::vector<Card *> &cards);

	void play();
	void play(const std::vector<Phase> &phases);
	bool activate();

	void skipPhase(Phase phase) { m_skippedPhase.insert(phase); }
	bool isPhaseSkipped(Phase phase) { return m_skippedPhase.find(phase) != m_skippedPhase.end(); }
	void clearSkippedPhase() { m_skippedPhase.clear(); }

	void showPrompt(const std::string &message, int number);
	void showPrompt(const std::string &message, const Card *card);
	void showPrompt(const std::string &message, const ServerPlayer *p1, const Card *card = nullptr);
	void showPrompt(const std::string &message, const ServerPlayer *p1, const ServerPlayer *p2, const Card *card = nullptr);
	void showPrompt(const std::string &message, const KA_IMPORT JsonArray &args = KA_IMPORT JsonArray());

	Event askForTriggerOrder(const EventList &options, bool cancelable);
	Card *askForCard(const std::string &pattern, bool optional = true);
	std::vector<Card *> askForCards(const std::string &pattern, int num, bool optional = false);
	std::vector<Card *> askForCards(const std::string &pattern, int minNum, int maxNum, bool optional = false);
	Card *askToChooseCard(ServerPlayer *owner, const std::string &areaFlag = "hej", bool handcardVisible = false);
	bool askToUseCard(const std::string &pattern, const std::vector<ServerPlayer *> &assignedTargets = std::vector<ServerPlayer *>());
	std::vector<std::vector<Card *>> askToArrangeCard(const std::vector<Card *> &cards, const std::vector<int> &capacities, const std::vector<std::string> &areaNames = std::vector<std::string>());
	std::string askForOption(const std::vector<std::string> &options);

	void broadcastProperty(const char *name, const KA_IMPORT Json &value, ServerPlayer *except = nullptr) const;
	void unicastProperty(const char *name, const KA_IMPORT Json &value, ServerPlayer *to);

	void addSkillHistory(const Skill *skill);
	void addSkillHistory(const Skill *skill, const std::vector<Card *> &cards);
	void addSkillHistory(const Skill *skill, const std::vector<ServerPlayer *> &targets);
	void addSkillHistory(const Skill *skill, const std::vector<Card *> &cards, const std::vector<ServerPlayer *> &targets);
	void clearSkillHistory();

	void addCardHistory(const std::string &name, int times = 1);
	void clearCardHistory();

	const Skill *getSkill(uint id) const;

	void addSkill(const Skill *skill, SkillAreaType type = SkillAreaType::Acquired);
	void removeSkill(const Skill *skill, SkillAreaType type = SkillAreaType::Unknown);

	//These two functions don't trigger SkillAdded/SkillRemoved events
	void attachSkill(const Skill *skill, SkillAreaType type = SkillAreaType::Acquired);
	void detachSkill(const Skill *skill, SkillAreaType type = SkillAreaType::Unknown);

	void broadcastTag(const std::string &key);
	void unicastTag(const std::string &key, ServerPlayer *to);

	std::vector<const General *> askForGeneral(const std::vector<const General *> &candidates, int num);

	//Network functions
	KA_IMPORT Json request(cmd command, const KA_IMPORT Json &arguments, int timeout = 0);
	void notify(cmd command); 
	void notify(cmd command, const KA_IMPORT Json &arguments);

	void prepareRequest(cmd command, const KA_IMPORT Json &arguments, int timeout = 0);

private:
	GameLogic *m_logic;
	std::set<Phase> m_skippedPhase;
	KA_IMPORT User *m_user;
};
