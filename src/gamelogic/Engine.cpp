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

#include "Engine.h"

#include "Card.h"
#include "GameMode.h"
#include "Package.h"
#include "General.h"
#include "Skill.h"

#include <algorithm>

Engine::Engine()
{
}

Engine *Engine::instance()
{
	static Engine engine;
	return &engine;
}

Engine::~Engine()
{
	for (const GameMode *mode : m_modes)
		delete mode;

	for (const std::pair<std::string, Package *> &p : m_packages)
		delete p.second;
}

const GameMode *Engine::mode(const std::string &name) const
{
	for (const GameMode *mode : m_modes) {
		if (mode->name() == name)
			return mode;
	}

	return nullptr;
}

void Engine::addPackage(Package *package)
{
	m_packages[package->name()] = package;

	std::vector<const General *> generals = package->generals(true);
	for (const General *general : generals) {
		m_generals[general->id()] = general;

		std::vector<const Skill *> skills = general->skills();
		for (const Skill *skill : skills)
			m_skills[skill->id()] = skill;
	}

	std::vector<const Card *> cards = package->cards();
	for (const Card *card : cards)
		m_cards[card->id()] = card;
}

std::vector<const Package *> Engine::packages() const
{
	std::vector<const Package *> packages;
	packages.reserve(m_packages.size());
	for (const std::pair<std::string, Package *> &p : m_packages)
		packages.push_back(p.second);
	return packages;
}

std::vector<const Package *> Engine::getPackages(const GameMode *mode) const
{
	std::vector<const Package *> packages;
	for (const std::pair<std::string, Package *> &p : m_packages) {
		Package *package = p.second;
		if (package->isAvailable(mode))
			packages.push_back(package);
	}
	return packages;
}

std::vector<const General *> Engine::getGenerals(bool includeHidden) const
{
	std::vector<const General *> generals;
	for (const std::pair<std::string, Package *> &p : m_packages) {
		GeneralList list = p.second->generals(includeHidden);
		size_t offset = generals.size();
		generals.resize(generals.size() + list.size());
		std::copy(list.begin(), list.end(), generals.begin() + offset);
	}
	return generals;
}
