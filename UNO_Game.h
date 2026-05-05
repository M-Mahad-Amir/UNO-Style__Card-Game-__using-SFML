// ============================================================
//  UNO_Game_v5.h  —  Declarations / Prototypes only
//
//  All implementations live in UNO_Game_v5.cpp.
//  Include this header in every translation unit that needs
//  game logic.

//  What changed from the console version:
//    • Removed all cout / cin / displaycardsinhand
//    • Removed runGame() / playerTurn() (GUI drives the loop)
//    • Added TurnPhase enum for GUI state-machine
//    • Added GUI-friendly action methods on GameManager
//    • startGame() now deals 7 cards (was 10, debug value)
//    • resetGame() lets the WIN screen restart without leaks
// ============================================================
// ============================================================
#pragma once

#include <string>
#include <vector>
#include <ctime>
#include <cstdlib>

using namespace std;

// ============================================================
//  ENUMS
// ============================================================

enum class Color    { RED, BLUE, GREEN, YELLOW, WILD };
enum class CardType { NUMBER, SKIP, REVERSE, DRAW_TWO, WILD, WILD_DRAW_FOUR };

// TurnPhase tells main.cpp exactly which screen / controls
// to show at any given moment.
enum class TurnPhase
{
    CHOOSE_ACTION,          // Normal: pick a card to play, or draw
    AWAITING_DRAW_DECISION, // Just drew — choose to play it or keep it
    CHOOSE_COLOR,           // Wild/WD4 played — must pick a colour
    PLAY_WILD_CARD,         // Colour chosen — must play a card of that colour
    GAME_OVER               // A player emptied their hand — game finished
};

// ============================================================
//  HELPER FUNCTIONS
// ============================================================

string colorToString(Color c);
string cardTypeToString(CardType t);

// ============================================================
//  CARD  (abstract base)
// ============================================================

class Card
{
protected:
    Color    color;
    CardType type;

public:
    Card(Color color, CardType type);
    virtual ~Card() = default;

    Color    getColor() const;
    CardType getType()  const;

    virtual string getLabel() const = 0;
    virtual int    getValue() const;
    virtual bool   operator==(const Card& other) const = 0;
};

// ============================================================
//  NORMAL CARD  (0-9)
// ============================================================

class NormalCard : public Card
{
    int value;
public:
    NormalCard(Color color, int v);

    int    getValue() const override;
    string getLabel() const override;
    bool   operator==(const Card& other) const override;
};

// ============================================================
//  SPECIAL CARD  (Skip / Reverse / Draw Two / Wild / WD4)
// ============================================================

class SpecialCard : public Card
{
public:
    SpecialCard(Color color, CardType type);

    string getLabel() const override;
    bool   operator==(const Card& other) const override;
};

// ============================================================
//  UNO DECK  (procedural card generator)
// ============================================================

class UnoDeck
{
    static bool seeded;   // defined in .cpp

public:
    static Card*         GenerateOneCard();
    static vector<Card*> GenerateCards(int count);
};

// ============================================================
//  PLAYER
// ============================================================

class Player
{
    string        name;
    vector<Card*> hand;
    bool          saidUno;

public:
    Player(const string& name);
    ~Player();

    // --- Getters ---
    string               getName()     const;
    bool                 getSaidUno()  const;
    int                  getHandSize() const;
    const vector<Card*>& getHand()     const;

    // --- Mutators ---
    void setSaidUno(bool val);
    void addCard(Card* card);
    void removeCard(Card* card);
};

// ============================================================
//  GAME MANAGER  (Singleton — one instance for the whole run)
// ============================================================

class GameManager
{
    // --- Singleton instance ---
    static GameManager* instance;   // defined in .cpp

    // --- Core state ---
    Card*           topCard          = nullptr;
    vector<Player*> players;
    int             currentPlayerIdx = 0;

    // --- GUI state-machine variables ---
    TurnPhase phase            = TurnPhase::CHOOSE_ACTION;
    Color     pendingWildColor = Color::RED;
    Card*     lastDrawnCard    = nullptr;
    Player*   winner           = nullptr;

    // Private constructor — use getInstance() instead.
    GameManager() = default;

    // Internal helpers
    void internalPlayCard(Player* p, Card* card);
    void applyEffect(Card* card);
    bool checkWin(Player* p);

public:
    static GameManager* getInstance();

    ~GameManager();

    // --- Setup ---
    void addPlayer(Player* p);
    void startGame();
    void resetGame();

    // --- Navigation ---
    Player* getCurrentPlayer();
    Player* getOtherPlayer();
    void    advanceTurn();
    int     getCurrentPlayerIdx() const;

    // --- State accessors ---
    TurnPhase getTurnPhase()        const;
    Card*     getTopCard()          const;
    Color     getPendingWildColor() const;
    Card*     getLastDrawnCard()    const;
    bool      isGameOver()          const;
    Player*   getWinner()           const;
    bool      hasCardsOfColor(Color c) const;

    // --- Playability check ---
    bool isPlayable(const Card* card) const;

    // --- Turn actions ---
    bool tryPlayCard(int handIndex);
    void drawCard();
    void playDrawnCard();
    void keepDrawnCard();
    void setChosenColor(Color c);
    bool tryPlayWildFollowUp(int handIndex);
    void skipWildFollowUp();
};
