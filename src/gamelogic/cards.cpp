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

#include "cards.h"

#include "CardArea.h"
#include "GameLogic.h"
#include "ServerPlayer.h"

DefaultCard::DefaultCard(Suit suit, int number)
	: Card(suit, number)
{
}

void DefaultCard::onUse(GameLogic *logic, CardUseStruct &use) const
{
	logic->sortByActionOrder(use.to);

	void *useData = static_cast<void *>(&use);
	logic->trigger(PreCardUsed, use.from, useData);

	CardsMoveStruct move;
	move.to.type = CardAreaType::Table;
	move.isOpen = true;
	move.cards.push_back(use.card);
	logic->moveCards(move);
}

void DefaultCard::use(GameLogic *logic, CardUseStruct &use) const
{
	for (ServerPlayer *target : use.to) {
		CardEffectStruct effect(use);
		effect.to = target;
		logic->takeCardEffect(effect);
	}

	if (use.target) {
		CardEffectStruct effect(use);
		logic->takeCardEffect(effect);
	}

	complete(logic);
}

void DefaultCard::complete(GameLogic *logic) const
{
	const CardArea *table = logic->table();
	if (table->contains(this)) {
		CardsMoveStruct move;
		move.cards.push_back(this);
		move.to.type = CardAreaType::DiscardPile;
		move.isOpen = true;
		logic->moveCards(move);
	}
}

BasicCard::BasicCard(Card::Suit suit, int number)
	: DefaultCard(suit, number)
{
	m_type = Type::Basic;
}


TrickCard::TrickCard(Card::Suit suit, int number)
	: DefaultCard(suit, number)
{
	m_type = Type::Trick;
}

void TrickCard::onEffect(GameLogic *logic, CardEffectStruct &effect) const
{
	if (isNullifiable(effect)) {
		std::vector<ServerPlayer *> players = logic->allPlayers();
		//@to-do: do not ask if no player can use nullification

		bool hasNullification = false;
		for (ServerPlayer *player : players) {
			const std::deque<const Card *> &cards = player->handcardArea()->cards();
			for (const Card *card : cards) {
				if (card->is("Nullification")) {
					hasNullification = true;
					break;
				}
			}

			if (hasNullification) {
				break;
			}
		}

		if (!hasNullification) {
			return;
		}

		for (ServerPlayer *player : players) {
			if (effect.from) {
				if (effect.to)
					player->showPrompt("trick-nullification-1", effect.from, effect.to, effect.use.card);
				else
					player->showPrompt("trick-nullification-2", effect.from, effect.use.card);
			} else if (effect.to) {
				player->showPrompt("trick-nullification-3", effect.to, effect.use.card);
			} else {
				player->showPrompt("trick-nullification-4", effect.use.card);
			}
			const Card *card = player->askForCard("Nullification");
			if (card) {
				CardUseStruct use;
				use.from = player;
				use.card = card;
				use.target = effect.use.card;
				use.extra = static_cast<void *>(&effect);
				logic->useCard(use);
				break;
			}
		}
	}
}

bool TrickCard::isNullifiable(const CardEffectStruct &) const
{
	return true;
}

EquipCard::EquipCard(Card::Suit suit, int number)
	: DefaultCard(suit, number)
	, m_skill(nullptr)
{
	m_type = Type::Equip;
	m_targetFixed = true;
}

void EquipCard::onUse(GameLogic *logic, CardUseStruct &use) const
{
	ServerPlayer *player = use.from;
	if (use.to.empty())
		use.to.push_back(player);

	void *data = static_cast<void *>(&use);
	logic->trigger(PreCardUsed, player, data);
}

void EquipCard::use(GameLogic *logic, CardUseStruct &use) const
{
	if (use.to.empty()) {
		CardsMoveStruct move;
		move << this;
		move.to.type = CardAreaType::DiscardPile;
		move.isOpen = true;
		logic->moveCards(move);
		return;
	}

	ServerPlayer *target = use.to.front();

	//Find the existing equip
	const Card *equippedCard = nullptr;
	std::deque<const Card *> equips = target->equipArea()->cards();
	for (const Card *card : equips) {
		if (card->subtype() == subtype()) {
			equippedCard = card;
			break;
		}
	}

	std::vector<CardsMoveStruct> moves;

	CardsMoveStruct install;
	install << this;
	install.to.type = CardAreaType::Equip;
	install.to.owner = target;
	install.isOpen = true;
	moves.push_back(install);

	if (equippedCard != nullptr) {
		CardsMoveStruct uninstall;
		uninstall << equippedCard;
		uninstall.to.type = CardAreaType::Table;
		uninstall.isOpen = true;
		moves.push_back(uninstall);
	}
	logic->moveCards(moves);

	if (equippedCard != nullptr) {
		const CardArea *table = logic->table();
		if (table->contains(equippedCard)) {
			CardsMoveStruct discard;
			discard << equippedCard;
			discard.to.type = CardAreaType::DiscardPile;
			discard.isOpen = true;
			logic->moveCards(discard);
		}
	}
}

void EquipCard::complete(GameLogic *) const
{
}

GlobalEffect::GlobalEffect(Card::Suit suit, int number)
	: TrickCard(suit, number)
{
	m_targetFixed = true;
	m_subtype = GlobalEffectType;
	m_maxTargetNum = InfinityNum;
}

void GlobalEffect::onUse(GameLogic *logic, CardUseStruct &use) const
{
	if (use.to.empty()) {
		std::vector<const Player *> selected;
		std::vector<ServerPlayer *> targets = logic->allPlayers();
		for (ServerPlayer *toSelect : targets) {
			if (targetFilter(selected, toSelect, use.from)) {
				selected.push_back(toSelect);
				use.to.push_back(toSelect);
			}
		}
	}
	TrickCard::onUse(logic, use);
}

AreaOfEffect::AreaOfEffect(Card::Suit suit, int number)
	: TrickCard(suit, number)
{
	m_targetFixed = true;
	m_subtype = AreaOfEffectType;
	m_maxTargetNum = InfinityNum;
}

void AreaOfEffect::onUse(GameLogic *logic, CardUseStruct &use) const
{
	if (use.to.empty()) {
		std::vector<const Player *> selected;
		std::vector<ServerPlayer *> targets = logic->otherPlayers(use.from);
		for (ServerPlayer *toSelect : targets) {
			if (targetFilter(selected, toSelect, use.from)) {
				selected.push_back(toSelect);
				use.to.push_back(toSelect);
			}
		}
	}
	TrickCard::onUse(logic, use);
}

SingleTargetTrick::SingleTargetTrick(Card::Suit suit, int number)
	: TrickCard(suit, number)
{
	m_subtype = SingleTargetType;
}

DelayedTrick::DelayedTrick(Card::Suit suit, int number)
	: TrickCard(suit, number)
{
	m_subtype = DelayedType;
}

bool DelayedTrick::targetFeasible(const std::vector<const Player *> &selected, const Player *) const
{
	return selected.size() == 1;
}

bool DelayedTrick::targetFilter(const std::vector<const Player *> &selected, const Player *toSelect, const Player *source) const
{
	if (!selected.empty() || toSelect == source)
		return false;

	if (!TrickCard::targetFilter(selected, toSelect, source))
		return false;

	const CardArea *area = toSelect->delayedTrickArea();
	if (area->size() <= 0) {
		return true;
	}

	const std::deque<const Card *> &area_cards = area->cards();
	for (const Card *card : area_cards) {
		if (card->is(this->name())) {
			return false;
		}
	}
	return true;
}

void DelayedTrick::onUse(GameLogic *logic, CardUseStruct &use) const
{
	logic->sortByActionOrder(use.to);

	void *use_data = static_cast<void *>(&use);
	logic->trigger(PreCardUsed, use.from, use_data);
}

void DelayedTrick::use(GameLogic *logic, CardUseStruct &use) const
{
	CardsMoveStruct move;
	move << use.card;
	move.isOpen = true;
	if (use.to.empty()) {
		move.to.type = CardAreaType::DiscardPile;
	} else {
		move.to.type = CardAreaType::DelayedTrick;
		move.to.owner = use.to.front();
	}
	logic->moveCards(move);
}

void DelayedTrick::onEffect(GameLogic *logic, CardEffectStruct &effect) const
{
	CardsMoveStruct move;
	move << this;
	move.to.type = CardAreaType::Table;
	move.isOpen = true;
	logic->moveCards(move);

	TrickCard::onEffect(logic, effect);
}

void DelayedTrick::effect(GameLogic *logic, CardEffectStruct &effect) const
{
	JudgeStruct judge(m_judgePattern);
	judge.who = effect.to;
	logic->judge(judge);

	if (judge.matched)
		takeEffect(logic, effect);
}

MovableDelayedTrick::MovableDelayedTrick(Card::Suit suit, int number)
	: DelayedTrick(suit, number)
{
	m_targetFixed = true;
}

void MovableDelayedTrick::onUse(GameLogic *logic, CardUseStruct &use) const
{
	if (use.to.empty())
		use.to.push_back(use.from);
	DelayedTrick::onUse(logic, use);
}

void MovableDelayedTrick::effect(GameLogic *logic, CardEffectStruct &effect) const
{
	JudgeStruct judge(m_judgePattern);
	judge.who = effect.to;
	logic->judge(judge);

	if (judge.matched) {
		takeEffect(logic, effect);
		const CardArea *table = logic->table();
		if (table->contains(this)) {
			CardsMoveStruct move;
			move << this;
			move.to.type = CardAreaType::DiscardPile;
			move.isOpen = true;
			logic->moveCards(move);
		}
	}
}

void MovableDelayedTrick::complete(GameLogic *logic) const
{
	const CardArea *table = logic->table();
	if (!table->contains(this))
		return;

	ServerPlayer *current = logic->currentPlayer();
	ServerPlayer *target = logic->currentPlayer();
	for (;;) {
		target = dynamic_cast<ServerPlayer *>(target->nextAlive());
		if (!targetFilter(std::vector<const Player *>(), target, nullptr) && target != current)
			continue;

		CardsMoveStruct move;
		move << this;
		move.to.type = CardAreaType::DelayedTrick;
		move.to.owner = target;
		move.isOpen = true;
		logic->moveCards(move);

		CardUseStruct use;
		use.card = this;
		use.to.push_back(target);

		void *data = static_cast<void *>(&use);
		for (ServerPlayer *to : use.to)
			logic->trigger(TargetConfirming, to, data);
		if (use.to.empty())
			continue;
		for (ServerPlayer *to : use.to)
			logic->trigger(TargetConfirmed, to, data);
		if (!use.to.empty())
			break;
	}
}

bool MovableDelayedTrick::isAvailable(const Player *player) const
{
	const std::deque<const Card *> &cards = player->delayedTrickArea()->cards();
	for (const Card *card : cards) {
		if (card->is(this->name()))
			return false;
	}
	return DelayedTrick::isAvailable(player);
}

Weapon::Weapon(Card::Suit suit, int number)
	: EquipCard(suit, number)
	, m_attackRange(0)
{
	m_subtype = WeaponType;
}


Armor::Armor(Card::Suit suit, int number)
	: EquipCard(suit, number)
{
	m_subtype = ArmorType;
}


Horse::Horse(Card::Suit suit, int number)
	: EquipCard(suit, number)
{
}

OffensiveHorse::OffensiveHorse(Card::Suit suit, int number)
	: Horse(suit, number)
	, m_extraOutDistance(-1)
{
	m_subtype = OffensiveHorseType;
}

DefensiveHorse::DefensiveHorse(Card::Suit suit, int number)
	: Horse(suit, number)
	, m_extraInDistance(+1)
{
	m_subtype = DefensiveHorseType;
}

Treasure::Treasure(Card::Suit suit, int number)
	: EquipCard(suit, number)
{
	m_subtype = TreasureType;
}
