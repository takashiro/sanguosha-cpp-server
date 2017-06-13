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

#include "maneuveringpackage.h"
#include "standard-trickcard.h"

#include "Engine.h"
#include "GameLogic.h"
#include "ServerPlayer.h"

FireSlash::FireSlash(Card::Suit suit, int number)
	: Slash(suit, number)
{
	setName("fire_slash");
	m_nature = DamageStruct::Fire;
}

ThunderSlash::ThunderSlash(Card::Suit suit, int number)
	: Slash(suit, number)
{
	setName("thunder_slash");
	m_nature = DamageStruct::Thunder;
}

Analeptic::Analeptic(Card::Suit suit, int number)
	: BasicCard(suit, number)
{
	setName("analeptic");
	m_targetFixed = true;
}

void Analeptic::onUse(GameLogic *logic, CardUseStruct &use)
{
	if (use.to.empty())
		use.to.push_back(use.from);
	BasicCard::onUse(logic, use);
}

void Analeptic::effect(GameLogic *logic, CardEffectStruct &effect)
{
	if (effect.to->phase() == PlayerPhase::Play) {
		effect.to->setDrunk(true);
		effect.to->broadcastProperty("drunk", true);
	} else {
		RecoverStruct recover;
		recover.card = this;
		recover.from = effect.from;
		recover.to = effect.to;
		logic->recover(recover);
	}
}

Fan::Fan(Card::Suit suit, int number)
	: Weapon(suit, number)
{
	setName("fan");
	m_attackRange = 4;
}

GudingBlade::GudingBlade(Card::Suit suit, int number)
	: Weapon(suit, number)
{
	setName("guding_blade");
	m_attackRange = 2;
}

Vine::Vine(Card::Suit suit, int number)
	: Armor(suit, number)
{
	setName("vine");
}

SilverLion::SilverLion(Card::Suit suit, int number)
	: Armor(suit, number)
{
	setName("silver_lion");
}

SupplyShortage::SupplyShortage(Card::Suit suit, int number)
	: DelayedTrick(suit, number)
{
	setName("supply_shortage");
	m_judgePattern = ".|^club";
	m_distanceLimit = 1;
}

void SupplyShortage::takeEffect(GameLogic *, CardEffectStruct &effect)
{
	effect.to->clearCardHistory();
	effect.to->skipPhase(PlayerPhase::Draw);
}

IronChain::IronChain(Card::Suit suit, int number)
	: TrickCard(suit, number)
{
	setName("iron_chain");
	m_subtype = DamageSpreadType;
	m_canRecast = true;
	m_maxTargetNum = 2;
}

void IronChain::effect(GameLogic *, CardEffectStruct &effect)
{
	bool chained = !effect.to->isChained();
	effect.to->setChained(chained);
	effect.to->broadcastProperty("chained", chained);
}

FireAttack::FireAttack(Card::Suit suit, int number)
	: SingleTargetTrick(suit, number)
{
	setName("fire_attack");
}

bool FireAttack::targetFilter(const std::vector<const Player *> &targets, const Player *toSelect, const Player *self) const
{
	return toSelect->handcardNum() > 0 && SingleTargetTrick::targetFilter(targets, toSelect, self);
}

void FireAttack::effect(GameLogic *logic, CardEffectStruct &effect)
{
	if (effect.to->handcardNum() <= 0)
		return;

	effect.to->showPrompt("fire-attack-show-card", effect.from);
	Card *card = effect.to->askForCard(".", false);
	if (card)
		effect.to->showCard(card);
	else
		return;

	if (effect.from->isAlive()) {
		std::string pattern = ".|";
		pattern += card->suitString();
		effect.from->showPrompt("fire-attack-discard-card", effect.to, card);
		Card *discarded = effect.from->askForCard(pattern);
		if (discarded) {
			CardsMoveStruct move;
			move << discarded;
			move.to.type = CardAreaType::DiscardPile;
			move.isOpen = true;
			logic->moveCards(move);

			DamageStruct damage;
			damage.card = this;
			damage.from = effect.from;
			damage.to = effect.to;
			damage.nature = DamageStruct::Fire;
			logic->damage(damage);
		}
	}
}

ManeuveringPackage::ManeuveringPackage()
	: Package("maneuvering")
{
	std::vector<Card *> cards = {
		// spade
		new GudingBlade(CardSuit::Spade, 1),
		new Vine(CardSuit::Spade, 2),
		new Analeptic(CardSuit::Spade, 3),
		new ThunderSlash(CardSuit::Spade, 4),
		new ThunderSlash(CardSuit::Spade, 5),
		new ThunderSlash(CardSuit::Spade, 6),
		new ThunderSlash(CardSuit::Spade, 7),
		new ThunderSlash(CardSuit::Spade, 8),
		new Analeptic(CardSuit::Spade, 9),
		new SupplyShortage(CardSuit::Spade, 10),
		new IronChain(CardSuit::Spade, 11),
		new IronChain(CardSuit::Spade, 12),
		new Nullification(CardSuit::Spade, 13),

		// club
		new SilverLion(CardSuit::Club, 1),
		new Vine(CardSuit::Club, 2),
		new Analeptic(CardSuit::Club, 3),
		new SupplyShortage(CardSuit::Club, 4),
		new ThunderSlash(CardSuit::Club, 5),
		new ThunderSlash(CardSuit::Club, 6),
		new ThunderSlash(CardSuit::Club, 7),
		new ThunderSlash(CardSuit::Club, 8),
		new Analeptic(CardSuit::Club, 9),
		new IronChain(CardSuit::Club, 10),
		new IronChain(CardSuit::Club, 11),
		new IronChain(CardSuit::Club, 12),
		new IronChain(CardSuit::Club, 13),

		// heart
		new Nullification(CardSuit::Heart, 1),
		new FireAttack(CardSuit::Heart, 2),
		new FireAttack(CardSuit::Heart, 3),
		new FireSlash(CardSuit::Heart, 4),
		new Peach(CardSuit::Heart, 5),
		new Peach(CardSuit::Heart, 6),
		new FireSlash(CardSuit::Heart, 7),
		new Jink(CardSuit::Heart, 8),
		new Jink(CardSuit::Heart, 9),
		new FireSlash(CardSuit::Heart, 10),
		new Jink(CardSuit::Heart, 11),
		new Jink(CardSuit::Heart, 12),
		new Nullification(CardSuit::Heart, 13),

		// diamond
		new Fan(CardSuit::Diamond, 1),
		new Peach(CardSuit::Diamond, 2),
		new Peach(CardSuit::Diamond, 3),
		new FireSlash(CardSuit::Diamond, 4),
		new FireSlash(CardSuit::Diamond, 5),
		new Jink(CardSuit::Diamond, 6),
		new Jink(CardSuit::Diamond, 7),
		new Jink(CardSuit::Diamond, 8),
		new Analeptic(CardSuit::Diamond, 9),
		new Jink(CardSuit::Diamond, 10),
		new Jink(CardSuit::Diamond, 11),
		new FireAttack(CardSuit::Diamond, 12)
	};

	addCards(cards);

	DefensiveHorse *hualiu = new DefensiveHorse(CardSuit::Diamond, 13);
	hualiu->setName("hualiu");
	addCard(hualiu);
}

ADD_PACKAGE(Maneuvering)
