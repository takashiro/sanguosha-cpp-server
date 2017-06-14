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

#include "common.h"

#include <string>
#include <vector>

class Skill
{
	friend class General;

public:
	enum Frequency
	{
		NotFrequent,
		Frequent,
		Compulsory,
		Limited,
		Wake
	};

	enum Type
	{
		InvalidType,
		TriggerType,
		ViewAsType,
		ProactiveType
	};

	Skill(const std::string &name);
	virtual ~Skill();

	uint id() const { return m_id; }
	const std::string &name() const { return m_name; }
	Type type() const { return m_type; }
	int subtype() const { return m_subtype; }
	Frequency frequency() const { return m_frequency; }
	const std::vector<const Skill *> &children() const { return m_children; }
	bool isLordSkill() const { return m_lordSkill; }
	const Skill *parent() const;

protected:
	void addChild(Skill *child);

	uint m_id;
	std::string m_name;
	Type m_type;
	int m_subtype;
	Frequency m_frequency;
	bool m_lordSkill;

private:
	std::vector<const Skill *> m_children;
	const Skill *m_parent;
};
