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

#include "EventType.h"
#include "Event.h"

#include <map>
#include <set>
#include <string>

class GameLogic;
class Player;

class EventHandler
{
public:
	EventHandler();
	virtual ~EventHandler() = 0;

	const std::set<EventType> &events() const { return m_events; }

	const std::string name() const { return m_name; }

	int priority() const;
	int priority(EventType event) const;

	bool isCompulsory() const { return m_compulsory; }

	virtual bool triggerable(ServerPlayer *owner) const = 0;
	virtual EventMap triggerable(GameLogic *logic, EventType event, ServerPlayer *owner, void *data) const;
	virtual EventList triggerable(GameLogic *logic, EventType event, ServerPlayer *owner, void *data, ServerPlayer *invoker) const;
	virtual bool onCost(GameLogic *logic, EventType event, ServerPlayer *target, void *data, ServerPlayer *invoker = nullptr) const;
	virtual bool effect(GameLogic *logic, EventType event, ServerPlayer *target, void *data, ServerPlayer *invoker = nullptr) const;

protected:
	std::set<EventType> m_events;
	std::string m_name;
	int m_defaultPriority;
	std::map<EventType, int> m_priorityMap;
	bool m_compulsory;
};
