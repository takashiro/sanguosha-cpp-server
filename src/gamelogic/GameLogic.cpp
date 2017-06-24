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

#include "GameLogic.h"

#include "cmd.h"

#include "Card.h"
#include "CardArea.h"
#include "Engine.h"
#include "EventHandler.h"
#include "GameConfig.h"
#include "GameMode.h"
#include "General.h"
#include "Package.h"
#include "ServerPlayer.h"

#include <Room.h>
#include <User.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <random>

KA_USING_NAMESPACE

GameLogic::GameLogic()
	: m_currentPlayer(nullptr)
	, m_round(0)
	, m_reshufflingCount(0)
{
	m_drawPile = new CardArea(CardAreaType::DrawPile);
	m_discardPile = new CardArea(CardAreaType::DiscardPile);
	m_table = new CardArea(CardAreaType::Table);
	m_table->setKeepVirtualCard(true);
	m_wugu = new CardArea(CardAreaType::Wugu);
}

GameLogic::~GameLogic()
{
	delete m_wugu;
	delete m_table;
	delete m_discardPile;
	delete m_drawPile;

	for (ServerPlayer *player : m_players) {
		delete player;
	}
}

void GameLogic::start()
{
	try {
		run();
	} catch (...) {
		std::cerr << "unexpected exception caught." << std::endl;
	}
}

void GameLogic::end()
{
}

void GameLogic::addPlayer(KA_IMPORT User *user)
{
	m_players.push_back(new ServerPlayer(this, user));
}

void GameLogic::removePlayer(KA_IMPORT User *user)
{
	for (auto i = m_players.begin(); i != m_players.end(); i++) {
		if ((*i)->user() == user) {
			m_players.erase(i);
			break;
		}
	}
}

void GameLogic::addEventHandler(const EventHandler *handler)
{
	const std::set<EventType> &events = handler->events();
	for(EventType event : events) {
		std::vector<const EventHandler *> &handlers = m_handlers[event];
		if (std::find(handlers.begin(), handlers.end(), handler) == handlers.end()) {
			handlers.push_back(handler);
		}
	}
}

void GameLogic::removeEventHandler(const EventHandler *handler)
{
	const std::set<EventType> &events = handler->events();
	for (EventType event : events) {
		std::vector<const EventHandler *> &handlers = m_handlers[event];
		auto i = std::find(handlers.begin(), handlers.end(), handler);
		if (i != handlers.end()) {
			handlers.erase(i);
		}
	}
}

void GameLogic::fire(EventType event, ServerPlayer *target, void *data)
{
	std::vector<const EventHandler *> rules;
	for (const EventHandler *rule : m_rules) {
		const std::set<EventType> &events = rule->events();
		if (events.find(event) != events.end()) {
			rules.push_back(rule);
		}
	}

	if (rules.size() > 1) {
		std::stable_sort(rules.begin(), rules.end(), [event](const EventHandler *a, const EventHandler *b){
			return a->priority(event) > b->priority(event);
		});
	}

	for (const EventHandler *rule : rules) {
		rule->effect(this, event, target, data);
	}
}

bool GameLogic::trigger(EventType event, ServerPlayer *target, void *data)
{
	std::vector<const EventHandler *> &handlers = m_handlers[event];

	std::stable_sort(handlers.begin(), handlers.end(), [event](const EventHandler *a, const EventHandler *b){
		return a->priority(event) > b->priority(event);
	});

	bool process_broken = false;
	size_t triggerable_index = 0;
	while (triggerable_index < handlers.size()) {
		int currentPriority = 0;
		std::map<ServerPlayer *, EventList> triggerable_events;

		//Construct triggerableEvents
		do {
			const EventHandler *handler = handlers.at(triggerable_index);
			if (triggerable_events.empty() || handler->priority(event) == currentPriority) {
				EventMap events = handler->triggerable(this, event, target, data);
				if (events.size() > 0) {
					std::vector<ServerPlayer *> players = this->players();
					for (ServerPlayer *p : players) {
						if (events.find(p) == events.end())
							continue;

						for (const std::pair<ServerPlayer *, Event> &e : events) {
							if (e.first == p) {
								triggerable_events[p].push_back(e.second);
								currentPriority = e.second.handler->priority(event);
							}
						}
					}
				}
			} else if (handler->priority(event) != currentPriority) {
				break;
			}
			triggerable_index++;
		} while (triggerable_index < handlers.size());

		if (!triggerable_events.empty()) {
			std::vector<ServerPlayer *> all_players = this->allPlayers(true);
			for (ServerPlayer *invoker : all_players) {
				if (triggerable_events.find(invoker) == triggerable_events.end())
					continue;

				for (;;) {
					EventList &events = triggerable_events[invoker];
					if (events.empty())
						break;

					bool has_compulsory = false;
					for (const Event &d : events) {
						if (d.handler->isCompulsory()) {
							has_compulsory = true;
							break;
						}
					}

					//Ask the invoker to determine the trigger order
					Event choice;
					if (events.size() > 1)
						choice = invoker->askForTriggerOrder(events, !has_compulsory);
					else if (has_compulsory)
						choice = events.front();
					else
						choice = invoker->askForTriggerOrder(events, true);

					//If the user selects "cancel"
					if (!choice.isValid())
						break;

					ServerPlayer *event_target = choice.to.empty() ? target : choice.to.front();

					//Ask the invoker for cost
					bool take_effect = choice.handler->onCost(this, event, event_target, data, invoker);

					//Take effect
					if (take_effect) {
						bool broken = choice.handler->effect(this, event, event_target, data, invoker);
						if (broken) {
							process_broken = true;
							break;
						}
					}

					//Remove targets that are in front of the triggered target
					for (size_t i = 0; i < events.size(); i++) {
						Event &d = events[i];
						if (d.handler != choice.handler)
							continue;

						for (ServerPlayer *to : choice.to) {
							if (d.to.back() == to) {
								events.erase(events.begin() + i);
								i--;
							} else {
								size_t index;
								size_t to_size = d.to.size();
								for (index = 0; index < to_size; index++) {
									if (d.to[index] == to) {
										break;
									}
								}
								std::vector<ServerPlayer *> rest_targets;
								rest_targets.reserve(to_size - index + 1);
								for (; index < to_size; index++) {
									rest_targets.push_back(d.to[index]);
								}
								d.to = std::move(rest_targets);
							}
						}

						if (choice.to.empty()) {
							events.erase(events.begin() + i);
							i--;
						}
					}
				}
			}
		}
	}

	return process_broken;
}

const std::vector<ServerPlayer *> &GameLogic::players() const
{
	return m_players;
}

ServerPlayer *GameLogic::findPlayer(uint id) const
{
	for (ServerPlayer *target : m_players) {
		if (target->id() == id) {
			return target;
		}
	}
	return nullptr;
}

std::vector<ServerPlayer *> GameLogic::allPlayers(bool include_dead) const
{
	std::vector<ServerPlayer *> players = this->players();
	ServerPlayer *current = currentPlayer();
	if (current == nullptr)
		return players;

	std::sort(players.begin(), players.end(), [](const ServerPlayer *a, const ServerPlayer *b){
		return a->seat() < b->seat();
	});

	size_t current_index = -1;
	for (int i = 0; i < players.size(); i++) {
		if (players[i] == current) {
			current_index = i;
			break;
		}
	}
	if (current_index == -1)
		return players;

	std::vector<ServerPlayer *> all_players;
	for (size_t i = current_index; i < players.size(); i++) {
		if (include_dead || players.at(i)->isAlive())
			all_players.push_back(players.at(i));
	}
	for (size_t i = 0; i < current_index; i++) {
		if (include_dead || players.at(i)->isAlive())
			all_players.push_back(players.at(i));
	}

	if (current->phase() == PlayerPhase::Inactive) {
		auto i = std::find(all_players.begin(), all_players.end(), current);
		if (i != all_players.end()) {
			all_players.erase(i);
			all_players.push_back(current);
		}
	}

	return all_players;
}

std::vector<ServerPlayer *> GameLogic::otherPlayers(ServerPlayer *except, bool include_dead) const
{
	std::vector<ServerPlayer *> players = allPlayers(include_dead);
	if (except && (except->isAlive() || include_dead)) {
		auto i = std::find(players.begin(), players.end(), except);
		if (i != players.end()) {
			players.erase(i);
		}
	}
	return players;
}

int GameLogic::countPlayer(bool include_dead) const
{
	if (!include_dead) {
		return countPlayer([] (ServerPlayer *player) {
			return player->isAlive();
		});
	} else {
		return static_cast<int>(m_players.size());
	}
}

int GameLogic::countPlayer(bool(*filter)(ServerPlayer *)) const
{
	int count = 0;
	for (ServerPlayer *player : m_players) {
		if (filter(player)) {
			count++;
		}
	}
	return count;
}

void GameLogic::sortByActionOrder(std::vector<ServerPlayer *> &players) const
{
	std::map<ServerPlayer *, size_t> action_order;
	for (ServerPlayer *player : players) {
		action_order[player] = -1;
	}

	std::vector<ServerPlayer *> all_players = this->allPlayers(true);
	for (size_t i = 0, max = all_players.size(); i < max; i++) {
		action_order[all_players[i]] = i;
	}

	std::sort(all_players.begin(), all_players.end(), [&action_order](ServerPlayer *a, ServerPlayer *b){
		return action_order.at(a) < action_order.at(b);
	});
}

const Card *GameLogic::getDrawPileCard()
{
	if (m_drawPile->size() < 1)
		reshuffleDrawPile();
	return m_drawPile->first();
}

std::vector<const Card *> GameLogic::getDrawPileCards(int n)
{
	if (m_drawPile->size() < n)
		reshuffleDrawPile();
	return m_drawPile->first(n);
}

void GameLogic::reshuffleDrawPile()
{
	if (m_discardPile->size() <= 0) {
		//@to-do: stand off. Game over.
	}

	m_reshufflingCount++;

	//@to-do: check reshuffling count limit
	/*if (limit > 0 && times == limit)
		gameOver(".");*/

	std::vector<const Card *> new_cards;
	std::deque<const Card *> &old_cards = m_discardPile->cards();
	for (const Card *card : old_cards) {
		new_cards.push_back(card);
	}
	m_discardPile->clear();
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(new_cards.begin(), new_cards.end(), g);
	for (const Card *card : new_cards) {
		m_cardPosition[card] = m_drawPile;
	}
	m_drawPile->add(new_cards, CardMoveDirection::Bottom);
}

void GameLogic::moveCards(const CardsMoveStruct &move)
{
	std::vector<CardsMoveStruct> moves;
	moves.push_back(move);
	moveCards(moves);
}

void GameLogic::moveCards(std::vector<CardsMoveStruct> &moves)
{
	filterCardsMove(moves);
	void *move_data = static_cast<void *>(&moves);
	std::vector<ServerPlayer *> all_players = this->allPlayers();
	for (ServerPlayer *player : all_players)
		trigger(BeforeCardsMove, player, move_data);

	filterCardsMove(moves);
	all_players = this->allPlayers();
	for (ServerPlayer *player : all_players)
		trigger(CardsMove, player, move_data);

	filterCardsMove(moves);
	for (size_t i = 0 ; i < moves.size(); i++) {
		const CardsMoveStruct &move = moves.at(i);
		CardArea *to = findArea(move.to);
		if (to == nullptr)
			continue;

		CardArea *from = findArea(move.from);
		if (from == nullptr)
			continue;

		for (const Card *card : move.cards) {
			if (from != m_cardPosition.at(card))
				continue;
			if (from->remove(card)) {
				to->add(card, move.to.direction);
				m_cardPosition[card] = to;
			}
		}
	}

	std::vector<ServerPlayer *> viewers = players();
	for (ServerPlayer *viewer : viewers) {
		JsonArray data;
		for (const CardsMoveStruct &move : moves)
			data.push_back(move.toJson(move.isRelevant(viewer)));
		viewer->notify(cmd::MoveCards, data);
	}

	all_players = this->allPlayers();
	for (ServerPlayer *player : all_players)
		trigger(AfterCardsMove, player, move_data);
}

bool GameLogic::useCard(CardUseStruct &use)
{
	if (use.card == nullptr || use.from == nullptr)
		return false;

	//Initialize isHandcard
	use.isHandCard = true;
	std::vector<const Card *> real_cards = use.card->realCards();
	for (const Card *card : real_cards) {
		CardArea *area = m_cardPosition[card];
		if (area == nullptr || area->owner() != use.from || area->type() != CardAreaType::Hand) {
			use.isHandCard = false;
			break;
		}
	}

	if (use.from->phase() == PlayerPhase::Play && use.addHistory)
		use.from->addCardHistory(use.card->name());

	try {
		use.card->onUse(this, use);

		void *data = static_cast<void *>(&use);
		trigger(CardUsed, use.from, data);

		if (use.from) {
			trigger(TargetChoosing, use.from, data);

			JsonObject args;
			args["from"] = use.from->id();
			//args["cards"]
			JsonArray tos;
			for (ServerPlayer *to : use.to)
				tos.push_back(to->id());
			args["to"] = tos;
			broadcastNotification(cmd::UseCard, args);

			if (use.from) {
				if (!use.to.empty() || use.target) {
					std::vector<ServerPlayer *> targets = use.to;
					for (ServerPlayer *to : targets) {
						trigger(TargetConfirming, to, data);
					}

					if (use.from && (!use.to.empty() || use.target)) {
						trigger(TargetChosen, use.from, data);

						if (use.from && !use.to.empty()) {
							std::vector<ServerPlayer *> targets = use.to;
							for (ServerPlayer *to : targets) {
								trigger(TargetConfirmed, to, data);
							}
							use.card->use(this, use);
						}
					}
				} else if (use.target) {
					use.card->use(this, use);
				}
			}
		}

		trigger(CardFinished, use.from, data);

	} catch (EventType e) {
		//@to-do: handle TurnBroken and StageChange
		throw e;
	}

	return true;
}

bool GameLogic::takeCardEffect(CardEffectStruct &effect)
{
	void *data = static_cast<void *>(&effect);
	bool canceled = false;
	if (effect.to) {
		if (effect.to->isAlive()) {
			canceled = trigger(CardEffect, effect.to, data);
			if (!canceled) {
				canceled = trigger(CardEffected, effect.to, data);
				if (!canceled) {
					effect.use.card->onEffect(this, effect);
					if (effect.to->isAlive() && !effect.isNullified())
						effect.use.card->effect(this, effect);
				}
			}
		}
	} else if (effect.use.target) {
		effect.use.card->onEffect(this, effect);
		if (!effect.isNullified())
			effect.use.card->effect(this, effect);
	}
	trigger(PostCardEffected, effect.to, data);
	return !canceled;
}

bool GameLogic::respondCard(CardResponseStruct &response)
{
	CardsMoveStruct move;
	move.cards.push_back(response.card);
	move.to.type = CardAreaType::Table;
	move.isOpen = true;
	moveCards(move);

	void *data = static_cast<void *>(&response);
	bool broken = trigger(CardResponded, response.from, data);

	if (response.card && m_table->contains(response.card)) {
		CardsMoveStruct move;
		move.cards.push_back(response.card);
		move.to.type = CardAreaType::DiscardPile;
		move.isOpen = true;
		moveCards(move);
	}

	return !broken;
}

void GameLogic::judge(JudgeStruct &judge)
{
	void *data = static_cast<void *>(&judge);

	if (trigger(StartJudge, judge.who, data))
		return;

	judge.card = getDrawPileCard();
	judge.matched = judge.pattern.match(judge.who, judge.card);

	CardsMoveStruct move;
	move.cards.push_back(judge.card);
	move.to.type = CardAreaType::Judge;
	move.to.owner = judge.who;
	move.isOpen = true;
	moveCards(move);

	std::vector<ServerPlayer *> players = allPlayers();
	for (ServerPlayer *player : players) {
		if (trigger(AskForRetrial, player, data))
			break;
	}
	trigger(FinishRetrial, judge.who, data);
	trigger(FinishJudge, judge.who, data);

	const CardArea *judgeCards = judge.who->judgeCards();
	if (judgeCards->contains(judge.card)) {
		CardsMoveStruct move;
		move.cards.push_back(judge.card);
		move.to.type = CardAreaType::DiscardPile;
		move.isOpen = true;
		moveCards(move);
	}
}

std::vector<const Card *> GameLogic::findCards(const KA_IMPORT Json &data)
{
	std::vector<const Card *> cards;
	if (data.isArray()) {
		JsonArray data_list = data.toArray();
		for (const Json &card_id : data_list) {
			const Card *card = findCard(card_id.toUInt());
			if (card)
				cards.push_back(card);
		}
	}
	return cards;
}

void GameLogic::damage(DamageStruct &damage)
{
	if (damage.to == nullptr || damage.to->isDead())
		return;

	void *data = static_cast<void *>(&damage);
	if (!damage.chain && !damage.transfer) {
		trigger(ConfirmDamage, damage.from, data);
	}

	if (trigger(BeforeDamage, damage.from, data))
		return;

	try {
		do {
			if (trigger(DamageStart, damage.to, data))
				break;

			if (damage.from && trigger(Damaging, damage.from, data))
				break;

			if (damage.to && trigger(Damaged, damage.to, data))
				break;

			if (damage.to)
				trigger(BeforeHpReduced, damage.to, data);

			if (damage.to) {
				JsonArray arg = {damage.to->id(), damage.nature, damage.damage};
				broadcastNotification(cmd::Damage, arg);

				int new_hp = damage.to->hp() - damage.damage;
				damage.to->setHp(new_hp);
				damage.to->broadcastProperty("hp", new_hp);

				trigger(AfterHpReduced, damage.to, data);
			}

			if (damage.from)
				trigger(AfterDamaging, damage.from, data);

			if (damage.to)
				trigger(AfterDamaged, damage.to, data);

		} while (false);

		if (damage.to)
			trigger(DamageComplete, damage.to, data);

	} catch (EventType e) {
		//@to-do: handle TurnBroken and StageChange
		throw e;
	}
}

void GameLogic::loseHp(ServerPlayer *victim, int lose)
{
	if (lose <= 0 || victim->isDead())
		return;

	void *data = static_cast<void *>(&lose);
	if (trigger(HpLost, victim, data))
		return;

	if (lose <= 0)
		return;

	int new_hp = victim->hp() - lose;
	victim->setHp(new_hp);
	victim->broadcastProperty("hp", new_hp);

	JsonObject arg;
	arg["victimId"] = victim->id();
	arg["loseHp"] = lose;
	broadcastNotification(cmd::LoseHp, arg);

	trigger(AfterHpReduced, victim, data);
	trigger(AfterHpLost, victim, data);
}

void GameLogic::recover(RecoverStruct &recover)
{
	if (recover.to == nullptr || recover.to->lostHp() == 0 || recover.to->isDead())
		return;

	void *data = static_cast<void *>(&recover);
	if (trigger(BeforeRecover, recover.to, data))
		return;
	if (recover.to == nullptr)
		return;

	int new_hp = std::min(recover.to->hp() + recover.recover, recover.to->maxHp());
	recover.to->setHp(new_hp);
	recover.to->broadcastProperty("hp", new_hp);

	JsonObject arg;
	arg["from"] = recover.from ? recover.from->id() : 0;
	arg["to"] = recover.to->id();
	arg["num"] = recover.recover;
	broadcastNotification(cmd::Recover, arg);

	trigger(AfterRecover, recover.to, data);
}

void GameLogic::killPlayer(ServerPlayer *victim, DamageStruct *damage)
{
	victim->setAlive(false);
	victim->broadcastProperty("alive", false);
	victim->broadcastProperty("role", victim->role());

	DeathStruct death;
	death.who = victim;
	death.damage = damage;
	void *data = static_cast<void *>(&death);

	trigger(BeforeGameOverJudge, victim, data);
	trigger(GameOverJudge, victim, data);

	trigger(Died, victim, data);
	trigger(BuryVictim, victim, data);
}

void GameLogic::gameOver(const std::vector<ServerPlayer *> &winners)
{
	JsonArray data;
	for (ServerPlayer *winner : winners)
		data.push_back(winner->id());
	broadcastNotification(cmd::GameOver, data);
	throw GameFinish;
}

std::map<uint, std::vector<const General *>> GameLogic::broadcastRequestForGenerals(const std::vector<ServerPlayer *> &players, int num, int limit)
{
	GeneralList generals;
	std::vector<const Package *> packages = this->packages();
	for (const Package *package : packages) {
		std::vector<const General *> package_generals = package->generals();
		generals.reserve(generals.size() + package_generals.size());
		for (const General *general : package_generals) {
			generals.push_back(general);
		}
	}

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(generals.begin(), generals.end(), g);

	size_t min_candidate_num = limit * players.size();
	size_t general_num = generals.size();
	while (min_candidate_num > generals.size()) {
		for (size_t i = 0, max = min_candidate_num - generals.size(); i < max; i++) {
			generals.push_back(generals.at(i % general_num));
		}
	}

	std::map<ServerPlayer *, GeneralList> playerCandidates;

	for (ServerPlayer *player : players) {
		GeneralList candidates;
		std::copy(generals.begin() + (player->seat() - 1) * limit, generals.begin() + limit, candidates.begin());
		playerCandidates[player] = candidates;

		JsonArray candidate_data;
		for (const General *general : candidates)
			candidate_data.push_back(general->id());

		JsonArray banned_pair_data;
		//@todo: load banned pairs

		JsonObject data;
		data["num"] = num;
		data["candidates"] = candidate_data;
		data["banned"] = banned_pair_data;

		player->prepareRequest(cmd::ChooseGeneral, data, m_config->timeout * 1000);
	}

	broadcastRequest(players);

	std::map<uint, GeneralList> result;
	for (ServerPlayer *player : players) {
		const GeneralList &candidates = playerCandidates[player];

		GeneralList generals;
		User *user = player->user();
		if (user) {
			Json reply_data = user->getReply();
			if (reply_data.isArray()) {
				const JsonArray &reply = reply_data.toArray();
				for (const Json &choice : reply) {
					uint id = choice.toUInt();
					for(const General *general : candidates) {
						if (general->id() == id) {
							generals.push_back(general);
							break;
						}
					}
				}
			}
		}

		//@to-do: handle banned pairs
		if (generals.size() < num) {
			generals.resize(num);
			std::copy(candidates.begin(), candidates.begin() + num, generals.begin());
		}

		result[player->id()] = generals;
	}

	return result;
}

void GameLogic::broadcastNotification(cmd command)
{
	Room *room = this->room();
	if (room) {
		room->broadcastNotification(static_cast<int>(command));
	}
}

void GameLogic::broadcastNotification(cmd command, const KA_IMPORT Json &arguments)
{
	Room *room = this->room();
	if (room) {
		room->broadcastNotification(static_cast<int>(command), arguments);
	}
}

void GameLogic::broadcastNotification(cmd command, const KA_IMPORT Json &arguments, ServerPlayer *except)
{
	Room *room = this->room();
	if (room) {
		room->broadcastNotification(static_cast<int>(command), arguments, except->user());
	}
}

void GameLogic::broadcastRequest(const std::vector<ServerPlayer *> &players)
{
	std::vector<User *> users;
	users.reserve(players.size());
	for (ServerPlayer *player : players) {
		users.push_back(player->user());
	}
	room()->broadcastRequest(users);
}

void GameLogic::loadMode(const GameMode *mode)
{
	const std::vector<const EventHandler *> &rules = mode->rules();
	m_rules.reserve(m_rules.size() + rules.size());
	for (const EventHandler *rule : rules) {
		addEventHandler(rule);
		m_rules.push_back(rule);
	}

	Engine *engine = Engine::instance();
	setPackages(engine->getPackages(mode));
}

void GameLogic::prepareToStart()
{
	Room *room = this->room();

	//Load game mode
	Engine *engine = Engine::instance();
	const GameMode *mode = engine->mode(m_config->mode);
	loadMode(mode);

	//Arrange seats for all the players
	std::vector<ServerPlayer *> players = this->players();
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(players.begin(), players.end(), g);
	int player_num = static_cast<int>(players.size());
	for (int i = 1; i < player_num; i++) {
		players[i - 1]->setSeat(i);
		players[i - 1]->setNext(players.at(i));
	}
	ServerPlayer *last_player = players.back();
	last_player->setSeat(static_cast<int>(players.size()));
	last_player->setNext(players.front());
	setCurrentPlayer(players.front());

	JsonArray player_list;
	for (ServerPlayer *player : players) {
		User *user = player->user();
		JsonObject info;
		info["user_id"] = user->id();
		info["player_id"] = player->id();
		player_list.push_back(info);
	}
	broadcastNotification(cmd::ArrangeSeat, player_list);

	//Import packages
	GeneralList generals;
	for (const Package *package : m_packages) {
		GeneralList package_generals = package->generals();
		size_t offset = generals.size();
		generals.resize(offset + package_generals.size());
		std::copy(package_generals.begin(), package_generals.end(), generals.begin() + offset);

		std::vector<const Card *> cards = package->cards();
		for (const Card *card : cards) {
			m_cards[card->id()] = card;
		}
	}

	//Prepare cards
	JsonArray card_data;
	for (const std::pair<uint, const Card *> &p : m_cards) {
		card_data.push_back(p.second->id());
	}
	broadcastNotification(cmd::PrepareCards, card_data);

	for (const std::pair<uint, const Card *> &p : m_cards) {
		m_drawPile->add(p.second);
		m_cardPosition[p.second] = m_drawPile;
	}

	std::deque<const Card *> &cards = m_drawPile->cards();
	std::shuffle(cards.begin(), cards.end(), g);

	fire(PrepareToStart);
}

CardArea *GameLogic::findArea(const CardsMoveStruct::Area &area)
{
	if (area.owner) {
		switch (area.type) {
		case CardAreaType::Hand: {
			ServerPlayer *owner = findPlayer(area.owner->id());
			return owner->handcardArea();
		}
		case CardAreaType::Equip:
			return area.owner->equipArea();
		case CardAreaType::DelayedTrick:
			return area.owner->delayedTrickArea();
		case CardAreaType::Judge:
			return area.owner->judgeCards();
		default:
			std::cerr << "Owner Area Not Found" << std::endl;
		}
	} else {
		switch (area.type) {
		case CardAreaType::DrawPile:
			return m_drawPile;
		case CardAreaType::DiscardPile:
			return m_discardPile;
		case CardAreaType::Table:
			return m_table;
		case CardAreaType::Wugu:
			return m_wugu;
		case CardAreaType::Unknown:
			return nullptr;
		default:
			std::cerr << "Global Area Not Found" << std::endl;
		}
	}
	return nullptr;
}

void GameLogic::filterCardsMove(std::vector<CardsMoveStruct> &moves)
{
	//Fill card source information
	for (size_t i = 0, maxi = moves.size(); i < maxi; i++) {
		CardsMoveStruct &move = moves[i];

		CardArea *destination = findArea(move.to);
		for (const Card *card : move.cards) {
			if (card->isVirtual()) {
				std::vector<const Card *> real_cards = card->realCards();
				auto i = std::find(move.cards.begin(), move.cards.end(), card);
				if (i != move.cards.end()) {
					move.cards.erase(i);
				}

				size_t offset = move.cards.size();
				move.cards.resize(offset + real_cards.size());
				std::copy(real_cards.begin(), real_cards.end(), move.cards.begin() + offset);

				std::map<const Card *, CardArea *>::iterator iter = m_cardPosition.find(card);
				if (iter != m_cardPosition.end()) {
					CardArea *source = iter->second;
					if (source) {
						source->remove(card);
					}
					m_cardPosition.erase(card);
				}

				if (destination->add(card)) {
					m_cardPosition[card] = destination;
				}
			}
		}

		if (move.from.type != CardAreaType::Unknown)
			continue;

		std::map<CardArea *, std::vector<const Card *>> cardSource;
		for (const Card *card : move.cards) {
			CardArea *from = m_cardPosition[card];
			if (from == nullptr)
				continue;
			cardSource[from].push_back(card);
		}

		for (const std::pair<CardArea *, std::vector<const Card *>> &iter : cardSource) {
			CardArea *from = iter.first;
			CardsMoveStruct submove;
			submove.from.type = from->type();
			submove.from.owner = dynamic_cast<ServerPlayer *>(from->owner());
			submove.from.ownerId = from->owner()->id();
			submove.from.name = from->name();
			submove.cards = iter.second;
			submove.to = move.to;
			submove.isOpen = move.isOpen;
			moves.push_back(submove);
		}

		moves.erase(moves.begin() + i);
		i--;
		maxi--;
	}
}

void GameLogic::run()
{
	prepareToStart();

	//@to-do: Turn broken event
	std::vector<ServerPlayer *> allPlayers = this->allPlayers();
	for (ServerPlayer *player : allPlayers) {
		trigger(GameStart, player);
	}

	for (;;) {
		try {
			ServerPlayer *current = currentPlayer();
			for (;;) {
				if (current->seat() == 1)
					m_round++;
				if (current->isDead()) {
					current = dynamic_cast<ServerPlayer *>(current->next());
					continue;
				}

				setCurrentPlayer(current);
				trigger(TurnStart, current);
				current = dynamic_cast<ServerPlayer *>(current->next());

				while (!m_extraTurns.empty()) {
					ServerPlayer *extra = m_extraTurns.front();
					m_extraTurns.erase(m_extraTurns.begin());
					setCurrentPlayer(extra);
					trigger(TurnStart, extra);
				}
			}
		} catch (EventType event) {
			if (event == GameFinish) {
				return;
			} else if (event == TurnBroken) {
				ServerPlayer *current = currentPlayer();
				trigger(TurnBroken, current);
				ServerPlayer *next = dynamic_cast<ServerPlayer *>(current->nextAlive(1, false));
				if (current->phase() != PlayerPhase::Inactive) {
					fire(PhaseEnd, current);
					current->setPhase(PlayerPhase::Inactive);
					current->broadcastProperty("phase", current->phaseString());
				}
				setCurrentPlayer(next);
			}
		}
	}
}
