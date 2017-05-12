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

enum EventType
{
	InvalidEvent,

	GameStart,
	GameFinish,

	TurnStart,
	PhaseStart,
	PhaseProceeding,
	PhaseEnd,
	PhaseChanging,
	PhaseSkipping,
	TurnBroken,
	StageChange,

	BeforeCardsMove,
	CardsMove,
	AfterCardsMove,

	DrawNCards,
	AfterDrawNCards,
	CountMaxCardNum,

	PreCardUsed,
	CardUsed,
	TargetChoosing,
	TargetConfirming,
	TargetChosen,
	TargetConfirmed,
	CardEffect,
	CardEffected,
	PostCardEffected,
	CardFinished,
	TrickCardCanceling,

	CardResponded,

	SlashEffect,
	SlashEffected,
	SlashProceed,
	SlashHit,
	SlashMissed,

	ConfirmDamage, // confirm the source, weight and nature of a damage
	BeforeDamage,  // trigger certain skill -- jueqing
	DamageStart,
	Damaging,
	Damaged,
	AfterDamaging,
	AfterDamaged,
	DamageComplete,

	BeforeRecover,
	AfterRecover,

	HpLost,
	AfterHpLost,

	BeforeHpReduced,
	AfterHpReduced,

	MaxHpChanged,

	StartJudge,
	AskForRetrial,
	FinishRetrial,
	FinishJudge,

	SkillAdded,
	SkillRemoved,

	EnterDying,
	QuitDying,
	AskForPeach,
	AskForPeachDone,
	BeforeGameOverJudge,
	GameOverJudge,
	Died,
	BuryVictim,

	EventTypeCount
};
