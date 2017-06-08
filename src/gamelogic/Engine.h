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
#include <map>

class Card;
class General;
class Package;
class Skill;
class GameMode;

class Engine
{
public:
	static Engine *instance();
	~Engine();

	void addMode(GameMode *mode) { m_modes.push_back(mode); }
	const GameMode *mode(const std::string &name) const;
	std::vector<const GameMode *> modes() const { return m_modes; }

	void addPackage(Package *package);
	const Package *package(const std::string &name) const { return m_packages.at(name); }
	std::vector<const Package *> packages() const;
	std::vector<const Package *> getPackages(const GameMode *mode) const;

	std::vector<const General *> getGenerals(bool includeHidden = false) const;
	const General *getGeneral(uint id) const { return m_generals.at(id); }

	std::vector<const Card *> getCards() const;
	const Card *getCard(uint id) const { return m_cards.at(id); }

	const Skill *getSkill(uint id) const { return m_skills.at(id); }

private:
	Engine();

	std::vector<const GameMode *> m_modes;
	std::map<std::string, Package *> m_packages;
	std::map<uint, const General *> m_generals;
	std::map<uint, const Card *> m_cards;
	std::map<uint, const Skill *> m_skills;
};

#define ADD_PACKAGE(name) namespace\
{\
struct PackageAdder\
{\
	PackageAdder()\
	{\
		Engine::instance()->addPackage(new name##Package);\
	}\
};\
PackageAdder __packageAdder__;\
}

#define ADD_MODE(name) namespace\
{\
struct ModeAdder\
{\
	ModeAdder()\
	{\
		Engine::instance()->addMode(new name##Mode);\
	}\
};\
ModeAdder __modeAdder__;\
}

