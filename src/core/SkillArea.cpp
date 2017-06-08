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

#include "SkillArea.h"

#include <algorithm>

SkillArea::SkillArea(Type type)
	: m_type(type)
{
}

SkillArea::~SkillArea()
{
}

void SkillArea::add(const Skill *skill)
{
	m_skills.push_back(skill);
}

bool SkillArea::remove(const Skill *skill)
{
	auto i = std::find(m_skills.begin(), m_skills.end(), skill);
	if (i != m_skills.end()) {
		m_skills.erase(i);
		return true;
	}

	return false;
}

bool SkillArea::contains(const Skill *skill)
{
	return std::find(m_skills.begin(), m_skills.end(), skill) != m_skills.end();
}
