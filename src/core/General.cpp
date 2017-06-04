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

#include "General.h"

#include "Skill.h"

General::General(const std::string &name, const std::string &kingdom, int maxHp, Gender gender)
	: m_name(name)
	, m_kingdom(kingdom)
	, m_maxHp(maxHp)
	, m_gender(gender)
	, m_lord(false)
	, m_hidden(false)
	, m_neverShown(false)
	, m_headExtraMaxHp(0)
	, m_deputyExtraMaxHp(0)
{
}

General::~General()
{
	for (const Skill *skill : m_skills)
		delete skill;
}

bool General::isCompanionWith(const General *general) const
{
	if (m_companions.find(general->name()) != m_companions.end())
		return true;

	return kingdom() == general->kingdom() && (isLord() || general->isLord());
}

void General::addSkill(Skill *skill)
{
	m_skills.push_back(skill);
}

bool General::hasSkill(const Skill *skill) const
{
	for (const Skill *i : m_skills) {
		if (i == skill) {
			return true;
		}
	}
	return false;
}
