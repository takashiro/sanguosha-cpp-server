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

#include "standardpackage.h"

#include "Engine.h"
#include "GameMode.h"

StandardPackage::StandardPackage()
	: Package("standard")
{
	addShuGenerals();
	addWeiGenerals();
	addWuGenerals();
	addQunGenerals();

	addBasicCards();
	addEquipCards();
	addTrickCards();
}

bool StandardPackage::isAvailable(const GameMode *mode) const
{
	return mode->name() == "standard";
}

ADD_PACKAGE(Standard)
