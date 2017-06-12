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
#include "standard-trickcard.h"
#include "skills.h"

#include "CardArea.h"
#include "GameLogic.h"
#include "General.h"
#include "ServerPlayer.h"

namespace{

class Qingnang: public ProactiveSkill
{
public:
	Qingnang() : ProactiveSkill("qingnang")
	{
	}

	bool isAvailable(const Player *self, const std::string &pattern) const override
	{
		return ProactiveSkill::isAvailable(self, pattern) && self->skillHistory(this) <= 0;
	}

	bool cardFilter(const std::vector<const Card *> &selected, const Card *card, const Player *source, const std::string &) const override
	{
		const CardArea *hand = source->handcardArea();
		return selected.empty() && hand->contains(card);
	}

	bool cardFeasible(const std::vector<const Card *> &selected, const Player *) const override
	{
		return selected.size() == 1;
	}

	bool playerFilter(const std::vector<const Player *> &selected, const Player *toSelect, const Player *) const override
	{
		return selected.empty() && toSelect->isWounded();
	}

	bool playerFeasible(const std::vector<const Player *> &selected, const Player *) const override
	{
		return selected.size() == 1;
	}

	bool cost(GameLogic *logic, ServerPlayer *, const std::vector<ServerPlayer *> &, const std::vector<Card *> &cards) const override
	{
		CardsMoveStruct move;
		move.cards = cards;
		move.to.type = CardAreaType::DiscardPile;
		move.isOpen = true;
		logic->moveCards(move);

		return true;
	}

	void effect(GameLogic *logic, ServerPlayer *from, const std::vector<ServerPlayer *> &to, const std::vector<Card *> &) const override
	{
		RecoverStruct recover;
		recover.from = from;
		recover.to = to.front();
		logic->recover(recover);
	}
};

class Jijiu : public OneCardViewAsSkill
{
public:
	Jijiu() : OneCardViewAsSkill("jijiu")
	{
	}

	bool isAvailable(const Player *self, const std::string &pattern) const override
	{
		return self->phase() == PlayerPhase::Inactive && pattern == "Peach";
	}

	bool viewFilter(const Card *card, const Player *, const std::string &pattern) const override
	{
		return card->color() == CardColor::Red && pattern == "Peach";
	}

	Card *viewAs(Card *card, const Player *) const override
	{
		Peach *peach = new Peach(card->suit(), card->number());
		peach->addSubcard(card);
		peach->setSkill(this);
		return peach;
	}
};

class Wushuang : public TriggerSkill
{
public:
	Wushuang() : TriggerSkill("wushuang")
	{
		m_events.insert(SlashEffect);
		m_events.insert(CardResponded);
		setFrequency(Compulsory);
	}

	EventMap triggerable(GameLogic *, EventType event, ServerPlayer *, void *data) const override
	{
		EventMap events;
		if (event == SlashEffect) {
			SlashEffectStruct *effect = static_cast<SlashEffectStruct *>(data);
			if (effect->from && effect->from->hasSkill(this)) {
				Event e(this, effect->from);
				e.to.push_back(effect->to);
				events.insert(std::make_pair(effect->from, e));
			}
		} else if (event == CardResponded) {
			CardResponseStruct *response = static_cast<CardResponseStruct *>(data);
			if (response->target && response->target->is("Duel")) {
				if (response->to && response->to->hasSkill(this)) {
					if (response->from->tag["wushuang_slash"].toBool()) {
						response->from->tag.erase("wushuang_slash");
					} else {
						Event e(this, response->to);
						e.to.push_back(response->from);
						events.insert(std::make_pair(response->to, e));
					}
				}
			}
		}

		return events;
	}

	bool effect(GameLogic *logic, EventType event, ServerPlayer *, void *data, ServerPlayer *) const override
	{
		if (event == SlashEffect) {
			SlashEffectStruct *effect = static_cast<SlashEffectStruct *>(data);
			effect->jinkNum = 2;
		} else if (event == CardResponded) {
			CardResponseStruct *response = static_cast<CardResponseStruct *>(data);

			response->from->showPrompt("duel-slash", response->to);
			Card *slash = response->from->askForCard("Slash");
			if (slash == nullptr)
				return true;

			response->from->tag["wushuang_slash"] = true;
			CardResponseStruct extra;
			extra.card = slash;
			extra.from = response->from;
			extra.target = response->target;
			extra.to = response->to;
			return logic->respondCard(extra);
		}

		return false;
	}
};

class Lijian: public ProactiveSkill
{
public:
	Lijian() : ProactiveSkill("lijian")
	{
	}

	bool isAvailable(const Player *self, const std::string &pattern) const override
	{
		return ProactiveSkill::isAvailable(self, pattern) && self->skillHistory(this) <= 0;
	}

	bool cardFilter(const std::vector<const Card *> &selected, const Card *, const Player *, const std::string &) const override
	{
		return selected.empty();
	}

	bool cardFeasible(const std::vector<const Card *> &selected, const Player *) const
	{
		return selected.size() == 1;
	}

	bool playerFilter(const std::vector<const Player *> &selected, const Player *toSelect, const Player *source) const
	{
		if (toSelect == source)
			return false;

		const General *general = toSelect->general();
		return general && general->gender() == Gender::Male && selected.size() < 2;
	}

	bool playerFeasible(const std::vector<const Player *> &selected, const Player *) const
	{
		return selected.size() == 2;
	}

	bool cost(GameLogic *logic, ServerPlayer *, const std::vector<ServerPlayer *> &, const std::vector<Card *> &cards) const override
	{
		CardsMoveStruct move;
		move.cards = cards;
		move.to.type = CardAreaType::DiscardPile;
		move.isOpen = true;
		logic->moveCards(move);

		return true;
	}

	void effect(GameLogic *logic, ServerPlayer *, const std::vector<ServerPlayer *> &to, const std::vector<Card *> &) const override
	{
		Duel *duel = new Duel(CardSuit::None, 0);
		duel->setSkill(this);

		CardUseStruct use;
		use.from = to.at(1);
		use.to.push_back(to.at(0));
		use.card = duel;
		logic->useCard(use);
	}
};

class Biyue: public TriggerSkill
{
public:
	Biyue() : TriggerSkill("biyue")
	{
		m_events.insert(PhaseStart);
	}

	bool triggerable(ServerPlayer *owner) const override
	{
		return TriggerSkill::triggerable(owner) && owner->phase() == PlayerPhase::Finish;
	}

	bool effect(GameLogic *, EventType, ServerPlayer *target, void *, ServerPlayer *) const override
	{
		target->drawCards(1);
		return false;
	}
};

}

void StandardPackage::addQunGenerals()
{
	// QUN 001
	General *huatuo = new General("huatuo", "qun", 3);
	huatuo->addSkill(new Qingnang);
	huatuo->addSkill(new Jijiu);
	addGeneral(huatuo);

	// QUN 002
	General *lvbu = new General("lvbu", "qun", 4);
	lvbu->addSkill(new Wushuang);
	addGeneral(lvbu);

	// QUN 003
	General *diaochan = new General("diaochan", "qun", 3, Gender::Female);
	diaochan->addSkill(new Lijian);
	diaochan->addSkill(new Biyue);
	addGeneral(diaochan);
}
