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

#include "BasicMode.h"

#include "CardArea.h"
#include "GameLogic.h"
#include "GameRule.h"
#include "ServerPlayer.h"

#include "cards.h"
#include "structs.h"

#include <Json.h>

#include <chrono>
#include <thread>

KA_USING_NAMESPACE

static std::vector<const EventHandler *> basic_rules()
{
	std::vector<const EventHandler *> rules = {
		new GameRule(TurnStart, [] (GameLogic *, ServerPlayer *current, void *) {
			current->setTurnCount(current->turnCount() + 1);
			if (!current->faceUp()) {
				current->setFaceUp(true);
			} else {
				current->play();
			}
			return false;
		}),

		new GameRule(PhaseProceeding, [] (GameLogic *logic, ServerPlayer *current, void *) {
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			switch (current->phase()) {
			case PlayerPhase::Judge: {
				CardArea *delayed_trick = current->delayedTrickArea();
				std::deque<Card *> tricks = current->delayedTrickArea()->cards();
				while (delayed_trick->size() > 0 && current->isAlive()) {
					Card *trick = delayed_trick->takeLast();

					if (trick->type() == CardType::Trick && trick->subtype() == TrickCard::DelayedType) {
						CardUseStruct use;
						use.card = trick;
						use.to.push_back(current);
						CardEffectStruct effect(use);
						effect.to = current;
						logic->takeCardEffect(effect);
						trick->complete(logic);
					}
				}
				break;
			}
			case PlayerPhase::Draw: {
				int num = 2;
				void *data = static_cast<void *>(&num);
				logic->trigger(DrawNCards, current, data);
				if (num > 0)
					current->drawCards(num);
				logic->trigger(AfterDrawNCards, current, data);
				break;
			}
			case PlayerPhase::Play: {
				while (current->isAlive()) {
					if (current->activate())
						break;
				}
				break;
			}
			case PlayerPhase::Discard: {
				int maxCardNum = current->hp();
				void *data = static_cast<void *>(&maxCardNum);
				logic->trigger(CountMaxCardNum, current, data);
				int discardNum = current->handcardNum() - maxCardNum;
				if (discardNum > 0) {
					current->showPrompt("ask_to_discard", discardNum);
					std::vector<Card *> cards = current->askForCards(".|.|.|hand", discardNum);

					CardsMoveStruct move;
					move.cards = cards;
					move.to.type = CardAreaType::DiscardPile;
					move.isOpen = true;
					logic->moveCards(move);
				}
				break;
			}
			default:;
			}
			return false;
		}),

		new GameRule(PhaseEnd, [] (GameLogic *, ServerPlayer *current, void *) {
			switch (current->phase()) {
			case PlayerPhase::Play:
			{
				current->clearCardHistory();
				current->clearSkillHistory();
			}
			default:;
			}
			if (current->isDrunk()) {
				current->setDrunk(false);
				current->broadcastProperty("drunk", false);
			}
			return false;
		}),

		new GameRule(AfterHpReduced, [] (GameLogic *logic, ServerPlayer *victim, void *data) {
			if (victim->hp() > 0)
				return false;

			victim->setDying(true);
			victim->broadcastProperty("dying", true);

			DeathStruct death;
			death.who = victim;
			death.damage = static_cast<DamageStruct *>(data);

			void *dyingData = static_cast<void *>(&death);

			std::vector<ServerPlayer *> allPlayers = logic->allPlayers();
			for (ServerPlayer *player : allPlayers) {
				if (logic->trigger(EnterDying, player, dyingData) || victim->hp() > 0 || victim->isDead())
					break;
			}

			if (victim->isAlive() && victim->hp() <= 0) {
				std::vector<ServerPlayer *> allPlayers = logic->allPlayers();
				for (ServerPlayer *saver : allPlayers) {
					if (victim->hp() > 0 || victim->isDead())
						break;

					logic->trigger(AskForPeach, saver, dyingData);
				}
				logic->trigger(AskForPeachDone, victim, dyingData);
			}

			victim->setDying(false);
			victim->broadcastProperty("dying", false);
			logic->trigger(QuitDying, victim, dyingData);

			return false;
		}),

		new GameRule(AskForPeach, [] (GameLogic *logic, ServerPlayer *current, void *data) {
			DeathStruct *dying = static_cast<DeathStruct *>(data);
			while (dying->who->hp() <= 0) {
				Card *peach = nullptr;
				if (dying->who->isAlive()) {
					int peachNum = 1 - dying->who->hp();
					if (current != dying->who) {
						JsonArray args;
						args.push_back("player");
						args.push_back(dying->who->id());
						args.push_back(peachNum);
						current->showPrompt("ask_for_peach", args);
						peach = current->askForCard("Peach");
					} else {
						current->showPrompt("ask_self_for_peach_or_analeptic", peachNum);
						peach = current->askForCard("Peach,Analeptic");
					}
				}
				if (peach == nullptr)
					break;

				CardUseStruct use;
				use.from = current;
				use.card = peach;
				use.to.push_back(dying->who);
				logic->useCard(use);
			}
			return false;
		}),

		new GameRule(AskForPeachDone, [] (GameLogic *logic, ServerPlayer *victim, void *data) {
			if (victim->hp() <= 0 && victim->isAlive()) {
				DeathStruct *death = static_cast<DeathStruct *>(data);
				logic->killPlayer(victim, death->damage);
			}
			return false;
		})
	};
	return rules;
}


BasicMode::BasicMode()
{
	m_rules = basic_rules();
}
