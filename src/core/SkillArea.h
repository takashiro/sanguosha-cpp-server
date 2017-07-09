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

#pragma once

#include "SkillAreaType.h"

#include <vector>

class Skill;

class SkillArea
{
public:
	using Type = SkillAreaType;

	SkillArea(Type type);
	~SkillArea();

	Type type() const { return m_type; }

	void add(const Skill *skill);
	bool remove(const Skill *skill);
	bool contains(const Skill *skill);

	const std::vector<const Skill *> &skills() const { return m_skills; }

	std::size_t size() const { return m_skills.size(); }

private:
	Type m_type;
	std::vector<const Skill *> m_skills;
};
