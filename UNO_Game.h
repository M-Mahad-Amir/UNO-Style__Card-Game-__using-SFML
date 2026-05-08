#pragma once
#include <string>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <iostream>
using namespace std;

// ============================================================
//  ENUMS
//  Define the core types used throughout the game.
//  Color:    the five possible card colours (4 playable + Wild).
//  CardType: the six kinds of cards in the deck.
//  TurnPhase: the state-machine states that control what the
//             current player is allowed to do on their turn.
// ============================================================

enum class Color    { RED, BLUE, GREEN, YELLOW, WILD };
enum class CardType { NUMBER, SKIP, REVERSE, DRAW_TWO, WILD, WILD_DRAW_FOUR };

enum class TurnPhase
{
    CHOOSE_ACTION,          // Player must play a card or draw
    AWAITING_DRAW_DECISION, // Player drew a playable card; choose to play or keep
    CHOOSE_COLOR,           // Player played a Wild; must pick a colour
    Play_Choosen_Card,      // Player must now play a card matching the chosen Wild colour
    GAME_OVER               // A player emptied their hand; game has ended
};

// ============================================================
//  HELPER FUNCTIONS
//  Convert enum values to human-readable strings for labels
//  and UI text. Used by getLabel() and the SFML frontend.
// ============================================================

string colorToString(Color c)
{
    switch (c) {
        case Color::RED:    return "Red";
        case Color::BLUE:   return "Blue";
        case Color::GREEN:  return "Green";
        case Color::YELLOW: return "Yellow";
        case Color::WILD:   return "Wild";
        default:            return "Unknown";
    }
}

string cardTypeToString(CardType t)
{
    switch (t) {
        case CardType::NUMBER:         return "Number";
        case CardType::SKIP:           return "Skip";
        case CardType::REVERSE:        return "Reverse";
        case CardType::DRAW_TWO:       return "Draw Two";
        case CardType::WILD:           return "Wild";
        case CardType::WILD_DRAW_FOUR: return "Wild Draw Four";
        default:                       return "Unknown";
    }
}

// ============================================================
//  CARD  (abstract base)
//  Stores the colour and type shared by every card.
//  getLabel() is pure virtual — each subclass formats its own
//  display string. getValue() returns -1 by default (only
//  NormalCard overrides it with a real number).
// ============================================================

class Card
{
protected:
    Color    color;
    CardType type;

public:
    Card(Color c, CardType t) : color(c), type(t) {}
    virtual ~Card() = default;

    Color    getColor() const { return color; }
    CardType getType()  const { return type;  }
    virtual int getValue() const { return -1; }

    virtual string getLabel() const = 0;
};

// ============================================================
//  NORMAL CARD  (0-9)
//  Concrete card for numbered cards. Stores an integer value
//  and builds its label as "Colour Number" (e.g. "Blue 7").
// ============================================================

class NormalCard : public Card
{
    int value;
public:
    NormalCard(Color color, int v) : Card(color, CardType::NUMBER), value(v) {}

    int getValue() const override { return value; }

    string getLabel() const override
    {
        return colorToString(color) + " " + to_string(value);
    }
};

// ============================================================
//  SPECIAL CARD
//  Covers Skip, Reverse, Draw Two, Wild, and Wild Draw Four.
//  getLabel() uses the stored CardType to build the correct
//  display string; Wild cards omit the colour prefix since
//  they are colourless until played.
// ============================================================

class SpecialCard : public Card
{
public:
    SpecialCard(Color color, CardType type) : Card(color, type) {}

    string getLabel() const override
    {
        switch (type) {
            case CardType::SKIP:           return colorToString(color) + " Skip";
            case CardType::REVERSE:        return colorToString(color) + " Reverse";
            case CardType::DRAW_TWO:       return colorToString(color) + " Draw Two";
            case CardType::WILD:           return "Wild";
            case CardType::WILD_DRAW_FOUR: return "Wild Draw Four";
            default:                       return "Unknown Special";
        }
    }
};

// ============================================================
//  UNO DECK
//  Static utility class — no instances needed.
//
//  GenerateOneCard(): seeds rand() once, then picks a random
//  id in [1..56] and maps it to a specific card:
//    ids  1-10  → Blue  0-9
//    ids 11-20  → Green 0-9
//    ids 21-30  → Yellow 0-9
//    ids 31-40  → Red   0-9
//    ids 41-44  → Skip  (one per colour)
//    ids 45-48  → Reverse (one per colour)
//    ids 49-52  → Draw Two (one per colour)
//    ids 53-54  → Wild
//    ids 55-56  → Wild Draw Four
//
//  GenerateCards(n): calls GenerateOneCard() n times and
//  returns the resulting vector — used for dealing hands.
// ============================================================

class UnoDeck
{
    static bool seeded;
public:
    static Card* GenerateOneCard()
    {
        if (!seeded) { srand((unsigned)time(nullptr)); seeded = true; }
        int id = rand() % 56 + 1;

        if (id == 1)  return new NormalCard(Color::BLUE, 0);
        if (id == 2)  return new NormalCard(Color::BLUE, 1);
        if (id == 3)  return new NormalCard(Color::BLUE, 2);
        if (id == 4)  return new NormalCard(Color::BLUE, 3);
        if (id == 5)  return new NormalCard(Color::BLUE, 4);
        if (id == 6)  return new NormalCard(Color::BLUE, 5);
        if (id == 7)  return new NormalCard(Color::BLUE, 6);
        if (id == 8)  return new NormalCard(Color::BLUE, 7);
        if (id == 9)  return new NormalCard(Color::BLUE, 8);
        if (id == 10) return new NormalCard(Color::BLUE, 9);

        if (id == 11) return new NormalCard(Color::GREEN, 0);
        if (id == 12) return new NormalCard(Color::GREEN, 1);
        if (id == 13) return new NormalCard(Color::GREEN, 2);
        if (id == 14) return new NormalCard(Color::GREEN, 3);
        if (id == 15) return new NormalCard(Color::GREEN, 4);
        if (id == 16) return new NormalCard(Color::GREEN, 5);
        if (id == 17) return new NormalCard(Color::GREEN, 6);
        if (id == 18) return new NormalCard(Color::GREEN, 7);
        if (id == 19) return new NormalCard(Color::GREEN, 8);
        if (id == 20) return new NormalCard(Color::GREEN, 9);

        if (id == 21) return new NormalCard(Color::YELLOW, 0);
        if (id == 22) return new NormalCard(Color::YELLOW, 1);
        if (id == 23) return new NormalCard(Color::YELLOW, 2);
        if (id == 24) return new NormalCard(Color::YELLOW, 3);
        if (id == 25) return new NormalCard(Color::YELLOW, 4);
        if (id == 26) return new NormalCard(Color::YELLOW, 5);
        if (id == 27) return new NormalCard(Color::YELLOW, 6);
        if (id == 28) return new NormalCard(Color::YELLOW, 7);
        if (id == 29) return new NormalCard(Color::YELLOW, 8);
        if (id == 30) return new NormalCard(Color::YELLOW, 9);

        if (id == 31) return new NormalCard(Color::RED, 0);
        if (id == 32) return new NormalCard(Color::RED, 1);
        if (id == 33) return new NormalCard(Color::RED, 2);
        if (id == 34) return new NormalCard(Color::RED, 3);
        if (id == 35) return new NormalCard(Color::RED, 4);
        if (id == 36) return new NormalCard(Color::RED, 5);
        if (id == 37) return new NormalCard(Color::RED, 6);
        if (id == 38) return new NormalCard(Color::RED, 7);
        if (id == 39) return new NormalCard(Color::RED, 8);
        if (id == 40) return new NormalCard(Color::RED, 9);

        if (id == 41) return new SpecialCard(Color::BLUE,   CardType::SKIP);
        if (id == 42) return new SpecialCard(Color::GREEN,  CardType::SKIP);
        if (id == 43) return new SpecialCard(Color::YELLOW, CardType::SKIP);
        if (id == 44) return new SpecialCard(Color::RED,    CardType::SKIP);

        if (id == 45) return new SpecialCard(Color::BLUE,   CardType::REVERSE);
        if (id == 46) return new SpecialCard(Color::GREEN,  CardType::REVERSE);
        if (id == 47) return new SpecialCard(Color::YELLOW, CardType::REVERSE);
        if (id == 48) return new SpecialCard(Color::RED,    CardType::REVERSE);

        if (id == 49) return new SpecialCard(Color::BLUE,   CardType::DRAW_TWO);
        if (id == 50) return new SpecialCard(Color::GREEN,  CardType::DRAW_TWO);
        if (id == 51) return new SpecialCard(Color::YELLOW, CardType::DRAW_TWO);
        if (id == 52) return new SpecialCard(Color::RED,    CardType::DRAW_TWO);

        if (id <= 54) return new SpecialCard(Color::WILD, CardType::WILD);
        return new SpecialCard(Color::WILD, CardType::WILD_DRAW_FOUR);
    }

    static vector<Card*> GenerateCards(int count)
    {
        vector<Card*> cards;
        for (int i = 0; i < count; i++)
            cards.push_back(GenerateOneCard());
        return cards;
    }
};

bool UnoDeck::seeded = false;

// ============================================================
//  PLAYER
//  Holds a player's name and their current hand (vector of
//  Card pointers). The destructor frees all cards in the hand
//  to avoid memory leaks when a Player object is deleted.
//
//  removeCard() searches by pointer identity and erases the
//  matching entry — used by GameManager when a card is played.
// ============================================================

class Player
{
    string        name;
    vector<Card*> hand;

public:
    Player(const string& n) : name(n) {}

    ~Player() { for (Card* c : hand) delete c; }

    string               getName()     const { return name;             }
    int                  getHandSize() const { return (int)hand.size(); }
    const vector<Card*>& getHand()     const { return hand;             }

    void addCard(Card* card) { if (card) hand.push_back(card); }

    void removeCard(Card* card)
    {
        for (int i = 0; i < (int)hand.size(); i++) {
            if (hand[i] == card) {
                hand.erase(hand.begin() + i);
                return;
            }
        }
    }
};

// ============================================================
//  GAME MANAGER  (Singleton)
//  Central controller for all game logic. Only one instance
//  ever exists (accessed via getInstance()). Owns the top card,
//  the player list, and the current turn state.
// ============================================================

class GameManager
{
    static GameManager* instance;

    Card*           topCard          = nullptr;
    vector<Player*> players;
    int             currentPlayerIdx = 0;
    TurnPhase       phase            = TurnPhase::CHOOSE_ACTION;
    Color           pendingWildColor = Color::RED;
    Card*           lastDrawnCard    = nullptr;
    Player*         winner           = nullptr;

    GameManager() = default;

    // ---- internalPlayCard ----
    // Moves a card from the player's hand to the discard pile
    // (deletes the old top card, sets the new one), resets the
    // phase to CHOOSE_ACTION, then runs applyEffect() to trigger
    // any special card behaviour.
    void internalPlayCard(Player* p, Card* card)
    {
        p->removeCard(card);
        delete topCard;
        topCard = card;
        phase = TurnPhase::CHOOSE_ACTION;
        applyEffect(card);
    }

    // ---- applyEffect ----
    // Reads the played card's type and applies its rule:
    //   NUMBER      → nothing extra.
    //   SKIP/REVERSE→ advance the turn (skip the other player).
    //   DRAW_TWO    → deal 2 cards to the opponent.
    //   WILD        → move to CHOOSE_COLOR phase.
    //   WILD_DRAW_4 → deal 4 cards to the opponent, then CHOOSE_COLOR.
    void applyEffect(Card* card)
    {
        switch (card->getType())
        {
        case CardType::NUMBER: break;

        case CardType::SKIP:
        case CardType::REVERSE:
            advanceTurn();
            break;

        case CardType::DRAW_TWO:
            getOtherPlayer()->addCard(UnoDeck::GenerateOneCard());
            getOtherPlayer()->addCard(UnoDeck::GenerateOneCard());
            break;

        case CardType::WILD:
            phase = TurnPhase::CHOOSE_COLOR;
            break;

        case CardType::WILD_DRAW_FOUR:
            for (int i = 0; i < 4; i++)
                getOtherPlayer()->addCard(UnoDeck::GenerateOneCard());
            phase = TurnPhase::CHOOSE_COLOR;
            break;

        default: break;
        }
    }

    // ---- checkWin ----
    // Called after every card play. If the player's hand is now
    // empty, records them as the winner and sets phase to GAME_OVER.
    bool checkWin(Player* p)
    {
        if (p->getHandSize() == 0) {
            winner = p;
            phase  = TurnPhase::GAME_OVER;
            return true;
        }
        return false;
    }

public:

    // ---- getInstance ----
    // Singleton accessor — creates the one instance on first call.
    static GameManager* getInstance()
    {
        if (!instance) instance = new GameManager();
        return instance;
    }

    ~GameManager()
    {
        delete topCard;
        for (Player* p : players) delete p;
    }

    // ---- Setup ----
    void addPlayer(Player* p) { players.push_back(p); }

    // ---- startGame ----
    // Deals 7 cards to each player, then draws a starting top card
    // (re-draws if it is a Wild/Wild Draw Four to ensure a valid start).
    // Resets all state variables to beginning-of-game defaults.
    void startGame()
    {
        for (Player* p : players) {
            vector<Card*> dealt = UnoDeck::GenerateCards(7);
            for (Card* c : dealt) p->addCard(c);
        }
        do {
            delete topCard;
            topCard = UnoDeck::GenerateOneCard();
        } while (topCard->getType() == CardType::WILD ||
                 topCard->getType() == CardType::WILD_DRAW_FOUR);

        phase            = TurnPhase::CHOOSE_ACTION;
        currentPlayerIdx = 0;
        winner           = nullptr;
        lastDrawnCard    = nullptr;
    }

    // ---- resetGame ----
    // Deletes the current players and their cards, creates a fresh
    // pair of players (Tom & Jerry), then calls startGame() to deal
    // new hands and reset all state — used by the "New Game" button.
    void resetGame()
    {
        delete topCard;
        topCard = nullptr;
        for (Player* p : players) delete p;
        players.clear();
        winner        = nullptr;
        lastDrawnCard = nullptr;
        players.push_back(new Player("Tom"));
        players.push_back(new Player("Jerry"));
        startGame();
    }

    // ---- Navigation ----
    // getCurrentPlayer / getOtherPlayer: index-based lookup for the
    // two-player game (indices 0 and 1).
    // advanceTurn: flips the index between 0 and 1.
    Player* getCurrentPlayer()          { return players[currentPlayerIdx];     }
    Player* getOtherPlayer()            { return players[1 - currentPlayerIdx]; }
    void    advanceTurn()               { currentPlayerIdx = 1 - currentPlayerIdx; }
    int     getCurrentPlayerIdx() const { return currentPlayerIdx; }

    // ---- State accessors ----
    // Simple getters exposing private state to the SFML frontend
    // without allowing it to mutate the game directly.
    TurnPhase getTurnPhase()        const { return phase;            }
    Card*     getTopCard()          const { return topCard;          }
    Color     getPendingWildColor() const { return pendingWildColor; }
    Card*     getLastDrawnCard()    const { return lastDrawnCard;    }
    Card*     get_lastDrawnCard()         { return lastDrawnCard;    }
    bool      isGameOver()          const { return phase == TurnPhase::GAME_OVER; }
    Player*   getWinner()           const { return winner;           }

    // ---- hasCardsOfColor ----
    // Returns true if the current player holds at least one card of
    // colour c. Used after a Wild to decide whether to enter the
    // Play_Choosen_Card phase or skip straight to the next turn.
    bool hasCardsOfColor(Color c) const
    {
        for (const Card* card : players[currentPlayerIdx]->getHand())
            if (card->getColor() == c) return true;
        return false;
    }

    // ---- isPlayable ----
    // A card is legal to play if any of these hold:
    //   • It is a Wild or Wild Draw Four (always playable).
    //   • Its colour matches the top card's colour.
    //   • Its type matches the top card's type (and for NUMBER
    //     cards, its value also matches).
    bool isPlayable(const Card* card) const
    {
        if (card->getType() == CardType::WILD ||
            card->getType() == CardType::WILD_DRAW_FOUR) return true;
        if (card->getColor() == topCard->getColor())      return true;
        if (card->getType() == topCard->getType()) {
            if (card->getType() == CardType::NUMBER)
                return card->getValue() == topCard->getValue();
            return true;
        }
        return false;
    }

    // ---- tryPlayCard ----
    // Called when the player clicks a hand card during CHOOSE_ACTION.
    // Validates the phase, index, and playability, then delegates to
    // internalPlayCard(). Checks for a win, then advances the turn
    // unless a Wild is waiting for colour selection.
    bool tryPlayCard(int handIndex)
    {
        if (phase != TurnPhase::CHOOSE_ACTION) return false;
        Player* p = getCurrentPlayer();
        if (handIndex < 0 || handIndex >= p->getHandSize()) return false;
        Card* card = p->getHand()[handIndex];
        if (!isPlayable(card)) return false;
        internalPlayCard(p, card);
        if (checkWin(p)) return true;
        if (phase == TurnPhase::CHOOSE_ACTION) advanceTurn();
        return true;
    }

    // ---- drawCard ----
    // Called when the player clicks "Draw Card" during CHOOSE_ACTION.
    // Generates one card and adds it to the hand.
    //   • If the drawn card is playable → phase moves to AWAITING_DRAW_DECISION
    //     so the player can choose to play it or keep it.
    //   • If not playable → turn immediately advances to the other player.
    void drawCard()
    {
        if (phase != TurnPhase::CHOOSE_ACTION) return;
        Player* p     = getCurrentPlayer();
        Card*   drawn = UnoDeck::GenerateOneCard();
        p->addCard(drawn);
        lastDrawnCard = drawn;
        if (isPlayable(drawn))
            phase = TurnPhase::AWAITING_DRAW_DECISION;
        else {
            lastDrawnCard = nullptr;
            advanceTurn();
        }
    }

    // ---- playDrawnCard ----
    // Player chose "Play It" after drawing a playable card.
    // Clears lastDrawnCard (so the highlight disappears), plays the
    // card via internalPlayCard(), checks for a win, then advances
    // the turn if no Wild colour selection is needed.
    void playDrawnCard()
    {
        if (phase != TurnPhase::AWAITING_DRAW_DECISION || !lastDrawnCard) return;
        Player* p    = getCurrentPlayer();
        Card*   card = lastDrawnCard;
        lastDrawnCard = nullptr;
        internalPlayCard(p, card);
        if (checkWin(p)) return;
        if (phase == TurnPhase::CHOOSE_ACTION) advanceTurn();
    }

    // ---- keepDrawnCard ----
    // Player chose "Keep It" after drawing a playable card.
    // Clears the draw highlight, resets phase to CHOOSE_ACTION,
    // then hands the turn over to the other player.
    void keepDrawnCard()
    {
        if (phase != TurnPhase::AWAITING_DRAW_DECISION) return;
        lastDrawnCard = nullptr;
        phase = TurnPhase::CHOOSE_ACTION;
        advanceTurn();
    }

    // ---- setChosenColor ----
    // Called when the player clicks a colour square after playing a Wild.
    // Stores the chosen colour in pendingWildColor and moves to the
    // Play_Choosen_Card phase, where the same player must play a matching card.
    void setChosenColor(Color c)
    {
        if (phase != TurnPhase::CHOOSE_COLOR) return;
        pendingWildColor = c;
        phase = TurnPhase::Play_Choosen_Card;
    }

    // ---- tryPlayWildFollowUp ----
    // During Play_Choosen_Card phase, the player must play a card whose
    // colour matches pendingWildColor. Only Wilds are exempt (handled
    // elsewhere). If valid, plays the card and checks for a win.
    bool tryPlayWildFollowUp(int handIndex)
    {
        if (phase != TurnPhase::Play_Choosen_Card) return false;
        Player* p = getCurrentPlayer();
        if (handIndex < 0 || handIndex >= p->getHandSize()) return false;
        Card* card = p->getHand()[handIndex];
        if (card->getColor() != pendingWildColor) return false;
        internalPlayCard(p, card);
        if (checkWin(p)) return true;
        if (phase == TurnPhase::CHOOSE_ACTION) advanceTurn();
        return true;
    }
};
GameManager* GameManager::instance = nullptr;