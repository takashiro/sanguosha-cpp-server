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

#include "StandardMode.h"

#include "CardArea.h"
#include "Engine.h"
#include "GameLogic.h"
#include "GameRule.h"
#include "General.h"
#include "Package.h"
#include "ServerPlayer.h"

#include <random>
#include <algorithm>

static std::vector<ServerPlayer *> GetPlayersByRole(GameLogic *logic, const std::string &role)
{
	std::vector<ServerPlayer *> result;
	std::vector<ServerPlayer *> allPlayers = logic->allPlayers(true);
	for(ServerPlayer *player : allPlayers) {
		if (player->role() == role)
			result.push_back(player);
	}
	return result;
}

StandardMode::StandardMode()
{
	m_name = "standard";

	m_rules.push_back(new GameRule(PrepareToStart, [] (GameLogic *logic, ServerPlayer *, void *) {
		std::vector<ServerPlayer *> players = logic->players();
		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(players.begin(), players.end(), g);

		size_t playerNum = players.size();
		ServerPlayer *lord = players.front();
		players.erase(players.begin());

		lord->setRole("lord");
		lord->broadcastProperty("role", "lord");

		size_t renegadeNum = playerNum > 4 ? 1 : 0;
		size_t rebelNum = playerNum / 2;
		size_t loyalistNum = playerNum - 1 - renegadeNum - rebelNum;

		std::map<std::string, size_t> roleMap;
		roleMap["renagade"] = renegadeNum;
		roleMap["rebel"] = rebelNum;
		roleMap["loyalist"] = loyalistNum;

		int playerIndex = 0;
		for (const std::pair<std::string, size_t> &iter : roleMap) {
			size_t num = iter.second;
			for (size_t i = 0; i < num; i++) {
				ServerPlayer *player = players.at(playerIndex);
				playerIndex++;
				player->setRole(iter.first);
			}
		}
		
		for (ServerPlayer *player : players) {
			player->unicastProperty("role", player->role(), player);
			player->broadcastProperty("role", "unknown");
		}

		GeneralList generals;
		std::vector<const Package *> packages = logic->packages();
		for (const Package *package : packages) {
			const std::vector<const General *> &package_generals = package->generals();
			size_t offset = generals.size();
			generals.resize(offset + package_generals.size());
			std::copy(package_generals.begin(), package_generals.end(), generals.begin() + offset);
		}

		std::shuffle(generals.begin(), generals.end(), g);

		GeneralList lord_candidates;
		for (const General *general : generals) {
			if (general->isLord())
				lord_candidates.push_back(general);
		}
		lord_candidates.push_back(generals.at(0));
		lord_candidates.push_back(generals.at(1));
		GeneralList reply_data = lord->askForGeneral(lord_candidates, 1);
		const General *reply_general = reply_data.front();
		lord->setGeneral(reply_general);
		lord->broadcastProperty("general_id", reply_general->id());

		std::map<uint, GeneralList> replies = logic->broadcastRequestForGenerals(players, 1, 5);
		for (const std::pair<uint, GeneralList> &iter : replies) {
			ServerPlayer *player = logic->findPlayer(iter.first);
			const GeneralList &generals = iter.second;
			const General *general = generals.front();
			player->setGeneral(general);
			player->broadcastProperty("general_id", general->id());
		}
		return false;
	}));

	m_rules.push_back(new GameRule(GameStart, [] (GameLogic *, ServerPlayer *current, void *) {
		current->broadcastProperty("general_id", current->general()->id());

		const General *general = current->general();

		int hp = general->maxHp();
		current->setMaxHp(hp);
		current->setHp(hp);
		current->broadcastProperty("maxHp", hp);
		current->broadcastProperty("hp", hp);

		current->setKingdom(general->kingdom());
		current->broadcastProperty("kingdom", general->kingdom());

		std::vector<const Skill *> skills = general->skills();
		for (const Skill *skill : skills)
			current->addSkill(skill);

		current->drawCards(4);
		return false;
	}));

	m_rules.push_back(new GameRule(BuryVictim, [] (GameLogic *logic, ServerPlayer *victim, void *data) {
		DeathStruct *death = static_cast<DeathStruct *>(data);
		if (death->damage == nullptr)
			return false;

		ServerPlayer *killer = death->damage->from;
		if (killer->isDead())
			return false;

		if (victim->role() == "rebel") {
			killer->drawCards(3);
		} else if (victim->role() == "loyalist" && killer->role() == "lord") {
			CardsMoveStruct discard;
			discard << killer->handcardArea()->cards();
			discard << killer->equipArea()->cards();
			discard.to.type = CardAreaType::DiscardPile;
			discard.isOpen = true;
			logic->moveCards(discard);
		}
		return false;
	}));

	m_rules.push_back(new GameRule(GameOverJudge, [] (GameLogic *logic, ServerPlayer *victim, void *) {
		std::vector<ServerPlayer *> winners;
		std::vector<ServerPlayer *> alivePlayers = logic->allPlayers();

		const std::string &victim_role = victim->role();
		victim->broadcastProperty("role", victim_role);
		if (victim_role == "lord") {
			if (alivePlayers.size() == 1 && alivePlayers.front()->role() == "renagade") {
				winners = alivePlayers;
			} else {
				winners = GetPlayersByRole(logic, "rebel");
			}
		} else {
			bool lordWin = true;
			for(ServerPlayer *player : alivePlayers) {
				if (player->role() == "rebel" || player->role() == "renagade") {
					lordWin = false;
					break;
				}
			}
			if (lordWin) {
				std::vector<ServerPlayer *> lord = GetPlayersByRole(logic, "lord");
				std::vector<ServerPlayer *> loyalists = GetPlayersByRole(logic, "loyalist");
				winners.resize(lord.size() + loyalists.size());

				auto i = winners.begin();
				i = std::copy(lord.begin(), lord.end(), i);
				i = std::copy(loyalists.begin(), loyalists.end(), i);
			}
		}

		if (winners.size() > 0) {
			for (ServerPlayer *player : alivePlayers)
				player->broadcastProperty("role", player->role());

			ServerPlayer *current = logic->currentPlayer();
			current->setPhase(PlayerPhase::Inactive);
			current->broadcastProperty("phase", "inactive");

			logic->gameOver(winners);
		}

		return false;
	}));
}

ADD_MODE(Standard)
