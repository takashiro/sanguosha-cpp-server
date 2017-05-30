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

#include "Event.h"

Event::Event()
    : handler(nullptr)
    , owner(nullptr)
{
}

Event::Event(const EventHandler *handler)
    : handler(handler)
    , owner(nullptr)
{
}

Event::Event(const EventHandler *handler, ServerPlayer *owner)
    : handler(handler)
    , owner(owner)
{
}

EventList::EventList(const Event &e)
{
    push_back(e);
}

EventMap::EventMap(const Event &e)
{
    insert(std::make_pair(e.owner, e));
}

EventMap::EventMap(const EventList &events)
{
	for (const Event &e : events) {
		insert(std::make_pair(e.owner, e));
	}
}
