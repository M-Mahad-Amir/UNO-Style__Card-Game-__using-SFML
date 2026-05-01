#include "Deck.h"

class UnoDeck // simple factory
{
    static bool seed;

public:
    static vector<Card *> GenerateCards(int NoOfCards)
    {
        vector<Card *> PlayerCards;
        for (int i = 0; i < NoOfCards; i++)
        {
            PlayerCards.push_back(GenerateOneCard());
        }
        return PlayerCards;
    }
    static Card *GenerateOneCard()
    {
        if (seed == false)
        {
            srand(time(NULL));
            seed = true;
        }
        int id = rand() % 56 + 1;

        // BLUE 0-9 ? id 1-10
        if (id == 1)
            return new NormalCard(Color::BLUE, 0);
        if (id == 2)
            return new NormalCard(Color::BLUE, 1);
        if (id == 3)
            return new NormalCard(Color::BLUE, 2);
        if (id == 4)
            return new NormalCard(Color::BLUE, 3);
        if (id == 5)
            return new NormalCard(Color::BLUE, 4);
        if (id == 6)
            return new NormalCard(Color::BLUE, 5);
        if (id == 7)
            return new NormalCard(Color::BLUE, 6);
        if (id == 8)
            return new NormalCard(Color::BLUE, 7);
        if (id == 9)
            return new NormalCard(Color::BLUE, 8);
        if (id == 10)
            return new NormalCard(Color::BLUE, 9);

        // GREEN 0-9 ? id 11-20
        if (id == 11)
            return new NormalCard(Color::GREEN, 0);
        if (id == 12)
            return new NormalCard(Color::GREEN, 1);
        if (id == 13)
            return new NormalCard(Color::GREEN, 2);
        if (id == 14)
            return new NormalCard(Color::GREEN, 3);
        if (id == 15)
            return new NormalCard(Color::GREEN, 4);
        if (id == 16)
            return new NormalCard(Color::GREEN, 5);
        if (id == 17)
            return new NormalCard(Color::GREEN, 6);
        if (id == 18)
            return new NormalCard(Color::GREEN, 7);
        if (id == 19)
            return new NormalCard(Color::GREEN, 8);
        if (id == 20)
            return new NormalCard(Color::GREEN, 9);

        // YELLOW 0-9 ? id 21-30
        if (id == 21)
            return new NormalCard(Color::YELLOW, 0);
        if (id == 22)
            return new NormalCard(Color::YELLOW, 1);
        if (id == 23)
            return new NormalCard(Color::YELLOW, 2);
        if (id == 24)
            return new NormalCard(Color::YELLOW, 3);
        if (id == 25)
            return new NormalCard(Color::YELLOW, 4);
        if (id == 26)
            return new NormalCard(Color::YELLOW, 5);
        if (id == 27)
            return new NormalCard(Color::YELLOW, 6);
        if (id == 28)
            return new NormalCard(Color::YELLOW, 7);
        if (id == 29)
            return new NormalCard(Color::YELLOW, 8);
        if (id == 30)
            return new NormalCard(Color::YELLOW, 9);

        // RED 0-9 ? id 31-40
        if (id == 31)
            return new NormalCard(Color::RED, 0);
        if (id == 32)
            return new NormalCard(Color::RED, 1);
        if (id == 33)
            return new NormalCard(Color::RED, 2);
        if (id == 34)
            return new NormalCard(Color::RED, 3);
        if (id == 35)
            return new NormalCard(Color::RED, 4);
        if (id == 36)
            return new NormalCard(Color::RED, 5);
        if (id == 37)
            return new NormalCard(Color::RED, 6);
        if (id == 38)
            return new NormalCard(Color::RED, 7);
        if (id == 39)
            return new NormalCard(Color::RED, 8);
        if (id == 40)
            return new NormalCard(Color::RED, 9);

        // id 41-54 ? special cards (fill in next)
        // SKIP x4 ? id 41-44
        if (id == 41)
            return new SpecialCard(Color::BLUE, CardType::SKIP);
        if (id == 42)
            return new SpecialCard(Color::GREEN, CardType::SKIP);
        if (id == 43)
            return new SpecialCard(Color::YELLOW, CardType::SKIP);
        if (id == 44)
            return new SpecialCard(Color::RED, CardType::SKIP);

        // REVERSE x4 ? id 45-48
        if (id == 45)
            return new SpecialCard(Color::BLUE, CardType::REVERSE);
        if (id == 46)
            return new SpecialCard(Color::GREEN, CardType::REVERSE);
        if (id == 47)
            return new SpecialCard(Color::YELLOW, CardType::REVERSE);
        if (id == 48)
            return new SpecialCard(Color::RED, CardType::REVERSE);

        // DRAW TWO x4 ? id 49-52
        if (id == 49)
            return new SpecialCard(Color::BLUE, CardType::DRAW_TWO);
        if (id == 50)
            return new SpecialCard(Color::GREEN, CardType::DRAW_TWO);
        if (id == 51)
            return new SpecialCard(Color::YELLOW, CardType::DRAW_TWO);
        if (id == 52)
            return new SpecialCard(Color::RED, CardType::DRAW_TWO);

        if (id == 53 || id == 54)
            return new SpecialCard(Color::WILD, CardType::WILD);

        // if id ==55 or 56 then this will execute
        return new SpecialCard(Color::WILD, CardType::WILD_DRAW_FOUR);
    }
};
bool UnoDeck::seed = false;