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

#include "hegstandardpackage.h"

#include "Card.h"
#include "CardArea.h"
#include "GameLogic.h"
#include "General.h"
#include "ServerPlayer.h"

#include "skills.h"
#include "structs.h"

#include <algorithm>

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

	bool cost(GameLogic *logic, ServerPlayer *, const std::vector<ServerPlayer *> &to, const std::vector<const Card *> &cards) const override
	{
		if (to.empty() || cards.empty())
			return false;

		CardsMoveStruct move;
		move.cards = cards;
		move.to.owner = to.front();
		move.to.type = CardAreaType::Hand;
		logic->moveCards(move);
		return true;
	}

	void effect(GameLogic *logic, ServerPlayer *from, const std::vector<ServerPlayer *> &, const std::vector<const Card *> &cards) const override
	{
		int oldValue = from->tag["rende_count"].toInt();
		int newValue = oldValue + static_cast<int>(cards.size());
		from->tag["rende_count"] = newValue;
		if (oldValue < 3 && newValue >= 3 && from->isWounded()) {
			RecoverStruct recover;
			recover.from = from;
			recover.to = from;
			logic->recover(recover);
		}
	}
};

class Jizhi : public TriggerSkill
{
public:
	Jizhi() : TriggerSkill("heg_jizhi")
	{
		m_events.insert(TargetChosen);
	}

	EventList triggerable(GameLogic *, EventType, ServerPlayer *player, void *data, ServerPlayer *) const override
	{
		if (!TriggerSkill::triggerable(player))
			return EventList();

		CardUseStruct *use = static_cast<CardUseStruct *>(data);
		if (use->card && use->card->type() == CardType::Trick && use->card->subtype() != TrickCard::DelayedType && !use->card->isVirtual())
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
	Qicai() : CardModSkill("heg_qicai")
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

class Kongcheng : public TriggerSkill
{
public:
	Kongcheng() : TriggerSkill("heg_kongcheng")
	{
		m_events.insert(TargetConfirming);
		setFrequency(Compulsory);
	}

	EventList triggerable(GameLogic *, EventType, ServerPlayer *player, void *data, ServerPlayer *) const override
	{
		if (TriggerSkill::triggerable(player) && player->handcardArea()->size() <= 0) {
			CardUseStruct *use = static_cast<CardUseStruct *>(data);
			if (use->card && (use->card->is("Slash") || use->card->is("Duel")) && std::find(use->to.begin(), use->to.end(), player) != use->to.end()) {
				return Event(this);
			}
		}
		return EventList();
	}

	bool effect(GameLogic *, EventType, ServerPlayer *player, void *data, ServerPlayer *) const override
	{
		CardUseStruct *use = static_cast<CardUseStruct *>(data);
		auto i = std::find(use->to.begin(), use->to.end(), player);
		if (i != use->to.end()) {
			use->to.erase(i);
		}
		return false;
	}
};

}

void HegStandardPackage::addShuGenerals()
{
	General *liubei = new General("liubei", "shu", 4);
	liubei->addSkill(new Rende);
	addGeneral(liubei);

	General *huangyueying = new General("huangyueying", "shu", 3, Gender::Female);
	huangyueying->addSkill(new Jizhi);
	huangyueying->addSkill(new Qicai);
	addGeneral(huangyueying);

	General *zhugeliang = new General("zhugeliang", "shu", 3);
	zhugeliang->addSkill(new Kongcheng);
	addGeneral(zhugeliang);

	General *guanyu = new General("guanyu", "shu", 5);
	addGeneral(guanyu);

	General *zhangfei = new General("zhangfei", "shu", 4);
	addGeneral(zhangfei);

	General *zhaoyun = new General("zhaoyun", "shu", 4);
	addGeneral(zhaoyun);

	General *huangzhong = new General("huangzhong", "shu", 4);
	addGeneral(huangzhong);

	General *weiyan = new General("weiyan", "shu", 4);
	addGeneral(weiyan);

	General *pangtong = new General("pangtong", "shu", 3);
	addGeneral(pangtong);

	General *wolong = new General("wolong", "shu", 3);
	addGeneral(wolong);

	General *liushan = new General("liushan", "shu", 3);
	addGeneral(liushan);

	General *menghuo = new General("menghuo", "shu", 4);
	addGeneral(menghuo);

	General *zhurong = new General("zhurong", "shu", 4, Gender::Female);
	addGeneral(zhurong);

	General *ganfuren = new General("ganfuren", "shu", 3, Gender::Female);
	addGeneral(ganfuren);
}
