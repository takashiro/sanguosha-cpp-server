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

#include <protocol.h>

enum class cmd
{
	Invalid = KA_IMPORT net::NetworkCommandCount,

	ShowPrompt,
	ArrangeSeat,
	PrepareCards,
	ModifyPlayer,
	ChooseGeneral,
	MoveCards,
	UseCard,
	AddCardHistory,
	Damage,
	LoseHp,
	Recover,
	AskForCard,
	ShowAmazingGrace,
	TakeAmazingGrace,
	ClearAmazingGrace,
	ChoosePlayerCard,
	ShowCard,
	AddSkill,
	RemoveSkill,
	InvokeSkill,
	ClearSkillHistory,
	TriggerOrder,
	ArrangeCard,
	ArrangeCardStart,
	ArrangeCardMove,
	ArrangeCardEnd,
	AskForOption,
	AddVirtualCard,
	RemoveVirtualCard,
	GameOver,
	Act,

	CommandCount
};
