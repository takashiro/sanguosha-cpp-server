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

#include "HegemonyMode.h"

#include "GameRule.h"
#include "GameLogic.h"
#include "General.h"
#include "ServerPlayer.h"
#include "SystemPackage.h"

HegemonyMode::HegemonyMode()
{
	m_name = "hegemony";

	m_rules.push_back(new GameRule(PrepareToStart, [] (GameLogic *logic, ServerPlayer *target, void *) {
		std::map<uint, GeneralList> replies = logic->broadcastRequestForGenerals(logic->players(), 2, 7);
		for (const std::pair<uint, GeneralList> &iter : replies) {
			ServerPlayer *player = logic->findPlayer(iter.first);
			const GeneralList &generals = iter.second;
			if (generals.size() >= 2) {
				player->setHeadGeneral(generals.at(0));
				player->setDeputyGeneral(generals.at(1));
			}
		}
		return false;
	}));

	m_rules.push_back(new GameRule(GameStart, [] (GameLogic *, ServerPlayer *current, void *) {
		uint hiden_general = SystemPackage::HiddenGeneralId();
		current->broadcastProperty("head_general_id", hiden_general, current);
		current->broadcastProperty("deputy_general_id", hiden_general, current);

		current->unicastProperty("head_general_id", hiden_general, current);
		current->unicastProperty("deputy_general_id", hiden_general, current);

		const General *head_general = current->headGeneral();
		const General *deputy_general = current->deputyGeneral();

		int head_hp = head_general->headMaxHp();
		int deputy_hp = deputy_general->deputyMaxHp();
		int hp = (head_hp + deputy_hp) / 2;
		current->setMaxHp(hp);
		current->setHp(hp);
		current->broadcastProperty("maxHp", hp);
		current->broadcastProperty("hp", hp);

		current->setKingdom(head_general->kingdom());
		current->unicastProperty("kingdom", head_general->kingdom(), current);
		current->broadcastProperty("kingdom", "unknown", current);

		std::vector<const Skill *> skills = head_general->skills();
		for (const Skill *skill : skills)
			current->addSkill(skill, SkillAreaType::Head);
		skills = deputy_general->skills();
		for (const Skill *skill : skills)
			current->addSkill(skill, SkillAreaType::Deputy);

		current->drawCards(4);

		return false;
	}));
}
