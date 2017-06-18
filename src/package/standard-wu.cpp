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

#include "Card.h"
#include "CardArea.h"
#include "GameLogic.h"
#include "General.h"
#include "ServerPlayer.h"

#include "skills.h"
#include "structs.h"

#include <algorithm>

namespace {

class Zhiheng : public ProactiveSkill
{
public:
	Zhiheng() : ProactiveSkill("zhiheng")
	{
	}

	bool isAvailable(const Player *self, const std::string &pattern) const override
	{
		return ProactiveSkill::isAvailable(self, pattern) && self->skillHistory(this) <= 0;
	}

	bool cardFilter(const std::vector<const Card *> &, const Card *, const Player *, const std::string &) const override
	{
		return true;
	}

	bool cardFeasible(const std::vector<const Card *> &selected, const Player *) const override
	{
		return selected.size() > 0;
	}

	bool playerFilter(const std::vector<const Player *> &, const Player *, const Player *) const override
	{
		return false;
	}

	bool playerFeasible(const std::vector<const Player *> &selected, const Player *) const override
	{
		return selected.empty();
	}

	void effect(GameLogic *logic, ServerPlayer *from, const std::vector<ServerPlayer *> &, const std::vector<const Card *> &cards) const override
	{
		CardsMoveStruct move;
		move.cards = cards;
		move.to.type = CardAreaType::DiscardPile;
		move.isOpen = true;
		logic->moveCards(move);

		from->drawCards(static_cast<int>(cards.size()));
	}
};

class Qixi : public OneCardViewAsSkill
{
public:
	Qixi() : OneCardViewAsSkill("qixi")
	{
	}

	bool viewFilter(const Card *card, const Player *, const std::string &) const override
	{
		return card->color() == CardColor::Black;
	}

	const Card *viewAs(const Card *card, const Player *) const override
	{
		Dismantlement *dismantlement = new Dismantlement(card->suit(), card->number());
		dismantlement->addSubcard(card);
		dismantlement->setSkill(this);
		return dismantlement;
	}
};

class Keji : public TriggerSkill
{
	class Record : public TriggerSkill
	{
	public:
		Record() : TriggerSkill("keji_record")
		{
			m_events.insert(PreCardUsed);
			m_events.insert(CardResponded);
			setFrequency(Compulsory);
		}

		EventList triggerable(GameLogic *, EventType event, ServerPlayer *player, void *data, ServerPlayer *) const override
		{
			if (player->phase() == PlayerPhase::Play && TriggerSkill::triggerable(player)) {
				const Card *card = nullptr;
				if (event == PreCardUsed)
					card = static_cast<CardUseStruct *>(data)->card;
				else
					card = static_cast<CardResponseStruct *>(data)->card;
				if (card->is("Slash"))
					player->tag["KejiSlashInPlayPhase"] = true;
			}
			return EventList();
		}
	};

public:
	Keji() : TriggerSkill("keji")
	{
		m_events.insert(PhaseStart);
		addChild(new Record);
	}

	bool triggerable(ServerPlayer *owner) const override
	{
		if (owner->phase() != PlayerPhase::Discard)
			return false;

		bool slashInPlayPhase = owner->tag.at("KejiSlashInPlayPhase").toBool();
		if (slashInPlayPhase) {
			owner->tag.erase("KejiSlashInPlayPhase");
			return false;
		}

		return TriggerSkill::triggerable(owner);
	}

	bool effect(GameLogic *, EventType, ServerPlayer *, void *, ServerPlayer *) const override
	{
		return true;
	}
};

class Kurou : public ProactiveSkill
{
public:
	Kurou() : ProactiveSkill("kurou")
	{
	}

	bool isAvailable(const Player *self, const std::string &pattern) const
	{
		return ProactiveSkill::isAvailable(self, pattern) && self->hp() > 0;
	}

	bool cardFeasible(const std::vector<const Card *> &selected, const Player *) const override
	{
		return selected.empty();
	}

	bool cost(GameLogic *logic, ServerPlayer *from, const std::vector<ServerPlayer *> &, const std::vector<const Card *> &) const override
	{
		logic->loseHp(from, 1);
		return true;
	}

	void effect(GameLogic *, ServerPlayer *from, const std::vector<ServerPlayer *> &, const std::vector<const Card *> &) const override
	{
		from->drawCards(2);
	}
};

class Yingzi : public TriggerSkill
{
public:
	Yingzi() : TriggerSkill("yingzi")
	{
		m_events.insert(DrawNCards);
	}

	bool effect(GameLogic *, EventType, ServerPlayer *, void *data, ServerPlayer *) const override
	{
		int *drawNum = static_cast<int *>(data);
		*drawNum++;
		return false;
	}
};

class Fanjian : public ProactiveSkill
{
public:
	Fanjian() : ProactiveSkill("fanjian")
	{
	}

	bool isAvailable(const Player *self, const std::string &pattern) const override
	{
		return ProactiveSkill::isAvailable(self, pattern) && self->skillHistory(this) <= 0;
	}

	bool cardFeasible(const std::vector<const Card *> &selected, const Player *) const override
	{
		return selected.empty();
	}

	bool playerFeasible(const std::vector<const Player *> &selected, const Player *) const override
	{
		return selected.size() == 1;
	}

	bool playerFilter(const std::vector<const Player *> &selected, const Player *toSelect, const Player *source) const override
	{
		return selected.empty() && toSelect != source;
	}

	void effect(GameLogic *logic, ServerPlayer *from, const std::vector<ServerPlayer *> &to, const std::vector<const Card *> &) const override
	{
		ServerPlayer *victim = to.front();

		std::vector<std::string> options = {"spade", "heart", "club", "diamond"};
		std::string suit = victim->askForOption(options);
		//@to-do: add a log that shows the result

		const Card *card = victim->askToChooseCard(from, "h");
		victim->showCard(card);
		if (card) {
			CardsMoveStruct move;
			move << card;
			move.to.owner = victim;
			move.to.type = CardAreaType::Hand;
			logic->moveCards(move);

			if (card->suitString() != suit) {
				DamageStruct damage;
				damage.from = from;
				damage.to = victim;
				logic->damage(damage);
			}
		}
	}
};

class Guose : public OneCardViewAsSkill
{
public:
	Guose() : OneCardViewAsSkill("guose")
	{
	}

	bool viewFilter(const Card *card, const Player *, const std::string &pattern) const override
	{
		return card->suit() == CardSuit::Diamond && pattern.empty();
	}

	const Card *viewAs(const Card *card, const Player *) const override
	{
		Card *indulgence = new Indulgence(card->suit(), card->number());
		indulgence->addSubcard(card);
		indulgence->setSkill(this);
		return indulgence;
	}
};

class Liuli : public ProactiveSkill
{
	class Timing : public TriggerSkill
	{
	public:
		Timing() : TriggerSkill("liuli")
		{
			m_events.insert(TargetConfirming);
		}

		EventList triggerable(GameLogic *logic, EventType, ServerPlayer *target, void *data, ServerPlayer *) const override
		{
			EventList events;
			if (!TriggerSkill::triggerable(target))
				return events;

			CardUseStruct *use = static_cast<CardUseStruct *>(data);
			if (use->card->is("Slash") && std::find(use->to.begin(), use->to.end(), target) != use->to.end()) {
				std::vector<ServerPlayer *> players = logic->otherPlayers(target);
				auto i = std::find(players.begin(), players.end(), use->from);
				if (i != players.end()) {
					players.erase(i);
				}

				for (ServerPlayer *p : players) {
					if (p->inAttackRangeOf(target)) {
						Event e(this, target);
						events.push_back(e);
						break;
					}
				}
			}

			return events;
		}

		bool cost(GameLogic *, EventType, ServerPlayer *target, void *data, ServerPlayer *) const override
		{
			CardUseStruct *use = static_cast<CardUseStruct *>(data);
			use->from->tag["liuli_slash_source"] = true;
			use->from->unicastTag("liuli_slash_source", target);
			target->showPrompt("invoke_liuli", use->from);
			return target->askToUseCard("@liuli");
		}

		bool effect(GameLogic *logic, EventType, ServerPlayer *daqiao, void *data, ServerPlayer *) const override
		{
			CardUseStruct *use = static_cast<CardUseStruct *>(data);

			std::vector<ServerPlayer *> players = logic->otherPlayers(daqiao);
			for (ServerPlayer *target : players) {
				if (target->tag.find("liuli_target") != target->tag.end()) {
					target->tag.erase("liuli_target");
					auto i = std::find(use->to.begin(), use->to.end(), daqiao);
					if (i != use->to.end()) {
						use->to.erase(i);
					}
					use->to.push_back(target);
					logic->sortByActionOrder(use->to);
					logic->trigger(TargetConfirming, target, data);
					return false;
				}
			}

			return false;
		}
	};

public:
	Liuli() : ProactiveSkill("liuli")
	{
		addChild(new Timing);
	}

	bool isAvailable(const Player *, const std::string &pattern) const override
	{
		return pattern == "@liuli";
	}

	bool cardFeasible(const std::vector<const Card *> &selected, const Player *) const override
	{
		return selected.size() == 1;
	}

	bool cardFilter(const std::vector<const Card *> &selected, const Card *, const Player *, const std::string &pattern) const override
	{
		return selected.empty() && pattern == "@liuli";
	}

	bool playerFeasible(const std::vector<const Player *> &selected, const Player *) const override
	{
		return selected.size() == 1;
	}

	bool playerFilter(const std::vector<const Player *> &selected, const Player *to_select, const Player *source) const override
	{
		if (!selected.empty())
			return false;

		return to_select->tag.find("liuli_slash_source") == to_select->tag.end() && to_select->inAttackRangeOf(source);
	}

	void effect(GameLogic *logic, ServerPlayer *, const std::vector<ServerPlayer *> &to, const std::vector<const Card *> &cards) const override
	{
		CardsMoveStruct move;
		move.cards = cards;
		move.to.type = CardAreaType::DiscardPile;
		move.isOpen = true;
		logic->moveCards(move);

		for (ServerPlayer *target : to) {
			target->tag["liuli_target"] = true;
		}
	}
};

class Qianxun : public CardModSkill
{
public:
	Qianxun() : CardModSkill("qianxun")
	{
	}

	bool targetFilter(const Card *card, const std::vector<const Player *> &, const Player *toSelect, const Player *) const override
	{
		if (card->is("Indulgence") || card->is("Snatch"))
			return !toSelect->hasSkill(this);
		return true;
	}
};

class Lianying : public TriggerSkill
{
public:
	Lianying() : TriggerSkill("lianying")
	{
		m_events.insert(AfterCardsMove);
	}

	EventList triggerable(GameLogic *, EventType, ServerPlayer *target, void *data, ServerPlayer *) const override
	{
		EventList events;

		if (TriggerSkill::triggerable(target)) {
			std::vector<CardsMoveStruct> *moves = static_cast<std::vector<CardsMoveStruct> *>(data);
			for (const CardsMoveStruct &move : *moves) {
				if (move.from.owner == target && move.from.type == CardAreaType::Hand && target->handcardNum() == 0) {
					Event e(this, target);
					events.push_back(e);
					break;
				}
			}
		}

		return events;
	}

	bool effect(GameLogic *, EventType, ServerPlayer *target, void *, ServerPlayer *) const override
	{
		target->drawCards(1);
		return false;
	}
};

class Xiaoji : public TriggerSkill
{
public:
	Xiaoji() : TriggerSkill("xiaoji")
	{
		m_events.insert(AfterCardsMove);
	}

	EventList triggerable(GameLogic *, EventType, ServerPlayer *target, void *data, ServerPlayer *) const override
	{
		EventList events;

		if (TriggerSkill::triggerable(target)) {
			std::vector<CardsMoveStruct> *moves = static_cast<std::vector<CardsMoveStruct> *>(data);
			for (const CardsMoveStruct &move : *moves) {
				if (move.from.owner == target && move.from.type == CardAreaType::Equip) {
					Event e(this, target);
					for (int i = 0; i < move.cards.size(); i++)
						events.push_back(e);
				}
			}
		}

		return events;
	}

	bool effect(GameLogic *, EventType, ServerPlayer *target, void *, ServerPlayer *) const override
	{
		target->drawCards(2);
		return false;
	}
};

class Jieyin : public ProactiveSkill
{
public:
	Jieyin() : ProactiveSkill("jieyin")
	{
	}

	bool isAvailable(const Player *self, const std::string &pattern) const override
	{
		return self->skillHistory(this) <= 0 && ProactiveSkill::isAvailable(self, pattern);
	}

	bool cardFilter(const std::vector<const Card *> &selected, const Card *card, const Player *source, const std::string &) const override
	{
		if (selected.size() > 2)
			return false;

		const CardArea *handArea = source->handcardArea();
		return handArea->contains(card);
	}

	bool cardFeasible(const std::vector<const Card *> &selected, const Player *) const override
	{
		return selected.size() == 2;
	}

	bool playerFilter(const std::vector<const Player *> &selected, const Player *toSelect, const Player *) const override
	{
		if (!selected.empty())
			return false;

		const General *general = toSelect->general();
		if (general == nullptr || general->gender() != Gender::Male)
			return false;

		return toSelect->isWounded();
	}

	bool playerFeasible(const std::vector<const Player *> &selected, const Player *) const override
	{
		return selected.size() == 1;
	}

	bool cost(GameLogic *logic, ServerPlayer *, const std::vector<ServerPlayer *> &, const std::vector<const Card *> &cards) const override
	{
		CardsMoveStruct discard;
		discard.cards = cards;
		discard.to.type = CardAreaType::DiscardPile;
		discard.isOpen = true;
		logic->moveCards(discard);

		return true;
	}

	void effect(GameLogic *logic, ServerPlayer *from, const std::vector<ServerPlayer *> &to, const std::vector<const Card *> &) const override
	{
		std::vector<ServerPlayer *> targets(1 + to.size());
		targets[0] = from;
		std::copy(to.begin(), to.end(), targets.begin() + 1);
		logic->sortByActionOrder(targets);

		for (ServerPlayer *target : targets) {
			RecoverStruct recover;
			recover.from = from;
			recover.to = target;
			logic->recover(recover);
		}
	}
};

}

void StandardPackage::addWuGenerals()
{
	//WU 001
	General *sunquan = new General("sunquan", "wu", 4);
	sunquan->setLord(true);
	sunquan->addSkill(new Zhiheng);
	addGeneral(sunquan);

	//WU 002
	General *ganning = new General("ganning", "wu", 4);
	ganning->addSkill(new Qixi);
	addGeneral(ganning);

	//WU 003
	General *lvmeng = new General("lvmeng", "wu", 4);
	lvmeng->addSkill(new Keji);
	addGeneral(lvmeng);

	//WU 004
	General *huanggai = new General("huanggai", "wu", 4);
	huanggai->addSkill(new Kurou);
	addGeneral(huanggai);

	//WU 005
	General *zhouyu = new General("zhouyu", "wu", 3);
	zhouyu->addSkill(new Yingzi);
	zhouyu->addSkill(new Fanjian);
	addGeneral(zhouyu);

	//WU 006
	General *daqiao = new General("daqiao", "wu", 3, Gender::Female);
	daqiao->addSkill(new Guose);
	daqiao->addSkill(new Liuli);
	addGeneral(daqiao);

	//WU 007
	General *luxun = new General("luxun", "wu", 3);
	luxun->addSkill(new Qianxun);
	luxun->addSkill(new Lianying);
	addGeneral(luxun);

	//WU 008
	General *sunshangxiang = new General("sunshangxiang", "wu", 3, Gender::Female);
	sunshangxiang->addSkill(new Xiaoji);
	sunshangxiang->addSkill(new Jieyin);
	addGeneral(sunshangxiang);
}
