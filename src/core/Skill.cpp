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

#include "Skill.h"

Skill::Skill(const std::string &name)
	: m_id(0)
	, m_name(name)
	, m_type(InvalidType)
	, m_subtype(InvalidType)
	, m_frequency(NotFrequent)
	, m_lordSkill(false)
	, m_parent(this)
{
	// Id is only assigned to skills directly attached to a general. Subskill id is 0.
}

Skill::~Skill()
{
	for (const Skill *child : m_children) {
		delete child;
	}
}

const Skill *Skill::parent() const
{
	return m_parent;
}

void Skill::addChild(Skill *subskill)
{
	subskill->m_parent = this;
	m_children.push_back(subskill);
}
