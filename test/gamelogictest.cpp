#include "CppUnitTest.h"

#include "GameLogic.h"
#include "EventHandler.h"
#include "ServerPlayer.h"

#include <User.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#define assert(exp) Assert::IsTrue(exp)

KA_USING_NAMESPACE

namespace
{
	class AvoidDamage : public EventHandler
	{
	public:
		AvoidDamage()
		{
			m_events.insert(Damaging);
			m_compulsory = true;
			m_defaultPriority = 1;
		}

		bool triggerable(ServerPlayer *target) const override
		{
			return target != nullptr;
		}

		bool effect(GameLogic *logic, EventType event, ServerPlayer *, void *data, ServerPlayer *) const override
		{
			return true;
		}
	};

	class DamageBuff : public EventHandler
	{
	public:
		DamageBuff()
		{
			m_events.insert(Damaging);
			m_compulsory = true;
		}

		bool triggerable(ServerPlayer *target) const override
		{
			return target != nullptr;
		}

		bool effect(GameLogic *logic, EventType event, ServerPlayer *, void *data, ServerPlayer *) const override
		{
			DamageStruct *damage = static_cast<DamageStruct *>(data);
			damage->damage++;
			return false;
		}
	};
}

namespace test
{
	TEST_CLASS(GameLogicTest)
	{
	public:

		TEST_METHOD(AddPlayerTest)
		{
			User user[2];
			GameLogic logic;
			logic.addPlayer(&user[0]);
			logic.addPlayer(&user[1]);

			assert(logic.countPlayer() == 2);
		}

		TEST_METHOD(DamageTest)
		{
			User user[2];
			GameLogic logic;
			logic.addPlayer(&user[0]);
			logic.addPlayer(&user[1]);

			const std::vector<ServerPlayer *> &player = logic.players();
			player[0]->setHp(3);
			player[1]->setHp(4);

			DamageStruct damage;
			damage.from = player[0];
			damage.to = player[1];
			logic.damage(damage);

			assert(player[1]->hp() == 3);
		}

		TEST_METHOD(LoseHpTest)
		{
			User user[2];
			GameLogic logic;
			logic.addPlayer(&user[0]);
			logic.addPlayer(&user[1]);

			const std::vector<ServerPlayer *> &player = logic.players();
			player[0]->setHp(3);
			player[1]->setHp(4);

			logic.loseHp(player[1], 4);

			assert(player[1]->hp() == 0);
		}

		TEST_METHOD(BreakDamageTest)
		{
			User user[2];
			GameLogic logic;
			logic.addPlayer(&user[0]);
			logic.addPlayer(&user[1]);

			const std::vector<ServerPlayer *> &player = logic.players();
			player[0]->setHp(3);
			player[1]->setHp(4);

			AvoidDamage avoid_damage;
			logic.addEventHandler(&avoid_damage);

			DamageStruct damage;
			damage.from = player[0];
			damage.to = player[1];
			logic.damage(damage);

			assert(player[1]->hp() == 4);
		}

		TEST_METHOD(BreakEventTest)
		{
			User user[2];
			GameLogic logic;
			logic.addPlayer(&user[0]);
			logic.addPlayer(&user[1]);

			const std::vector<ServerPlayer *> &player = logic.players();
			player[0]->setHp(3);
			player[1]->setHp(4);

			AvoidDamage avoid_damage;
			DamageBuff damage_buff;
			logic.addEventHandler(&avoid_damage);
			logic.addEventHandler(&damage_buff);

			DamageStruct damage;
			damage.from = player[0];
			damage.to = player[1];
			logic.damage(damage);

			assert(player[1]->hp() == 4);
		}
	};
}
