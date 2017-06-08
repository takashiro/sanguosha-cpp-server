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

#include "ServerPlayer.h"

#include "Card.h"
#include "CardArea.h"
#include "CardPattern.h"
#include "Event.h"
#include "EventHandler.h"
#include "GameLogic.h"
#include "General.h"
#include "Skill.h"
#include "SkillArea.h"

#include <User.h>
#include <Json.h>

#include <algorithm>

KA_USING_NAMESPACE

ServerPlayer::ServerPlayer(GameLogic *logic, KA_IMPORT User *user)
	: Player(user->id())
	, m_logic(logic)
	, m_user(user)
{
}

ServerPlayer::~ServerPlayer()
{
}

void ServerPlayer::drawCards(int n)
{
	CardsMoveStruct move;
	move.from.type = CardAreaType::DrawPile;
	move.from.direction = CardMoveDirection::Top;
	move.to.type = CardAreaType::Hand;
	move.to.owner = this;
	move.cards = m_logic->getDrawPileCards(n);

	m_logic->moveCards(move);
}

void ServerPlayer::recastCard(Card *card)
{
	CardsMoveStruct recast;
	recast.cards.push_back(card);
	recast.to.type = CardAreaType::Table;
	recast.isOpen = true;
	m_logic->moveCards(recast);

	const CardArea *table = m_logic->table();
	if (table->contains(card)) {
		CardsMoveStruct discard;
		discard.cards.push_back(card);
		discard.to.type = CardAreaType::DiscardPile;
		discard.isOpen = true;
		m_logic->moveCards(discard);
	}

	drawCards(1);
}

void ServerPlayer::showCard(Card *card)
{
	JsonArray card_data;
	card_data.push_back(card->id());

	JsonObject data;
	data["from"] = id();
	data["cards"] = card_data;
	m_logic->broadcastNotification(cmd::ShowCard, data);
}

void ServerPlayer::showCards(const std::vector<Card *> &cards)
{
	JsonArray card_data;
	for(Card *card : cards)
		card_data.push_back(card->id());

	JsonObject data;
	data["from"] = id();
	data["cards"] = card_data;
	notify(cmd::ShowCard, data);
}

void ServerPlayer::play()
{
	std::vector<Phase> phases;
	phases.reserve(7);
	phases.push_back(Phase::RoundStart);
	phases.push_back(Phase::Start);
	phases.push_back(Phase::Judge);
	phases.push_back(Phase::Draw);
	phases.push_back(Phase::Play);
	phases.push_back(Phase::Discard);
	phases.push_back(Phase::Finish);
	play(phases);
}

void ServerPlayer::play(const std::vector<Player::Phase> &phases)
{
	PhaseChangeStruct change;
	for(Phase to : phases) {
		if (to == Phase::Inactive)
			break;
		change.from = phase();
		change.to = to;

		void *data = static_cast<void *>(&change);
		bool skip = m_logic->trigger(PhaseChanging, this, data);

		setPhase(change.to);
		broadcastProperty("phase", static_cast<int>(to));

		if ((skip || isPhaseSkipped(change.to)) && !m_logic->trigger(PhaseSkipping, this, data))
			continue;

		if (!m_logic->trigger(PhaseStart, this))
			m_logic->trigger(PhaseProceeding, this);
		m_logic->trigger(PhaseEnd, this);
	}

	change.from = phase();
	change.to = Phase::Inactive;

	void *data = static_cast<void *>(&change);
	m_logic->trigger(PhaseChanging, this, data);

	setPhase(change.to);
	broadcastProperty("phase", static_cast<int>(change.to));

	clearSkippedPhase();
}

bool ServerPlayer::activate()
{
	int timeout = 15 * 1000;
	Json reply_data = request(cmd::Act, Json(), timeout);
	if (!reply_data.isObject())
		return true;
	const JsonObject &reply = reply_data.toObject();
	if (reply.empty())
		return true;

	std::vector<ServerPlayer *> targets;
	if (reply.find("to") != reply.end()) {
		const Json &tos_data = reply.at("to");
		if (tos_data.isArray()) {
			const JsonArray &tos = tos_data.toArray();
			for (const Json &to : tos) {
				uint toId = to.toUInt();
				ServerPlayer *target = m_logic->findPlayer(toId);
				if (target)
					targets.push_back(target);
			}
		}
	}

	std::vector<Card *> cards;
	if (reply.find("cards") != reply.end()) {
		cards = m_logic->findCards(reply.at("cards"));
	}

	const Skill *skill = nullptr;
	if (reply.find("skill_id") != reply.end()) {
		uint skill_id = reply.at("skill_id").toUInt();
		if (skill_id)
			skill = getSkill(skill_id);
	}

	Card *card = nullptr;
	if (skill) {
		//@TO-DO: invoke skill
	} else {
		card = cards.size() > 0 ? cards.front() : nullptr;
	}

	std::vector<const Player *> const_targets;
	for (const Player *target : targets) {
		const_targets.push_back(target);
	}

	if (card != nullptr) {
		if (card->canRecast() && targets.empty()) {
			recastCard(card);
		} else if (card->isAvailable(this) && card->isValid(const_targets, this)) {
			CardUseStruct use;
			use.from = this;
			use.to = targets;
			use.card = card;
			m_logic->useCard(use);
		}
		return false;
	} else {
		return true;
	}
}

void ServerPlayer::showPrompt(const std::string &message, int number)
{
	JsonArray data;
	data.push_back(message);
	data.push_back(number);
	notify(cmd::ShowPrompt, data);
}

void ServerPlayer::showPrompt(const std::string &message, const KA_IMPORT JsonArray &args)
{
	JsonArray data;
	data.push_back(message);
	data.push_back(args);
	notify(cmd::ShowPrompt, data);
}

void ServerPlayer::showPrompt(const std::string &message, const Card *card)
{
	JsonArray args;
	args.push_back("card");
	args.push_back(card->id());
	showPrompt(message, args);
}

void ServerPlayer::showPrompt(const std::string &message, const ServerPlayer *from, const Card *card)
{
	JsonArray args;
	args.push_back("player");
	args.push_back(from->id());
	if (card) {
		args.push_back("card");
		args.push_back(card->id());
	}
	showPrompt(message, args);
}

void ServerPlayer::showPrompt(const std::string &message, const ServerPlayer *p1, const ServerPlayer *p2, const Card *card)
{
	JsonArray args;
	args.push_back("player");
	args.push_back(p1->id());
	args.push_back("player");
	args.push_back(p2->id());
	if (card) {
		args.push_back("card");
		args.push_back(card->id());
	}
	showPrompt(message, args);
}

Event ServerPlayer::askForTriggerOrder(const EventList &options, bool cancelable)
{
	JsonObject data;
	data["cancelable"] = cancelable;

	JsonArray optionData;
	for (const Event &e : options) {
		JsonObject eventData;
		eventData["name"] = e.handler->name();
		JsonArray targetData;
		for (ServerPlayer *to : e.to)
			targetData.push_back(to->id());
		eventData["to"] = targetData;
		optionData.push_back(eventData);
	}
	data["options"] = optionData;

	int timeout = 15 * 1000;
	Json replyData = request(cmd::TriggerOrder, data, timeout);
	if (replyData.isNull())
		return cancelable ? Event() : options.front();

	uint eventId = replyData.toUInt();
	if (eventId < options.size())
		return options.at(eventId);

	return cancelable ? Event() : options.front();
}

Card *ServerPlayer::askForCard(const std::string &pattern, bool optional)
{
	JsonObject data;
	data["pattern"] = pattern;
	data["optional"] = optional;

	int timeout = 15 * 1000;
	Json replyData;
	for (;;) {
		replyData = request(cmd::AskForCard, data, timeout);
		if (replyData.isNull())
			break;

		const JsonObject &reply = replyData.toObject();
		std::vector<Card *> cards = m_logic->findCards(reply.at("cards"));
		uint skillId = reply.at("skillId").toUInt();
		if (skillId) {
			//@TODO: filter cards
		}
		if (cards.size() != 1)
			break;

		Card *card = cards.front();
		CardPattern p(pattern);
		if (p.match(this, card))
			return card;
	}

	if (!optional) {
		CardPattern p(pattern);

		const std::deque<Card *> &handcards = handcardArea()->cards();
		for (Card *card : handcards) {
			if (p.match(this, card))
				return card;
		}

		const std::deque<Card *> &equips = equipArea()->cards();
		for (Card *card : equips) {
			if (p.match(this, card))
				return card;
		}
	}

	return nullptr;
}

std::vector<Card *> ServerPlayer::askForCards(const std::string &pattern, int num, bool optional)
{
	return askForCards(pattern, num, num, optional);
}

std::vector<Card *> ServerPlayer::askForCards(const std::string &pattern, int minNum, int maxNum, bool optional)
{
	if (maxNum < minNum)
		maxNum = minNum;

	JsonObject data;
	data["pattern"] = pattern;
	data["minNum"] = minNum;
	data["maxNum"] = maxNum;
	data["optional"] = optional;

	std::set<Card *> cards;

	int timeout = 15 * 1000;
	Json reply = request(cmd::AskForCard, data, timeout);
	if (reply.isObject()) {
		const JsonObject &reply_data = reply.toObject();
		if (reply_data.find("cards") != reply_data.end()) {
			std::vector<Card *> all_cards = m_logic->findCards(reply_data.at("cards"));
			CardPattern p(pattern);
			for (Card *card : all_cards) {
				if (p.match(this, card)) {
					cards.insert(card);
				}
			}
		}
	}

	if (!optional) {
		if (cards.size() < minNum) {
			std::vector<Card *> all_cards;
			const std::deque<Card *> &hand_cards = handcardArea()->cards();
			const std::deque<Card *> &equips = equipArea()->cards();
			all_cards.reserve(hand_cards.size() + equips.size());
			for (Card *card : hand_cards)
				all_cards.push_back(card);
			for (Card *card : equips)
				all_cards.push_back(card);

			CardPattern p(pattern);
			for (Card *card : all_cards) {
				if (p.match(this, card)) {
					cards.insert(card);
					if (cards.size() >= minNum)
						break;
				}
			}
		}
	}

	std::vector<Card *> result;
	for (Card *card : cards) {
		result.push_back(card);
	}
	return result;
}

Card *ServerPlayer::askToChooseCard(ServerPlayer *owner, const std::string &areaFlag, bool handcardVisible)
{
	const CardArea *hand_card_area = owner->handcardArea();
	const CardArea *equip_area = owner->equipArea();
	const CardArea *delayed_trick_area = owner->delayedTrickArea();

	JsonObject data;

	if (areaFlag.find('h') != std::string::npos) {
		JsonArray handcard_data;
		if (handcardVisible) {
			const std::deque<Card *> &cards = hand_card_area->cards();
			for (const Card *card : cards)
				handcard_data.push_back(card->id());
			data["handcards"] = handcard_data;
		} else {
			data["handcards"] = owner->handcardNum();
		}
	}

	if (areaFlag.find('e') != std::string::npos) {
		JsonArray equip_data;
		const std::deque<Card *> &cards = equip_area->cards();
		for (const Card *card : cards)
			equip_data.push_back(card->id());
		data["equips"] = equip_data;
	}

	if (areaFlag.find('j') != std::string::npos) {
		JsonArray trick_data;
		const std::deque<Card *> &cards = delayed_trick_area->cards();
		for (const Card *card : cards)
			trick_data.push_back(card->id());
		data["delayedTricks"] = trick_data;
	}

	int timeout = 15 * 1000;
	uint card_id = request(cmd::ChoosePlayerCard, data, timeout).toUInt();
	if (card_id > 0) {
		if (areaFlag.find('h') != std::string::npos  && handcardVisible) {
			Card *card = hand_card_area->findCard(card_id);
			if (card)
				return card;
		}
		if (areaFlag.find('e') != std::string::npos) {
			Card *card = equip_area->findCard(card_id);
			if (card)
				return card;
		}
		if (areaFlag.find('j') != std::string::npos) {
			Card *card = delayed_trick_area->findCard(card_id);
			if (card)
				return card;
		}
	}

	if (areaFlag.find('h') != std::string::npos && hand_card_area->size() > 0)
		return hand_card_area->rand();

	if (areaFlag.find('e') != std::string::npos  && equip_area->size() > 0)
		return equip_area->rand();

	if (areaFlag.find('j') != std::string::npos  && delayed_trick_area->size() > 0)
		return delayed_trick_area->rand();

	return nullptr;
}

bool ServerPlayer::askToUseCard(const std::string &pattern, const std::vector<ServerPlayer *> &assignedTargets)
{
	JsonObject data;
	data["pattern"] = pattern;

	JsonArray target_ids;
	for (ServerPlayer *target : assignedTargets)
		target_ids.push_back(target->id());
	data["assignedTargets"] = target_ids;

	int timeout = 15 * 1000;
	const Json reply = request(cmd::Act, data, timeout);

	CardUseStruct use;
	if (!reply.isObject())
		return false;

	std::vector<const Player *> targets;
	const JsonArray &tos = reply["to"].toArray();
	for (const Json &to : tos) {
		uint to_id = to.toUInt();
		ServerPlayer *target = m_logic->findPlayer(to_id);
		if (target) {
			targets.push_back(target);
			use.to.push_back(target);
		}
	}

	std::vector<Card *> cards = m_logic->findCards(reply["cards"]);

	const Skill *skill = nullptr;
	uint skill_id = reply["skill_id"].toUInt();
	if (skill_id)
		skill = getSkill(skill_id);

	Card *card = nullptr;
	if (skill) {
		/*if (skill->type() == Skill::ViewAsType) {
			if (skill->subtype() == ViewAsSkill::ProactiveType) {
				const ProactiveSkill *proactiveSkill = static_cast<const ProactiveSkill *>(skill);
				proactiveSkill->effect(m_logic, this, targets, cards);
				return true;
			} else if (skill->subtype() == ViewAsSkill::ConvertType) {
				const ViewAsSkill *viewAsSkill = static_cast<const ViewAsSkill *>(skill);
				if (viewAsSkill->isValid(cards, this, pattern))
					card = viewAsSkill->viewAs(cards, this);
			}
		}*/
	} else {
		card = cards.size() > 0 ? cards.front() : nullptr;
	}

	if (card == nullptr)
		return false;

	use.from = this;
	use.card = card;
	for (ServerPlayer *target : assignedTargets) {
		bool contained = false;
		for (ServerPlayer *current : use.to) {
			if (current == target) {
				contained = true;
				break;
			}
		}
		if (!contained)
			return false;
	}

	if (!card->isValid(targets, this))
		return false;

	return m_logic->useCard(use);
}

std::vector<std::vector<Card *>> ServerPlayer::askToArrangeCard(const std::vector<Card *> &cards, const std::vector<int> &capacities, const std::vector<std::string> &areaNames)
{
	JsonObject data;

	JsonArray capacity_data;
	for (int capacity : capacities)
		capacity_data.push_back(capacity);
	data["capacities"] = capacity_data;

	JsonArray card_data;
	for (const Card *card : cards)
		card_data.push_back(card->id());
	data["cards"] = card_data;

	JsonArray area_data;
	for (const std::string &name : areaNames) {
		area_data.push_back(area_data);
	}
	data["areaNames"] = area_data;

	std::vector<std::vector<Card *>> result;

	int timeout = 15 * 1000;
	const Json reply = request(cmd::ArrangeCard, data, timeout * 3);
	if (reply.isArray()) {
		int maxi = static_cast<int>(std::min(capacities.size(), reply.size()));
		for (int i = 0; i < maxi; i++) {
			const Json card_data = reply.at(i);
			std::vector<Card *> found = Card::Find(cards, card_data);
			int capacity = capacities.at(i);
			std::vector<Card *> add;
			add.reserve(capacity);
			for (int c = 0; c < capacity; c++) {
				add.push_back(found.at(i));
			}
			result.push_back(std::move(add));
		}
	}

	return result;
}

std::string ServerPlayer::askForOption(const std::vector<std::string> &options)
{
	if (options.size() == 0)
		return std::string();

	if (options.size() == 1)
		return options.front();

	JsonArray option_data;
	option_data.reserve(options.size());
	for (const std::string &option : options) {
		option_data.push_back(option);
	}
	int timeout = 15 * 1000;
	int reply = request(cmd::AskForOption, option_data, timeout).toInt();
	if (0 <= reply && reply < options.size())
		return options.at(reply);
	else
		return options.front();
}

void ServerPlayer::broadcastProperty(const char *name, const KA_IMPORT Json &value, ServerPlayer *except) const
{
	JsonArray data;
	data.push_back(id());
	data.push_back(name);
	data.push_back(value);
	m_logic->broadcastNotification(cmd::ModifyPlayer, data, except);
}

void ServerPlayer::unicastProperty(const char *name, const KA_IMPORT Json &value, ServerPlayer *to)
{
	JsonArray data;
	data.push_back(id());
	data.push_back(name);
	data.push_back(value);
	to->notify(cmd::ModifyPlayer, data);
}

void ServerPlayer::addSkillHistory(const Skill *skill)
{
	Player::addSkillHistory(skill);

	JsonObject data;
	data["invoker_id"] = this->id();
	data["skill_id"] = skill->id();

	notify(cmd::InvokeSkill, data);
}

void ServerPlayer::addSkillHistory(const Skill *skill, const std::vector<Card *> &cards)
{
	Player::addSkillHistory(skill);

	JsonObject data;
	data["invoker_id"] = this->id();
	data["skill_id"] = skill->id();

	JsonArray card_data;
	for (const Card *card : cards)
		card_data.push_back(card->id());
	data["cards"] = card_data;

	notify(cmd::InvokeSkill, data);
}

void ServerPlayer::addSkillHistory(const Skill *skill, const std::vector<ServerPlayer *> &targets)
{
	Player::addSkillHistory(skill);

	JsonObject data;
	data["invoker_id"] = this->id();
	data["skill_id"] = skill->id();

	JsonArray target_data;
	for (const ServerPlayer *target : targets)
		target_data.push_back(target->id());
	data["targets"] = target_data;

	m_logic->broadcastNotification(cmd::InvokeSkill, data);
}

void ServerPlayer::addSkillHistory(const Skill *skill, const std::vector<Card *> &cards, const std::vector<ServerPlayer *> &targets)
{
	Player::addSkillHistory(skill);

	JsonObject data;
	data["invoker_id"] = this->id();
	data["skill_id"] = skill->id();

	JsonArray card_data;
	for (const Card *card : cards)
		card_data.push_back(card->id());
	data["cards"] = card_data;

	JsonArray target_data;
	for (const ServerPlayer *target : targets)
		target_data.push_back(target->id());
	data["targets"] = target_data;

	m_logic->broadcastNotification(cmd::InvokeSkill, data);
}

void ServerPlayer::clearSkillHistory()
{
	Player::clearSkillHistory();
	m_logic->broadcastNotification(cmd::ClearSkillHistory, id());
}

void ServerPlayer::addCardHistory(const std::string &name, int times)
{
	Player::addCardHistory(name, times);

	JsonArray data;
	data.push_back(name);
	data.push_back(times);
	notify(cmd::AddCardHistory, data);
}

void ServerPlayer::clearCardHistory()
{
	Player::clearCardHistory();
	notify(cmd::AddCardHistory);
}

const Skill *ServerPlayer::getSkill(uint id) const
{
	const std::vector<const Skill *> &head_skills = headSkillArea()->skills();
	for (const Skill *skill : head_skills) {
		if (skill->id() == id)
			return skill;
	}

	const std::vector<const Skill *> &deputy_skills = deputySkillArea()->skills();
	for (const Skill *skill : deputy_skills) {
		if (skill->id() == id)
			return skill;
	}

	const std::vector<const Skill *> &acqured_skills = acquiredSkillArea()->skills();
	for (const Skill *skill : acqured_skills) {
		if (skill->id() == id)
			return skill;

	}

	return nullptr;
}

void ServerPlayer::addSkill(const Skill *skill, SkillAreaType type)
{
	attachSkill(skill, type);

	SkillStruct add;
	add.owner = this;
	add.skill = skill;
	add.area = type;
	void *data = static_cast<void *>(&add);
	m_logic->trigger(SkillAdded, this, data);
}

void ServerPlayer::removeSkill(const Skill *skill, SkillAreaType type)
{
	detachSkill(skill, type);

	SkillStruct remove;
	remove.owner = this;
	remove.skill = skill;
	remove.area = type;
	void *data = static_cast<void *>(&remove);
	m_logic->trigger(SkillRemoved, this, data);
}

void ServerPlayer::attachSkill(const Skill *skill, SkillAreaType type)
{
	Player::addSkill(skill, type);

	JsonObject data;
	data["player_id"] = id();
	data["skill_id"] = skill->id();
	data["skill_area"] = static_cast<int>(type);
	m_logic->broadcastNotification(cmd::AddSkill, data);
}

void ServerPlayer::detachSkill(const Skill *skill, SkillAreaType type)
{
	Player::removeSkill(skill, type);

	JsonObject data;
	data["player_id"] = id();
	data["skill_id"] = skill->id();
	data["skill_area"] = static_cast<int>(type);
	m_logic->broadcastNotification(cmd::RemoveSkill, data);
}

void ServerPlayer::broadcastTag(const std::string &key)
{
	JsonObject data;
	data["player_id"] = id();
	data["key"] = key;
	data["value"] = tag.at(key);
	m_logic->broadcastNotification(cmd::SetPlayerTag, data);
}

void ServerPlayer::unicastTag(const std::string &key, ServerPlayer *to)
{
	JsonObject data;
	data["player_id"] = id();
	data["key"] = key;
	data["value"] = tag.at(key);
	to->notify(cmd::SetPlayerTag, data);
}

std::vector<const General *> ServerPlayer::askForGeneral(const std::vector<const General *> &candidates, int num)
{
	JsonObject data;
	data["num"] = num;

	JsonArray candidateData;
	for (const General *candidate : candidates)
		candidateData.push_back(candidate->id());
	data["candidates"] = candidateData;

	GeneralList result;
	int timeout = 15 * 1000;
	Json response = request(cmd::ChooseGeneral, data, timeout);
	if (response.isArray()) {
		JsonArray response_data = response.toArray();

		for (const Json &id_data : response_data) {
			uint id = id_data.toUInt();
			for (const General *candidate : candidates) {
				if (candidate->id() == id) {
					result.push_back(candidate);
					break;
				}
			}
		}
	}

	if (result.size() < num) {
		result.clear();
		for (size_t i = 0, candidate_num = candidates.size(); i < num && i < candidate_num; i++) {
			result.push_back(candidates[i]);
		}
	}

	return result;
}

KA_IMPORT Json ServerPlayer::request(cmd command, const KA_IMPORT Json &arguments, int timeout)
{
	return m_user->request(static_cast<int>(command), arguments, timeout);
}

void ServerPlayer::notify(cmd command)
{
	m_user->notify(static_cast<int>(command));
}

void ServerPlayer::notify(cmd command, const KA_IMPORT Json &arguments)
{
	m_user->notify(static_cast<int>(command), arguments);
}

void ServerPlayer::prepareRequest(cmd command, const KA_IMPORT Json &arguments, int timeout)
{
	m_user->prepareRequest(static_cast<int>(command), arguments, timeout);
}
