#include "Player.h"

class Player
{
private:
    string name;         // Player's display name
    vector<Card *> hand; // Cards currently held by this player
    bool saidUno;        // tracks whether player called UNO when hand hit 1 card

public:
    // Constructor: sets player identity, hand starts empty
    Player(const string &name)
        : name(name), saidUno(false) {}

    // Destructor: Player owns its Card* pointers so we clean them up here.
    // When GameManager moves a card to discard pile via removeCard(),
    // it takes back the pointer so no double-free happens.
    ~Player()
    {
        for (Card *c : hand)
            delete c;
        hand.clear();
    }

    // --- Getters ---
    string getName() const { return name; }
    bool getSaidUno() const { return saidUno; }
    int getHandSize() const { return hand.size(); }

    // Returns a const reference so GameManager/UIManager can read
    // the hand for rendering and valid-move checking  but cannot modify it directly.
    const vector<Card *> &getHand() const { return hand; }

    // --- Mutators ---
    void setSaidUno(bool val) { saidUno = val; }

    // Adds a drawn card into this player's hand.
    // Called by GameManager whenever a card is dealt or drawn from pile.
    void addCard(Card *card)
    {
        if (card)
            hand.push_back(card);
    }

    // Removes and returns the card at the given hand index.
    // Returns nullptr if index is out of range.
    // GameManager calls this when the player plays a card,
    // then passes the returned pointer to the discard pile.

    // note commented by Mutahir
    //     Card* removeCard(int index) {
    //        if (index < 0 || index >= (int)hand.size()) return nullptr;
    //          Card* played = hand[index];
    //         hand.erase(hand.begin() + index);
    //         // Auto-reset UNO flag whenever hand size moves away from 1
    //         if (hand.size() != 1) saidUno = false;
    //         return played;
    //     }
    void RemovefromHands(Card *c)
    {
        for (int i = 0; i < hand.size(); i++)
        {
            if (hand[i] == c)
            {
                hand.erase(hand.begin() + i);
                return;
            }
        }
    }
    // Checks whether the player holds at least one card that can legally
    // be played on top of topCard given the currentColor.
    // GameManager uses this to decide whether to force a draw.
    // currentColor is passed separately because after a Wild is played,
    // the active color is chosenColor — not topCard->getColor().
    bool hasPlayableCard(const Card *topCard, Color currentColor) const
    {
        for (const Card *c : hand)
        {
            // Wild cards are always playable regardless of color or type
            if (c->getType() == CardType::WILD ||
                c->getType() == CardType::WILD_DRAW_FOUR)
                return true;
            // Same color as the current active color
            if (c->getColor() == currentColor)
                return true;
            // Same type (symbol match: Skip on Skip, Reverse on Reverse, etc.)
            if (c->getType() == topCard->getType())
            {
                if (c->getType() == CardType::NUMBER)
                {
                    if (c->getValue() == topCard->getValue())
                        return true;
                }
                else
                {
                    return true; // same special type is always a valid match
                }
            }
        }
        return false;
    }
    // added by Mutahir temporary
    Card *cmp(Card *c)
    {
        for (Card *c1 : hand)
        {
            if (*c1 == *c) // this need to be changed
            {
                return c1;
            }
        }
        return nullptr;
    }
};