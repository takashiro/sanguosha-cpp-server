#include "test.h"

#include "CardPattern.h"

#include "Card.h"
#include "CardArea.h"
#include "Player.h"

namespace UnitTest
{
	class TestCard : public Card
	{
	public:
		TestCard(Suit suit = CardSuit::Spade, int number = 10)
			: Card(suit, number)
		{
			m_id = 1;
			m_name = "test";
		}

		SGS_CARD(TestCard, Card)
	};

	TEST_CLASS(CardPatternTest)
	{
	public:

		TEST_METHOD(Empty)
		{
			TestCard card;
			CardPattern p("");
			assert(!p.match(nullptr, &card));
		}

		TEST_METHOD(TypeOnly1)
		{
			TestCard card;
			CardPattern p("test");
			assert(p.match(nullptr, &card));
		}

		TEST_METHOD(TypeOnly2)
		{
			TestCard card;
			CardPattern p("test2");
			assert(!p.match(nullptr, &card));
		}

		TEST_METHOD(WildcardType)
		{
			TestCard card;
			CardPattern p(".");
			assert(p.match(nullptr, &card));
		}

		TEST_METHOD(Suit1)
		{
			TestCard card;
			CardPattern p(".|spade");
			assert(p.match(nullptr, &card));
		}

		TEST_METHOD(Suit2)
		{
			TestCard card;
			CardPattern p(".|heart");
			assert(!p.match(nullptr, &card));
		}

		TEST_METHOD(NotSuit1)
		{
			TestCard card;
			CardPattern p(".|^spade");
			assert(!p.match(nullptr, &card));
		}

		TEST_METHOD(NotSuit2)
		{
			TestCard card;
			CardPattern p(".|^heart");
			assert(p.match(nullptr, &card));
		}

		TEST_METHOD(Number)
		{
			TestCard card;
			CardPattern p(".|spade|10");
			assert(p.match(nullptr, &card));
		}

		TEST_METHOD(NumberRange)
		{
			TestCard card;
			CardPattern p("test|spade|9-12");
			assert(p.match(nullptr, &card));
		}

		TEST_METHOD(MultipleExpression1)
		{
			TestCard card;
			CardPattern p("test2#test|.|10");
			assert(p.match(nullptr, &card));
		}

		TEST_METHOD(MultipleExpression2)
		{
			TestCard card;
			CardPattern p("test2#test|.|11");
			assert(!p.match(nullptr, &card));
		}

		TEST_METHOD(WildcardSuit)
		{
			TestCard card;
			CardPattern p("test|.");
			assert(p.match(nullptr, &card));
		}

		TEST_METHOD(WildcardNumber)
		{
			TestCard card;
			CardPattern p("test|.|.");
			assert(p.match(nullptr, &card));
		}

		TEST_METHOD(EquipArea)
		{
			TestCard card1;
			Player player(0);
			player.equipArea()->add(&card1);
			TestCard card2;

			CardPattern p("test|.|.|equipped");
			assert(p.match(&player, &card1));
			assert(!p.match(&player, &card2));
		}

		TEST_METHOD(HandCardArea)
		{
			TestCard card;
			Player player(0);

			CardPattern p("test|.|.|hand");
			assert(!p.match(&player, &card));
			player.handcardArea()->add(&card);
			assert(p.match(&player, &card));
		}
	};
}
