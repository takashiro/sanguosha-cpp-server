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
#include "Gender.h"

#include <set>
#include <vector>
#include <string>

class Skill;

class General
{
	friend class Package;

public:
	General(const std::string &name, const std::string &kingdom, int maxHp, Gender gender = Gender::Male);
	~General();

	uint id() const { return m_id; }

	void setName(const std::string &name) { m_name = name; }
	const std::string &name() const { return m_name; }

	void setKingdom(const std::string &kingdom) { m_kingdom = kingdom; }
	const std::string &kingdom() const { return m_kingdom; }

	void setMaxHp(int maxHp) { m_maxHp = maxHp; }
	int maxHp() const { return m_maxHp; }

	void setGender(Gender gender) { m_gender = gender; }
	Gender gender() const { return m_gender; }

	void setLord(bool lord) { m_lord = lord; }
	bool isLord() const { return m_lord; }

	void setHidden(bool hidden) { m_hidden = hidden; }
	bool isHidden() const { return m_hidden; }

	void setNeverShown(bool neverShown) { m_neverShown = neverShown; }
	bool isNeverShown() const { return m_neverShown; }

	void setHeadExtraMaxHp(int maxHp) { m_headExtraMaxHp = maxHp; }
	int headExtraMaxHp() const { return m_headExtraMaxHp; }
	int headMaxHp() const { return maxHp() + headExtraMaxHp(); }

	void setDeputyExtraMaxHp(int maxHp) { m_deputyExtraMaxHp = maxHp; }
	int deputyExtraMaxHp() const { return m_deputyExtraMaxHp; }
	int deputyMaxHp() const { return maxHp() + deputyExtraMaxHp(); }

	void addCompanion(const std::string &companion) { m_companions.insert(companion); }
	std::set<std::string> companions() const { return m_companions; }
	bool isCompanionWith(const General *general) const;

	void addSkill(Skill *skill);
	bool hasSkill(const Skill *skill) const;
	std::vector<const Skill *> skills() const { return m_skills; }

private:
	int m_id;
	std::string m_name;
	std::string m_kingdom;
	int m_maxHp;
	Gender m_gender;
	bool m_lord;
	bool m_hidden;
	bool m_neverShown;

	int m_headExtraMaxHp;
	int m_deputyExtraMaxHp;

	std::set<std::string> m_companions;

	std::vector<const Skill *> m_skills;
};

typedef std::vector<const General *> GeneralList;
