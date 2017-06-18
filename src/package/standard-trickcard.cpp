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

#include "standardpackage.h"
#include "standard-trickcard.h"

#include "cmd.h"

#include "CardArea.h"
#include "GameLogic.h"
#include "GameConfig.h"
#include "ServerPlayer.h"

KA_USING_NAMESPACE

AmazingGrace::AmazingGrace(CardSuit suit, int number)
	: GlobalEffect(suit, number)
{
	setName("amazing_grace");
}

void AmazingGrace::use(GameLogic *logic, CardUseStruct &use) const
{
	CardsMoveStruct move;
	move.from.type = CardAreaType::DrawPile;
	move.from.direction = CardMoveDirection::Top;
	move.to.type = CardAreaType::Wugu;
	move.isOpen = true;

	int n = static_cast<int>(logic->allPlayers().size());
	move.cards = logic->getDrawPileCards(n);

	logic->moveCards(move);

	logic->broadcastNotification(cmd::ShowAmazingGrace);

	try {
		GlobalEffect::use(logic, use);
		clearRestCards(logic);
	} catch (EventType e) {
		if (e == TurnBroken || e == StageChange)
			clearRestCards(logic);
		throw e;
	}
}

void AmazingGrace::effect(GameLogic *logic, CardEffectStruct &effect) const
{
	int timeout = logic->config()->timeout * 1000;

	Json reply_data = effect.to->request(cmd::TakeAmazingGrace, Json(), timeout);
	uint cardId = reply_data.toUInt();

	const Card *takenCard = nullptr;
	const CardArea *wugu = logic->wugu();
	const std::deque<const Card *> &cards = wugu->cards();
	for (const Card *card : cards) {
		if (card->id() == cardId) {
			takenCard = card;
			break;
		}
	}
	if (takenCard == nullptr)
		takenCard = cards.front();

	CardsMoveStruct move;
	move.from.type = CardAreaType::Wugu;
	move << takenCard;
	move.to.type = CardAreaType::Hand;
	move.to.owner = effect.to;
	move.isOpen = true;
	logic->moveCards(move);
}

void AmazingGrace::clearRestCards(GameLogic *logic) const
{
	logic->broadcastNotification(cmd::ClearAmazingGrace);

	const CardArea *wugu = logic->wugu();
	if (wugu->size() <= 0)
		return;

	CardsMoveStruct move;
	move << wugu->cards();
	move.from.type = CardAreaType::Wugu;
	move.to.type = CardAreaType::DiscardPile;
	move.isOpen = true;
	logic->moveCards(move);
}

GodSalvation::GodSalvation(CardSuit suit, int number)
	: GlobalEffect(suit, number)
{
	setName("god_salvation");
}

bool GodSalvation::isNullifiable(const CardEffectStruct &effect) const
{
	return effect.to->isWounded() && TrickCard::isNullifiable(effect);
}

void GodSalvation::effect(GameLogic *logic, CardEffectStruct &effect) const
{
	if (effect.to->isWounded()) {
		RecoverStruct recover;
		recover.card = this;
		recover.from = effect.from;
		recover.to = effect.to;
		logic->recover(recover);
	}
}

SavageAssault::SavageAssault(CardSuit suit, int number)
	: AreaOfEffect(suit, number)
{
	setName("savage_assault");
}

void SavageAssault::effect(GameLogic *logic, CardEffectStruct &effect) const
{
	effect.to->showPrompt("savage-assault-slash", effect.from);
	const Card *slash = effect.to->askForCard("Slash");
	if (slash) {
		CardResponseStruct response;
		response.from = effect.to;
		response.to = effect.from;
		response.card = slash;
		response.target = this;
		logic->respondCard(response);
	} else {
		DamageStruct damage;
		damage.card = this;
		damage.from = effect.from->isAlive() ? effect.from : nullptr;
		damage.to = effect.to;
		logic->damage(damage);
	}
}

ArcheryAttack::ArcheryAttack(CardSuit suit, int number)
	: AreaOfEffect(suit, number)
{
	setName("archery_attack");
}

void ArcheryAttack::effect(GameLogic *logic, CardEffectStruct &effect) const
{
	effect.to->showPrompt("archery-attack-jink", effect.from);
	const Card *jink = effect.to->askForCard("Jink");
	if (jink) {
		CardResponseStruct response;
		response.from = effect.to;
		response.to = effect.from;
		response.card = jink;
		response.target = this;
		logic->respondCard(response);
	} else {
		DamageStruct damage;
		damage.card = this;
		damage.from = effect.from->isAlive() ? effect.from : nullptr;
		damage.to = effect.to;
		logic->damage(damage);
	}
}

ExNihilo::ExNihilo(CardSuit suit, int number)
	: SingleTargetTrick(suit, number)
{
	setName("ex_nihilo");
	m_targetFixed = true;
}

void ExNihilo::onUse(GameLogic *logic, CardUseStruct &use) const
{
	if (use.to.empty())
		use.to.push_back(use.from);
	SingleTargetTrick::onUse(logic, use);
}

void ExNihilo::effect(GameLogic *, CardEffectStruct &effect) const
{
	effect.to->drawCards(2);
}

Duel::Duel(CardSuit suit, int number)
	: SingleTargetTrick(suit, number)
{
	setName("duel");
}

bool Duel::targetFilter(const std::vector<const Player *> &targets, const Player *toSelect, const Player *self) const
{
	return targets.empty() && toSelect != self && SingleTargetTrick::targetFilter(targets, toSelect, self);
}

void Duel::effect(GameLogic *logic, CardEffectStruct &effect) const
{
	ServerPlayer *first = effect.to;
	ServerPlayer *second = effect.from;

	for (;;) {
		if (!first->isAlive())
			break;
		first->showPrompt("duel-slash", second);
		const Card *slash = first->askForCard("Slash");
		if (slash == nullptr)
			break;
		CardResponseStruct response;
		response.card = slash;
		response.target = this;
		response.from = first;
		response.to = second;
		if (!logic->respondCard(response))
			break;
		std::swap(first, second);
	}

	DamageStruct damage;
	damage.card = this;
	damage.from = second->isAlive() ? second : nullptr;
	damage.to = first;
	if (second != effect.from)
		damage.byUser = false;
	logic->damage(damage);
}

Indulgence::Indulgence(CardSuit suit, int number)
	: DelayedTrick(suit, number)
{
	setName("indulgence");
	m_judgePattern = ".|^heart";
}

void Indulgence::takeEffect(GameLogic *, CardEffectStruct &effect) const
{
	effect.to->clearCardHistory();
	effect.to->skipPhase(PlayerPhase::Play);
}

Snatch::Snatch(CardSuit suit, int number)
	: SingleTargetTrick(suit, number)
{
	setName("snatch");
	m_distanceLimit = 1;
}

bool Snatch::targetFilter(const std::vector<const Player *> &targets, const Player *toSelect, const Player *self) const
{
	return toSelect != self && !toSelect->isAllNude() && SingleTargetTrick::targetFilter(targets, toSelect, self);
}

void Snatch::effect(GameLogic *logic, CardEffectStruct &effect) const
{
	if (effect.from->isDead())
		return;
	if (effect.to->isAllNude())
		return;

	const Card *card = effect.from->askToChooseCard(effect.to);
	if (card) {
		CardsMoveStruct move;
		move << card;
		move.to.owner = effect.from;
		move.to.type = CardAreaType::Hand;
		logic->moveCards(move);
	}
}

Dismantlement::Dismantlement(CardSuit suit, int number)
	: SingleTargetTrick(suit, number)
{
	setName("dismantlement");
}

bool Dismantlement::targetFilter(const std::vector<const Player *> &targets, const Player *toSelect, const Player *self) const
{
	return toSelect != self && !toSelect->isAllNude() && SingleTargetTrick::targetFilter(targets, toSelect, self);
}

void Dismantlement::effect(GameLogic *logic, CardEffectStruct &effect) const
{
	if (effect.from->isDead())
		return;
	if (effect.to->isAllNude())
		return;

	const Card *card = effect.from->askToChooseCard(effect.to);
	if (card) {
		CardsMoveStruct move;
		move << card;
		move.to.type = CardAreaType::DiscardPile;
		move.isOpen = true;
		logic->moveCards(move);
	}
}

Collateral::Collateral(CardSuit suit, int number)
	: SingleTargetTrick(suit, number)
{
	setName("collateral");
}

bool Collateral::isAvailable(const Player *player) const
{
	bool can_use = false;
	const Player *next = player->nextAlive();
	while (next && next != player) {
		if (next->equipArea()->contains("Weapon")) {
			can_use = true;
			break;
		}
		next = next->nextAlive();
	}
	return can_use && SingleTargetTrick::isAvailable(player);
}

bool Collateral::targetFeasible(const std::vector<const Player *> &targets, const Player *) const
{
	return targets.size() == 2;
}

bool Collateral::targetFilter(const std::vector<const Player *> &targets, const Player *toSelect, const Player *self) const
{
	if (!targets.empty()) {
		if (targets.size() >= 2)
			return false;
		const Player *slashSource = targets.front();
		// @to-do: Check prohibit skills like Kongcheng
		return toSelect->inAttackRangeOf(slashSource);
	} else {
		const CardArea *equips = toSelect->equipArea();
		return equips->contains("Weapon") && toSelect != self && SingleTargetTrick::targetFilter(targets, toSelect, self);
	}
}

void Collateral::onUse(GameLogic *logic, CardUseStruct &use) const
{
	if (use.to.size() > 1) {
		use.extra = static_cast<void *>(use.to.at(1));
		use.to.erase(use.to.begin() + 1);
	}
	SingleTargetTrick::onUse(logic, use);
}

bool Collateral::doCollateral(CardEffectStruct &effect) const
{
	ServerPlayer *victim = static_cast<ServerPlayer *>(effect.use.extra);
	if (victim) {
		if (!victim->inAttackRangeOf(effect.to))
			return false;
		std::vector<ServerPlayer *> targets = {victim};
		effect.to->showPrompt("collateral-slash", effect.from, victim);
		return effect.to->askToUseCard("Slash", targets);
	}
}

void Collateral::effect(GameLogic *logic, CardEffectStruct &effect) const
{
	const Card *weapon = nullptr;
	const std::deque<const Card *> &cards = effect.to->equipArea()->cards();
	for (const  Card *card : cards) {
		if (card->subtype() == EquipCard::WeaponType) {
			weapon = card;
			break;
		}
	}

	ServerPlayer *victim = static_cast<ServerPlayer *>(effect.use.extra);
	if (victim && victim->isDead()) {
		if (effect.from->isAlive() && effect.to->isAlive() && weapon) {
			CardsMoveStruct move;
			move << weapon;
			move.to.type = CardAreaType::Hand;
			move.to.owner = effect.from;
			logic->moveCards(move);
		}
	} else if (effect.from->isDead()) {
		if (effect.to->isAlive())
			doCollateral(effect);
	} else {
		if (effect.to->isDead()) {
			; // do nothing
		} else if (weapon == nullptr) {
			doCollateral(effect);
		} else {
			if (!doCollateral(effect)) {
				if (weapon) {
					CardsMoveStruct move;
					move << weapon;
					move.to.type = CardAreaType::Hand;
					move.to.owner = effect.from;
					logic->moveCards(move);
				}
			}
		}
	}
}

Nullification::Nullification(CardSuit suit, int number)
	: SingleTargetTrick(suit, number)
{
	setName("nullification");
	m_targetFixed = true;
}

bool Nullification::isAvailable(const Player *) const
{
	return false;
}

void Nullification::effect(GameLogic *, CardEffectStruct &effect) const
{
	CardEffectStruct *trick_effect = static_cast<CardEffectStruct *>(effect.use.extra);
	if (trick_effect) {
		if (trick_effect->to)
			trick_effect->use.nullifiedList.push_back(trick_effect->to);
		else if (trick_effect->use.card->is("Nullification"))
			trick_effect->use.isNullified = true;
	}
}

Lightning::Lightning(CardSuit suit, int number)
	: MovableDelayedTrick(suit, number)
{
	setName("lightning");
	m_judgePattern = ".|spade|2~9";
}

void Lightning::takeEffect(GameLogic *logic, CardEffectStruct &effect) const
{
	DamageStruct damage;
	damage.to = effect.to;
	damage.card = this;
	damage.damage = 3;
	damage.nature = DamageStruct::Thunder;
	logic->damage(damage);
}

void StandardPackage::addTrickCards()
{
	std::vector<Card *> cards = {
		new AmazingGrace(CardSuit::Heart, 3),
		new AmazingGrace(CardSuit::Heart, 4),
		new GodSalvation(CardSuit::Heart, 1),
		new SavageAssault(CardSuit::Spade, 7),
		new SavageAssault(CardSuit::Spade, 13),
		new SavageAssault(CardSuit::Club, 7),
		new ArcheryAttack(CardSuit::Heart, 1),
		new Duel(CardSuit::Spade, 1),
		new Duel(CardSuit::Club, 1),
		new Duel(CardSuit::Diamond, 1),
		new ExNihilo(CardSuit::Heart, 7),
		new ExNihilo(CardSuit::Heart, 8),
		new ExNihilo(CardSuit::Heart, 9),
		new ExNihilo(CardSuit::Heart, 11),
		new Snatch(CardSuit::Spade, 3),
		new Snatch(CardSuit::Spade, 4),
		new Snatch(CardSuit::Spade, 11),
		new Snatch(CardSuit::Diamond, 3),
		new Snatch(CardSuit::Diamond, 4),
		new Dismantlement(CardSuit::Spade, 3),
		new Dismantlement(CardSuit::Spade, 4),
		new Dismantlement(CardSuit::Spade, 12),
		new Dismantlement(CardSuit::Club, 3),
		new Dismantlement(CardSuit::Club, 4),
		new Dismantlement(CardSuit::Heart, 12),
		new Collateral(CardSuit::Club, 12),
		new Collateral(CardSuit::Club, 13),
		new Nullification(CardSuit::Spade, 11),
		new Nullification(CardSuit::Club, 12),
		new Nullification(CardSuit::Club, 13),
		new Indulgence(CardSuit::Spade, 6),
		new Indulgence(CardSuit::Club, 6),
		new Indulgence(CardSuit::Heart, 6),
		new Lightning(CardSuit::Spade, 1)
	};
	addCards(cards);
}
