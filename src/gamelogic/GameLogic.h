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

#include "Event.h"
#include "EventType.h"
#include "structs.h"

#include <GameDriver.h>

#include <list>
#include <map>
#include <vector>

class Card;
class CardArea;
class GameMode;
class General;
class ServerPlayer;
class Package;
class GameConfig;
enum class cmd;

class GameLogic : public KA_IMPORT GameDriver
{
public:
	GameLogic();
	~GameLogic();

	void addPlayer(KA_IMPORT User *user) override;

	const GameConfig *config() const { return m_config; }

	const std::vector<const Package *> &packages() const { return m_packages; }
	void setPackages(const std::vector<const Package *> &packages) { m_packages = packages; }

	void addEventHandler(const EventHandler *handler);
	void removeEventHandler(const EventHandler *handler);

	// Fire game rules only
	void fire(EventType event, ServerPlayer *target = nullptr, void *data = nullptr);
	// Trigger all events
	bool trigger(EventType event, ServerPlayer *target, void *data = nullptr);

	void setCurrentPlayer(ServerPlayer *player) { m_currentPlayer = player; }
	ServerPlayer *currentPlayer() const { return m_currentPlayer; }

	const std::vector<ServerPlayer *> &players() const;
	ServerPlayer *findPlayer(uint id) const;

	std::vector<ServerPlayer *> allPlayers(bool includeDead = false) const;
	std::vector<ServerPlayer *> otherPlayers(ServerPlayer *except, bool includeDead = false) const;
	int countPlayer(bool include_dead = false) const;
	int countPlayer(bool(*filter)(ServerPlayer *)) const;
	void sortByActionOrder(std::vector<ServerPlayer *> &players) const;

	void addExtraTurn(ServerPlayer *player) { m_extraTurns.push_back(player); }
	const std::list<ServerPlayer *> &extraTurns() const { return m_extraTurns; }

	const Card *getDrawPileCard();
	std::vector<const Card *> getDrawPileCards(int n);
	void reshuffleDrawPile();
	int reshufflingCount() const { return m_reshufflingCount; }

	CardArea *drawPile() const { return m_drawPile; }
	CardArea *discardPile() const { return m_discardPile; }
	CardArea *table() const { return m_table; }
	CardArea *wugu() const { return m_wugu; }

	void moveCards(const CardsMoveStruct &move);
	void moveCards(std::vector<CardsMoveStruct> &moves);

	bool useCard(CardUseStruct &use);
	bool takeCardEffect(CardEffectStruct &effect);

	//It returns false iff the corresponding CardResponded event is broken.
	bool respondCard(CardResponseStruct &response);

	void judge(JudgeStruct &judge);

	const Card *findCard(uint id) const { return m_cards.at(id); }
	std::vector<const Card *> findCards(const KA_IMPORT Json &data);

	void damage(DamageStruct &damage);
	void loseHp(ServerPlayer *victim, int lose);
	void recover(RecoverStruct &recover);

	void killPlayer(ServerPlayer *victim, DamageStruct *damage = nullptr);
	void gameOver(const std::vector<ServerPlayer *> &winners);

	std::map<uint, std::vector<const General *>> broadcastRequestForGenerals(const std::vector<ServerPlayer *> &players, int num, int limit);

	void broadcastNotification(cmd command);
	void broadcastNotification(cmd command, const KA_IMPORT Json &arguments);
	void broadcastNotification(cmd command, const KA_IMPORT Json &arguments, ServerPlayer *except);

	void broadcastRequest(const std::vector<ServerPlayer *> &players);

protected:
	void loadMode(const GameMode *mode);

	void prepareToStart();
	CardArea *findArea(const CardsMoveStruct::Area &area);
	void filterCardsMove(std::vector<CardsMoveStruct> &moves);

	void run();

private:
	GameConfig *m_config;

	std::vector<const EventHandler *> m_rules;
	std::vector<const EventHandler *> m_handlers[EventTypeCount];

	std::vector<ServerPlayer *> m_players;
	ServerPlayer *m_currentPlayer;
	std::list<ServerPlayer *> m_extraTurns;
	std::vector<const Package *> m_packages;
	std::map<uint, const Card *> m_cards;
	int m_round;
	int m_reshufflingCount;

	CardArea *m_drawPile;
	CardArea *m_discardPile;
	CardArea *m_table;
	CardArea *m_wugu;

	std::map<const Card *, CardArea *> m_cardPosition;
};
