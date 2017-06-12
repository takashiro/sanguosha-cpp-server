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
#include "standard-basiccard.h"

#include "skills.h"
#include "structs.h"

#include "Card.h"
#include "CardArea.h"
#include "GameLogic.h"
#include "General.h"
#include "ServerPlayer.h"

#include <algorithm>

KA_USING_NAMESPACE

namespace
{

class Rende : public ProactiveSkill
{
	class Reset : public TriggerSkill
	{
	public:
		Reset() : TriggerSkill("#rende_reset")
		{
			m_events.insert(PhaseChanging);
		}

		EventList triggerable(GameLogic *, EventType, ServerPlayer *owner, void *data, ServerPlayer *) const override
		{
			if (owner->tag.find("rende_count") != owner->tag.end()) {
				PhaseChangeStruct *change = static_cast<PhaseChangeStruct *>(data);
				if (change->to == PlayerPhase::Inactive)
					owner->tag.erase("rende_count");
			}
			return EventList();
		}
	};

public:
	Rende() : ProactiveSkill("rende")
	{
		addChild(new Reset);
	}

	bool isAvailable(const Player *player, const std::string &pattern) const override
	{
		return player->handcardNum() > 0 && pattern.empty();
	}

	bool cardFilter(const std::vector<const Card *> &, const Card *card, const Player *self, const std::string &) const override
	{
		const CardArea *handcard = self->handcardArea();
		return handcard->contains(card);
	}

	bool cardFeasible(const std::vector<const Card *> &cards, const Player *) const override
	{
		return cards.size() > 0;
	}

	bool playerFilter(const std::vector<const Player *> &targets, const Player *, const Player *) const override
	{
		return targets.empty();
	}

	bool playerFeasible(const std::vector<const Player *> &targets, const Player *) const override
	{
		return targets.size() == 1;
	}

	void effect(GameLogic *logic, ServerPlayer *from, const std::vector<ServerPlayer *> &to, const std::vector<Card *> &cards) const override
	{
		if (to.empty() || cards.empty())
			return;

		CardsMoveStruct move;
		move.cards = cards;
		move.to.owner = to.front();
		move.to.type = CardAreaType::Hand;
		logic->moveCards(move);

		int oldValue = from->tag["rende_count"].toInt();
		int newValue = oldValue + static_cast<int>(cards.size());
		from->tag["rende_count"] = newValue;
		if (oldValue < 2 && newValue >= 2 && from->isWounded()) {
			RecoverStruct recover;
			recover.from = from;
			recover.to = from;
			logic->recover(recover);
		}
	}
};

class Wusheng : public OneCardViewAsSkill
{
public:
	Wusheng() : OneCardViewAsSkill("wusheng")
	{
	}

	bool viewFilter(const Card *card, const Player *, const std::string &) const override
	{
		return card->color() == CardColor::Red;
	}

	Card *viewAs(Card *subcard, const Player *) const override
	{
		Slash *slash = new Slash(subcard->suit(), subcard->number());
		slash->setSkill(this);
		slash->addSubcard(subcard);
		return slash;
	}

	bool isAvailable(const Player *self, const std::string &pattern) const override
	{
		if (pattern.empty())
			return CheckAvailability<Slash>(self);
		else
			return pattern == "Slash";
	}
};

class Paoxiao : public CardModSkill
{
public:
	Paoxiao() : CardModSkill("paoxiao")
	{
	}

	int extraUseNum(const Card *card, const Player *player) const override
	{
		if (player->hasSkill(this) && player->phase() == PlayerPhase::Play && card->is("Slash"))
			return Card::InfinityNum;
		return 0;
	}
};

class Guanxing : public TriggerSkill
{
public:
	Guanxing() : TriggerSkill("guanxing")
	{
		m_events.insert(PhaseStart);
	}

	bool triggerable(ServerPlayer *owner) const override
	{
		return TriggerSkill::triggerable(owner) && owner->phase() == PlayerPhase::Start;
	}

	bool effect(GameLogic *logic, EventType, ServerPlayer *target, void *, ServerPlayer *) const override
	{
		int n = logic->countPlayer(false);
		n = std::min(n, 5);
		std::vector<Card *> cards = logic->getDrawPileCards(n);

		std::vector<int> capacities = {5, 5};
		std::vector<std::string> areaNames = {
			"cards_on_the_top_of_draw_pile",
			"cards_on_the_bottom_of_draw_pile"
		};
		std::vector<std::vector<Card *>> result = target->askToArrangeCard(cards, capacities, areaNames);

		CardArea *drawPile = logic->drawPile();
		drawPile->remove(cards);
		drawPile->add(result.at(0), CardMoveDirection::Top);
		drawPile->add(result.at(1), CardMoveDirection::Bottom);

		return false;
	}
};

class Kongcheng : public CardModSkill
{
public:
	Kongcheng() : CardModSkill("kongcheng")
	{
		m_frequency = Compulsory;
	}

	bool targetFilter(const Card *card, const std::vector<const Player *> &, const Player *toSelect, const Player *) const override
	{
		if (card->is("Slash") || card->is("Duel")) {
			if (toSelect->handcardNum() <= 0 && toSelect->hasSkill(this))
				return false;
		}
		return true;
	}
};

class Longdan : public OneCardViewAsSkill
{
public:
	Longdan() : OneCardViewAsSkill("longdan")
	{
	}

	bool viewFilter(const Card *card, const Player *, const std::string &pattern) const override
	{
		if (pattern == "Slash" || pattern.empty())
			return card->is("Jink");
		else if (pattern == "Jink")
			return card->is("Slash");
		return false;
	}

	Card *viewAs(Card *subcard, const Player *) const override
	{
		Card *card = nullptr;
		if (subcard->is("Slash"))
			card = new Jink(subcard->suit(), subcard->number());
		else if (subcard->is("Jink"))
			card = new Slash(subcard->suit(), subcard->number());
		if (card) {
			card->setSkill(this);
			card->addSubcard(subcard);
		}
		return card;
	}

	bool isAvailable(const Player *self, const std::string &pattern) const override
	{
		if (pattern.empty())
			return CheckAvailability<Slash>(self);
		else
			return pattern == "Slash" || pattern == "Jink";
	}
};

class Mashu : public StatusSkill
{
public:
	Mashu() : StatusSkill("mashu")
	{
	}

	void validate(ServerPlayer *target) const override
	{
		int distance = target->extraOutDistance() - 1;
		target->setExtraOutDistance(distance);
		target->broadcastProperty("extraOutDistance", distance);
	}

	void invalidate(ServerPlayer *target) const override
	{
		int distance = target->extraOutDistance() + 1;
		target->setExtraOutDistance(distance);
		target->broadcastProperty("extraOutDistance", distance);
	}
};

class Tieqi : public TriggerSkill
{
	class Effect : public TriggerSkill
	{
	public:
		Effect() : TriggerSkill("tieqi_effect")
		{
			m_events.insert(SlashProceed);
			setFrequency(Compulsory);
		}

		EventList triggerable(GameLogic *, EventType, ServerPlayer *, void *data, ServerPlayer *) const override
		{
			EventList events;
			SlashEffectStruct *effect = static_cast<SlashEffectStruct *>(data);
			if (effect->from && effect->from->hasSkill(parent()) && effect->from->tag.find("tieqi_victims") != effect->from->tag.end()) {
				JsonArray victims = effect->from->tag.at("tieqi_victims").toArray();
				int max = static_cast<int>(victims.size());
				for (int i = 0; i < max; i++) {
					if (victims.at(i).toUInt() == effect->to->id()) {
						victims.erase(victims.begin() + i);
						if (victims.empty())
							effect->from->tag.erase("tieqi_victims");
						else
							effect->from->tag["tieqi_victims"] = victims;

						Event e(this, effect->from);
						e.to.push_back(effect->to);
						events.push_back(e);
						break;
					}
				}
			}
			return events;
		}

		bool effect(GameLogic *, EventType, ServerPlayer *, void *, ServerPlayer *) const override
		{
			return true;
		}
	};

public:
	Tieqi() : TriggerSkill("tieqi")
	{
		m_events.insert(TargetChosen);
		setFrequency(Frequent);

		addChild(new Effect);
	}

	EventList triggerable(GameLogic *, EventType, ServerPlayer *player, void *data, ServerPlayer *) const override
	{
		EventList events;

		CardUseStruct *use = static_cast<CardUseStruct *>(data);
		if (TriggerSkill::triggerable(player) && use->card && use->card->is("Slash")) {
			for (ServerPlayer *to : use->to) {
				Event e(this, player);
				e.to.push_back(to);
				events.push_back(e);
			}
		}

		return events;
	}

	bool effect(GameLogic *logic, EventType, ServerPlayer *target, void *, ServerPlayer *invoker) const override
	{
		JudgeStruct judge(".|red");
		judge.who = invoker;
		logic->judge(judge);

		if (judge.matched) {
			JsonArray victims = invoker->tag.at("tieqi_victims").toArray();
			victims.push_back(target->id());
			invoker->tag["tieqi_victims"] = victims;
		}

		return false;
	}
};

class Jizhi : public TriggerSkill
{
public:
	Jizhi() : TriggerSkill("jizhi")
	{
		m_events.insert(TargetChosen);
	}

	EventList triggerable(GameLogic *, EventType, ServerPlayer *player, void *data, ServerPlayer *) const override
	{
		if (!TriggerSkill::triggerable(player))
			return EventList();

		CardUseStruct *use = static_cast<CardUseStruct *>(data);
		if (use->card && use->card->type() == CardType::Trick)
			return Event(this, player);
		return EventList();
	}

	bool effect(GameLogic *, EventType, ServerPlayer *target, void *, ServerPlayer *) const override
	{
		target->drawCards(1);
		return false;
	}
};

class Qicai : public CardModSkill
{
public:
	Qicai() : CardModSkill("qicai")
	{
		m_frequency = Compulsory;
	}

	int extraDistanceLimit(const Card *card, const std::vector<const Player *> &, const Player *, const Player *source) const override
	{
		if (card->type() == CardType::Trick && source->hasSkill(this))
			return Card::InfinityNum;
		return 0;
	}
};

}

void StandardPackage::addShuGenerals()
{
	// SHU 001
	General *liubei = new General("liubei", "shu", 4);
	liubei->setLord(true);
	liubei->addSkill(new Rende);
	addGeneral(liubei);

	// SHU 002
	General *guanyu = new General("guanyu", "shu", 5);
	guanyu->addSkill(new Wusheng);
	addGeneral(guanyu);

	// SHU 003
	General *zhangfei = new General("zhangfei", "shu", 4);
	zhangfei->addSkill(new Paoxiao);
	addGeneral(zhangfei);

	// SHU 004
	General *zhugeliang = new General("zhugeliang", "shu", 3);
	zhugeliang->addSkill(new Guanxing);
	zhugeliang->addSkill(new Kongcheng);
	addGeneral(zhugeliang);

	// SHU 005
	General *zhaoyun = new General("zhaoyun", "shu", 4);
	zhaoyun->addSkill(new Longdan);
	addGeneral(zhaoyun);

	// SHU 006
	General *machao = new General("machao", "shu", 4);
	machao->addSkill(new Tieqi);
	machao->addSkill(new Mashu);
	addGeneral(machao);

	// SHU 007
	General *huangyueying = new General("huangyueying", "shu", 3, Gender::Female);
	huangyueying->addSkill(new Jizhi);
	huangyueying->addSkill(new Qicai);
	addGeneral(huangyueying);
}
