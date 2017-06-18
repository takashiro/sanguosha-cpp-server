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

#include "Card.h"
#include "CardArea.h"
#include "ServerPlayer.h"
#include "GameLogic.h"
#include "General.h"

#include "skills.h"

KA_USING_NAMESPACE

namespace {

class Jianxiong : public MasochismSkill
{
public:
	Jianxiong() : MasochismSkill("jianxiong")
	{
	}

	int triggerable(GameLogic *, ServerPlayer *, DamageStruct &damage) const override
	{
		if (damage.card)
			return 1;

		return 0;
	}

	bool effect(GameLogic *logic, ServerPlayer *target, DamageStruct &damage) const override
	{
		CardsMoveStruct obtain;
		obtain << damage.card;
		obtain.to.owner = target;
		obtain.to.type = CardAreaType::Hand;
		obtain.isOpen = true;
		logic->moveCards(obtain);
		return false;
	}
};

class Fankui : public MasochismSkill
{
public:
	Fankui() : MasochismSkill("fankui")
	{
	}

	int triggerable(GameLogic *, ServerPlayer *, DamageStruct &damage) const override
	{
		if (damage.from && damage.from->handcardNum() + damage.from->equipNum() > 0)
			return 1;

		return 0;
	}

	bool effect(GameLogic *logic, ServerPlayer *target, DamageStruct &damage) const override
	{
		const Card *card = target->askToChooseCard(damage.from, "he");
		if (card) {
			CardsMoveStruct obtain;
			obtain << card;
			obtain.to.owner = target;
			obtain.to.type = CardAreaType::Hand;
			logic->moveCards(obtain);
		}

		return false;
	}
};

class Guicai : public ProactiveSkill
{
	class Timing : public TriggerSkill
	{
	public:
		Timing() : TriggerSkill("guicai")
		{
			m_events.insert(AskForRetrial);
			setFrequency(Compulsory);
		}

		bool cost(GameLogic *, EventType, ServerPlayer *target, void *data, ServerPlayer *) const override
		{
			JudgeStruct *judge = static_cast<JudgeStruct *>(data);
			target->showPrompt("guicai_ask_for_retrial", judge->who);
			return target->askToUseCard("@guicai");
		}

		bool effect(GameLogic *logic, EventType, ServerPlayer *target, void *data, ServerPlayer *) const override
		{
			uint cardId = target->tag["guicai_card"].toUInt();
			target->tag.erase("guicai_card");
			const Card *card = logic->findCard(cardId);
			if (card) {
				JudgeStruct *judge = static_cast<JudgeStruct *>(data);
				judge->card = card;
				judge->matched = judge->pattern.match(judge->who, judge->card);
			}
			return false;
		}
	};

public:
	Guicai() : ProactiveSkill("guicai")
	{
		addChild(new Timing);
	}

	bool isAvailable(const Player *self, const std::string &pattern) const override
	{
		return self->handcardNum() > 0 && pattern == "@guicai";
	}

	bool cardFilter(const std::vector<const Card *> &selected, const Card *card, const Player *source, const std::string &) const override
	{
		return selected.empty() && source->handcardArea()->contains(card);
	}

	bool cardFeasible(const std::vector<const Card *> &selected, const Player *) const override
	{
		return selected.size() == 1;
	}

	bool cost(GameLogic *logic, ServerPlayer *from, const std::vector<ServerPlayer *> &, const std::vector<const Card *> &cards) const override
	{
		const Card *card = cards.front();
		CardResponseStruct response;
		response.card = card;
		response.from = from;
		return logic->respondCard(response);
	}

	void effect(GameLogic *, ServerPlayer *from, const std::vector<ServerPlayer *> &, const std::vector<const Card *> &cards) const override
	{
		from->tag["guicai_card"] = cards.front()->id();
	}
};

class Luoshen : public TriggerSkill
{
public:
	Luoshen() : TriggerSkill("luoshen")
	{
		m_events.insert(PhaseStart);
	}

	bool triggerable(ServerPlayer *owner) const override
	{
		return owner->phase() == PlayerPhase::Start && TriggerSkill::triggerable(owner);
	}

	bool effect(GameLogic *logic, EventType event, ServerPlayer *target, void *data, ServerPlayer *invoker) const override
	{
		JudgeStruct judge(".|black");
		judge.who = target;

		for (;;) {
			logic->judge(judge);
			if (judge.matched) {
				CardsMoveStruct obtain;
				obtain << judge.card;
				obtain.to.owner = target;
				obtain.to.type = CardAreaType::Hand;
				logic->moveCards(obtain);

				Event option(this, target);
				Event choice = target->askForTriggerOrder(option, true);
				if (!choice.isValid() || choice.handler != this) {
					break;
				}

				bool takeEffect = cost(logic, event, target, data, invoker);
				if (takeEffect) {
					invoker->addSkillHistory(this);
				}
			} else {
				break;
			}
		}

		return false;
	}
};

class Qingguo : public OneCardViewAsSkill
{
public:
	Qingguo() : OneCardViewAsSkill("qingguo")
	{
	}

	bool viewFilter(const Card *card, const Player *, const std::string &) const override
	{
		return card->color() == CardColor::Black;
	}

	const Card *viewAs(const Card *subcard, const Player *) const override
	{
		Jink *jink = new Jink(subcard->suit(), subcard->number());
		jink->setSkill(this);
		jink->addSubcard(subcard);
		return jink;
	}

	bool isAvailable(const Player *, const std::string &pattern) const override
	{
		return pattern == "Jink";
	}
};

class Ganglie : public MasochismSkill
{
public:
	Ganglie() : MasochismSkill("ganglie")
	{
	}

	int triggerable(GameLogic *, ServerPlayer *, DamageStruct &) const override
	{
		return 1;
	}

	bool effect(GameLogic *logic, ServerPlayer *target, DamageStruct &damage) const override
	{
		JudgeStruct judge(".|^heart");
		judge.who = target;
		logic->judge(judge);

		if (judge.matched && damage.from) {
			damage.from->showPrompt("ganglie_discard_cards", target);
			std::vector<const Card *> cards = damage.from->askForCards(".|.|.|hand", 2, true);
			if (cards.size() == 2) {
				CardsMoveStruct discard;
				discard.cards = cards;
				discard.to.type = CardAreaType::DiscardPile;
				discard.isOpen = true;
				logic->moveCards(discard);
			} else {
				DamageStruct revenge;
				revenge.from = target;
				revenge.to = damage.from;
				logic->damage(revenge);
			}
		}

		return false;
	}
};

class Tuxi : public ProactiveSkill
{
	class Timing : public TriggerSkill
	{
	public:
		Timing() : TriggerSkill("tuxi")
		{
			m_events.insert(PhaseStart);
		}

		bool triggerable(ServerPlayer *owner) const override
		{
			return TriggerSkill::triggerable(owner) && owner->phase() == PlayerPhase::Draw;
		}

		bool cost(GameLogic *, EventType, ServerPlayer *target, void *, ServerPlayer *) const override
		{
			target->showPrompt("tuxi_select_players");
			return target->askToUseCard("@tuxi");
		}

		bool effect(GameLogic *logic, EventType, ServerPlayer *from, void *, ServerPlayer *) const override
		{
			if (from->tag.find("tuxi_victims") == from->tag.end())
				return false;

			std::vector<ServerPlayer *> victims;
			JsonArray data = from->tag.at("tuxi_victims").toArray();
			victims.reserve(data.size());
			for (const Json &player_id : data) {
				ServerPlayer *victim = logic->findPlayer(player_id.toUInt());
				if (victim)
					victims.push_back(victim);
			}
			logic->sortByActionOrder(victims);

			for (ServerPlayer *victim : victims) {
				const Card *card = from->askToChooseCard(victim, "h");

				CardsMoveStruct obtain;
				obtain << card;
				obtain.to.owner = from;
				obtain.to.type = CardAreaType::Hand;
				logic->moveCards(obtain);
			}

			return true;
		}
	};

public:
	Tuxi() : ProactiveSkill("tuxi")
	{
		addChild(new Timing);
	}

	bool isAvailable(const Player *, const std::string &pattern) const override
	{
		return pattern == "@tuxi";
	}

	bool playerFilter(const std::vector<const Player *> &selected, const Player *toSelect, const Player *source) const override
	{
		return selected.size() < 2 && toSelect != source && toSelect->handcardNum() > 0;
	}

	bool playerFeasible(const std::vector<const Player *> &selected, const Player *) const override
	{
		int num = static_cast<int>(selected.size());
		return 1 <= num && num <= 2;
	}

	void effect(GameLogic *, ServerPlayer *from, const std::vector<ServerPlayer *> &to, const std::vector<const Card *> &) const override
	{
		JsonArray victims;
		for (ServerPlayer *victim : to) {
			victims.push_back(victim->id());
		}
		from->tag["tuxi_victims"] = victims;
	}
};

class Luoyi : public TriggerSkill
{
	class Effect : public TriggerSkill
	{
	public:
		Effect() : TriggerSkill("luoyi")
		{
			m_events.insert(Damaging);
			setFrequency(Compulsory);
		}

		EventList triggerable(GameLogic *, EventType, ServerPlayer *owner, void *data, ServerPlayer *) const override
		{
			if (!TriggerSkill::triggerable(owner) || owner->tag.find("luoyi_effect") == owner->tag.end())
				return EventList();

			DamageStruct *damage = static_cast<DamageStruct *>(data);
			if (damage->card && (damage->card->is("Slash") || damage->card->is("Duel")) && damage->to != owner)
				damage->damage++;

			return EventList();
		}
	};

	class Reset : public TriggerSkill
	{
	public:
		Reset() : TriggerSkill("luoyi")
		{
			m_events.insert(PhaseEnd);
			setFrequency(Compulsory);
		}

		bool triggerable(ServerPlayer *owner) const override
		{
			if (TriggerSkill::triggerable(owner) && owner->phase() == PlayerPhase::Play && owner->tag.find("luoyi_effect") != owner->tag.end())
				owner->tag.erase("luoyi_effect");
			return false;
		}
	};

public:
	Luoyi() : TriggerSkill("luoyi")
	{
		m_events.insert(DrawNCards);

		addChild(new Effect);
		addChild(new Reset);
	}

	bool effect(GameLogic *, EventType, ServerPlayer *target, void *data, ServerPlayer *) const override
	{
		int *drawNum = static_cast<int *>(data);
		*drawNum--;

		target->tag["luoyi_effect"] = true;

		return false;
	}
};

class Tiandu : public TriggerSkill
{
public:
	Tiandu() : TriggerSkill("tiandu")
	{
		m_events.insert(FinishJudge);
	}

	EventList triggerable(GameLogic *, EventType, ServerPlayer *player, void *data, ServerPlayer *) const override
	{
		if (TriggerSkill::triggerable(player)) {
			JudgeStruct *judge = static_cast<JudgeStruct *>(data);
			if (judge->card && player->judgeCards()->contains(judge->card))
				return Event(this, player);
		}
		return EventList();
	}

	bool effect(GameLogic *logic, EventType, ServerPlayer *player, void *data, ServerPlayer *) const override
	{
		JudgeStruct *judge = static_cast<JudgeStruct *>(data);
		CardsMoveStruct obtain;
		obtain << judge->card;
		obtain.to.owner = player;
		obtain.to.type = CardAreaType::Hand;
		logic->moveCards(obtain);
		return false;
	}
};

} //namespace

void StandardPackage::addWeiGenerals()
{
	// WEI 001
	General *caocao = new General("caocao", "wei", 4);
	caocao->setLord(true);
	caocao->addSkill(new Jianxiong);
	addGeneral(caocao);

	// WEI 002
	General *simayi = new General("simayi", "wei", 3);
	simayi->addSkill(new Fankui);
	simayi->addSkill(new Guicai);
	addGeneral(simayi);

	// WEI 003
	General *xiahoudun = new General("xiahoudun", "wei", 4);
	xiahoudun->addSkill(new Ganglie);
	addGeneral(xiahoudun);

	// WEI 004
	General *zhangliao = new General("zhangliao", "wei", 4);
	zhangliao->addSkill(new Tuxi);
	addGeneral(zhangliao);

	// WEI 005
	General *xuchu = new General("xuchu", "wei", 4);
	xuchu->addSkill(new Luoyi);
	addGeneral(xuchu);

	// WEI 006
	General *guojia = new General("guojia", "wei", 3);
	guojia->addSkill(new Tiandu);
	addGeneral(guojia);

	// WEI 007
	General *zhenji = new General("zhenji", "wei", 3, Gender::Female);
	zhenji->addSkill(new Luoshen);
	zhenji->addSkill(new Qingguo);
	addGeneral(zhenji);
}
