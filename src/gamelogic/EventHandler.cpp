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

#include "common.h"
#include "EventHandler.h"

EventHandler::EventHandler()
	: m_defaultPriority(0)
	, m_compulsory(false)
{
}

EventHandler::~EventHandler()
{
}

int EventHandler::priority() const
{
	return m_defaultPriority;
}

int EventHandler::priority(EventType event) const
{
	return m_priorityMap.find(event) != m_priorityMap.end() ? m_priorityMap.at(event) : m_defaultPriority;
}

EventMap EventHandler::triggerable(GameLogic *logic, EventType event, ServerPlayer *owner, void *data) const
{
	EventMap result;
	EventList events = triggerable(logic, event, owner, data, owner);
	for (const Event &d : events)
		result.insert(std::make_pair(owner, d));
	return result;
}

EventList EventHandler::triggerable(GameLogic *logic, EventType event, ServerPlayer *owner, void *data, ServerPlayer *invoker) const
{
	KA_UNUSED(logic, event, data, invoker);

	EventList events;
	if (triggerable(owner))
		events.push_back(Event(this, owner));

	return events;
}

bool EventHandler::onCost(GameLogic *logic, EventType event, ServerPlayer *target, void *data, ServerPlayer *invoker) const
{
	KA_UNUSED(logic, event, target, data, invoker);
	return true;
}

bool EventHandler::effect(GameLogic *logic, EventType event, ServerPlayer *target, void *data, ServerPlayer *invoker) const
{
	KA_UNUSED(logic, event, target, data, invoker);
	return false;
}
