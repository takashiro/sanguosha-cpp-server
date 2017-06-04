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

#include <string>
#include <vector>

class EventHandler;

class GameMode
{
public:
	GameMode();
	virtual ~GameMode();

	const std::string &name() const { return m_name; }
	int minPlayerNum() const { return m_minPlayerNum; }
	int maxPlayerNum() const { return m_maxPlayerNum; }

	const std::vector<const EventHandler *> &rules() const { return m_rules; }

protected:
	std::string m_name;
	int m_minPlayerNum;
	int m_maxPlayerNum;
	std::vector<const EventHandler *> m_rules;
};
