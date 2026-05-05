// ============================================================
//  UNO_Game_v5.cpp  —  Full game logic implementations
//
//  All class method definitions live here.
//  Declarations / prototypes are in UNO_Game_v5.h.

//  What changed from the console version:
//    • Removed all cout / cin / displaycardsinhand
//    • Removed runGame() / playerTurn() (GUI drives the loop)
//    • Added TurnPhase enum for GUI state-machine
//    • Added GUI-friendly action methods on GameManager
//    • startGame() now deals 7 cards (was 10, debug value)
//    • resetGame() lets the WIN screen restart without leaks
// ============================================================
#include "UNO_Game_v5.h"

// ============================================================
//  STATIC MEMBER DEFINITIONS
// ============================================================

bool         UnoDeck::seeded       = false;
GameManager* GameManager::instance = nullptr;

// ============================================================
//  HELPER FUNCTIONS
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
// ============================================================

Card::Card(Color color, CardType type) : color(color), type(type) {}

Color    Card::getColor() const { return color; }
CardType Card::getType()  const { return type;  }
int      Card::getValue() const { return -1;    }

// ============================================================
//  NORMAL CARD  (0-9)
// ============================================================

NormalCard::NormalCard(Color color, int v) : Card(color, CardType::NUMBER), value(v) {}

int NormalCard::getValue() const { return value; }

string NormalCard::getLabel() const
{
    return colorToString(color) + " " + to_string(value);
}

bool NormalCard::operator==(const Card& other) const
{
    return color == other.getColor() || value == other.getValue();
}

// ============================================================
//  SPECIAL CARD  (Skip / Reverse / Draw Two / Wild / WD4)
// ============================================================

SpecialCard::SpecialCard(Color color, CardType type) : Card(color, type) {}

string SpecialCard::getLabel() const
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

bool SpecialCard::operator==(const Card& other) const
{
    return color == other.getColor() || type == other.getType();
}

// ============================================================
//  UNO DECK

//  56 distinct "slots": 40 number cards (4 colours × 0-9)
//  plus 4 Skip, 4 Reverse, 4 Draw Two, 2 Wild, 2 Wild Draw Four.
// ============================================================

Card* UnoDeck::GenerateOneCard()
// Returns a freshly heap-allocated card chosen at random.
// Caller (or GameManager) is responsible for eventual deletion.
{
    if (!seeded) { srand((unsigned)time(nullptr)); seeded = true; }
    int id = rand() % 56 + 1;

    // ---- BLUE  0-9  (id 1-10) ----
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

    // ---- GREEN  0-9  (id 11-20) ----
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

    // ---- YELLOW  0-9  (id 21-30) ----
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

    // ---- RED  0-9  (id 31-40) ----
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

    // ---- SKIP  x4  (id 41-44) ----
    if (id == 41) return new SpecialCard(Color::BLUE,   CardType::SKIP);
    if (id == 42) return new SpecialCard(Color::GREEN,  CardType::SKIP);
    if (id == 43) return new SpecialCard(Color::YELLOW, CardType::SKIP);
    if (id == 44) return new SpecialCard(Color::RED,    CardType::SKIP);

    // ---- REVERSE  x4  (id 45-48) ----
    if (id == 45) return new SpecialCard(Color::BLUE,   CardType::REVERSE);
    if (id == 46) return new SpecialCard(Color::GREEN,  CardType::REVERSE);
    if (id == 47) return new SpecialCard(Color::YELLOW, CardType::REVERSE);
    if (id == 48) return new SpecialCard(Color::RED,    CardType::REVERSE);

    // ---- DRAW TWO  x4  (id 49-52) ----
    if (id == 49) return new SpecialCard(Color::BLUE,   CardType::DRAW_TWO);
    if (id == 50) return new SpecialCard(Color::GREEN,  CardType::DRAW_TWO);
    if (id == 51) return new SpecialCard(Color::YELLOW, CardType::DRAW_TWO);
    if (id == 52) return new SpecialCard(Color::RED,    CardType::DRAW_TWO);

    // ---- WILD  x2  (id 53-54) ----
    if (id <= 54) return new SpecialCard(Color::WILD, CardType::WILD);

    // ---- WILD DRAW FOUR  x2  (id 55-56) ----
    return new SpecialCard(Color::WILD, CardType::WILD_DRAW_FOUR);
}

// Helper: generate 'count' cards at once.
vector<Card*> UnoDeck::GenerateCards(int count)
{
    vector<Card*> cards;
    for (int i = 0; i < count; i++)
        cards.push_back(GenerateOneCard());
    return cards;
}

// ============================================================
//  PLAYER
// ============================================================

Player::Player(const string& name) : name(name), saidUno(false) {}

Player::~Player()
{
    for (Card* c : hand) delete c;
}

string               Player::getName()     const { return name;             }
bool                 Player::getSaidUno()  const { return saidUno;          }
int                  Player::getHandSize() const { return (int)hand.size(); }
const vector<Card*>& Player::getHand()     const { return hand;             }

void Player::setSaidUno(bool val) { saidUno = val; }
void Player::addCard(Card* card)  { if (card) hand.push_back(card); }

// Remove the exact pointer from hand (does NOT delete the card —
// the caller becomes responsible for it, e.g. places it on topCard).
void Player::removeCard(Card* card)
{
    for (int i = 0; i < (int)hand.size(); i++) {
        if (hand[i] == card) {
            hand.erase(hand.begin() + i);
            if ((int)hand.size() != 1) saidUno = false;
            return;
        }
    }
}

// ============================================================
//  GAME MANAGER
// ============================================================

GameManager* GameManager::getInstance()
{
    if (!instance) instance = new GameManager();
    return instance;
}

GameManager::~GameManager()
{
    delete topCard;
    for (Player* p : players) delete p;
}

// ============================================================
//  SETUP
// ============================================================

// Add a player before calling startGame().
void GameManager::addPlayer(Player* p) { players.push_back(p); }

void GameManager::startGame()
{
    for (Player* p : players) {
        vector<Card*> dealt = UnoDeck::GenerateCards(7);   // 7 cards — official UNO rule
        for (Card* c : dealt) p->addCard(c);
    }

    // The first discard-pile card must never be a Wild.
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

void GameManager::resetGame()
{
    delete topCard;
    topCard = nullptr;
    for (Player* p : players) delete p;
    players.clear();

    winner        = nullptr;
    lastDrawnCard = nullptr;

    players.push_back(new Player("Player 1"));
    players.push_back(new Player("Player 2"));
    startGame();
}

// ============================================================
//  NAVIGATION
// ============================================================

Player* GameManager::getCurrentPlayer()          { return players[currentPlayerIdx];     }
Player* GameManager::getOtherPlayer()            { return players[1 - currentPlayerIdx]; }
void    GameManager::advanceTurn()               { currentPlayerIdx = 1 - currentPlayerIdx; }
int     GameManager::getCurrentPlayerIdx() const { return currentPlayerIdx; }

// ============================================================
//  STATE ACCESSORS
// ============================================================

TurnPhase GameManager::getTurnPhase()        const { return phase;            }
Card*     GameManager::getTopCard()          const { return topCard;          }
Color     GameManager::getPendingWildColor() const { return pendingWildColor; }
Card*     GameManager::getLastDrawnCard()    const { return lastDrawnCard;    }
bool      GameManager::isGameOver()          const { return phase == TurnPhase::GAME_OVER; }
Player*   GameManager::getWinner()           const { return winner;           }

// True if the current player has at least one card of colour c.
bool GameManager::hasCardsOfColor(Color c) const
{
    for (const Card* card : players[currentPlayerIdx]->getHand())
        if (card->getColor() == c) return true;
    return false;
}

// ============================================================
//  PLAYABILITY CHECK
// ============================================================

bool GameManager::isPlayable(const Card* card) const
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

// ============================================================
//  INTERNAL HELPERS
//  place card on discard pile and trigger rule effects. Resets phase to CHOOSE_ACTION first; applyEffect may override.
// ============================================================

void GameManager::internalPlayCard(Player* p, Card* card)
{
    p->removeCard(card);
    delete topCard;
    topCard = card;
    phase = TurnPhase::CHOOSE_ACTION;   // default, overridden below for Wilds
    applyEffect(card);
}

// ----------------------------------------------------------------
//  INTERNAL: rule side-effects after a card lands on the pile.
//
//  Skip / Reverse:
//    Pre-advance here so the caller's final advanceTurn() double-
//    toggles back → net effect = other player loses their turn.
//
//  Draw Two / Wild Draw Four:
//    Other player draws cards but keeps their regular turn.
//    No pre-advance → caller's single advanceTurn() gives other
//    player their normal turn (with extra cards in hand).
//
//  Wild / WD4:
//    Set phase to CHOOSE_COLOR so the GUI knows to show the
//    colour-picker overlay.  Turn does NOT advance yet.
// ----------------------------------------------------------------
void GameManager::applyEffect(Card* card)
{
    switch (card->getType())
    {
    case CardType::NUMBER:
        break;  // No side-effect.

    case CardType::SKIP:
    case CardType::REVERSE:             // Reverse = Skip in 2-player
        advanceTurn();                  // pre-advance trick
        break;

    case CardType::DRAW_TWO:
        getOtherPlayer()->addCard(UnoDeck::GenerateOneCard());
        getOtherPlayer()->addCard(UnoDeck::GenerateOneCard());
        // No advance — other player still gets their turn.
        break;

    case CardType::WILD:
        phase = TurnPhase::CHOOSE_COLOR;
        break;

    case CardType::WILD_DRAW_FOUR:
        // Other player draws 4, then current player picks a colour.
        for (int i = 0; i < 4; i++)
            getOtherPlayer()->addCard(UnoDeck::GenerateOneCard());
        phase = TurnPhase::CHOOSE_COLOR;
        break;

    default:
        break;
    }
}

// check if player p has won; set state accordingly. Returns true if the game just ended.
bool GameManager::checkWin(Player* p)
{
    if (p->getHandSize() == 0) {
        winner = p;
        phase  = TurnPhase::GAME_OVER;
        return true;
    }
    return false;
}

// ============================================================
//  TURN ACTIONS
// ============================================================

// --- Normal play: current player plays hand[handIndex] ---
// Returns false if the card cannot be played (GUI can flash an error).
bool GameManager::tryPlayCard(int handIndex)
{
    if (phase != TurnPhase::CHOOSE_ACTION) return false;

    Player* p = getCurrentPlayer();
    if (handIndex < 0 || handIndex >= p->getHandSize()) return false;

    Card* card = p->getHand()[handIndex];
    if (!isPlayable(card)) return false;

    internalPlayCard(p, card);

    if (checkWin(p)) return true;

    // Only advance the turn if we're not pausing for a Wild colour choice.
    if (phase == TurnPhase::CHOOSE_ACTION)
        advanceTurn();

    return true;
}

// --- Draw a card from the deck ---
// If the drawn card IS playable, moves to AWAITING_DRAW_DECISION so
// the GUI can offer "Play it" / "Keep it".
// If NOT playable, auto-keeps and immediately advances the turn.
void GameManager::drawCard()
{
    if (phase != TurnPhase::CHOOSE_ACTION) return;

    Player* p     = getCurrentPlayer();
    Card*   drawn = UnoDeck::GenerateOneCard();
    p->addCard(drawn);
    lastDrawnCard = drawn;   // remember so GUI can highlight it

    if (isPlayable(drawn)) {
        phase = TurnPhase::AWAITING_DRAW_DECISION;
    } else {
        // Not playable — auto-keep, pass turn. 
        // TO BE ADDED -> DISPLAY AN APPROPRIATE MESSAGE IN GUI.
        lastDrawnCard = nullptr;
        advanceTurn();
    }
}

void GameManager::playDrawnCard()
{
    if (phase != TurnPhase::AWAITING_DRAW_DECISION || !lastDrawnCard) return;

    Player* p    = getCurrentPlayer();
    Card*   card = lastDrawnCard;
    lastDrawnCard = nullptr;

    internalPlayCard(p, card);

    if (checkWin(p)) return;

    if (phase == TurnPhase::CHOOSE_ACTION)
        advanceTurn();
    // If phase == CHOOSE_COLOR (Wild drawn & played), GUI shows colour picker next.
}

void GameManager::keepDrawnCard()
{
    if (phase != TurnPhase::AWAITING_DRAW_DECISION) return;
    lastDrawnCard = nullptr;
    phase = TurnPhase::CHOOSE_ACTION;
    advanceTurn();
}

// --- Set the chosen Wild colour  (from CHOOSE_COLOR) ---
// Moves phase to PLAY_WILD_CARD so the GUI shows the filtered hand.
void GameManager::setChosenColor(Color c)
{
    if (phase != TurnPhase::CHOOSE_COLOR) return;
    pendingWildColor = c;
    phase = TurnPhase::PLAY_WILD_CARD;
}

// --- Play a card of the chosen colour as Wild follow-up  (PLAY_WILD_CARD) ---
// Returns false if the card colour does not match pendingWildColor.
bool GameManager::tryPlayWildFollowUp(int handIndex)
{
    if (phase != TurnPhase::PLAY_WILD_CARD) return false;

    Player* p = getCurrentPlayer();
    if (handIndex < 0 || handIndex >= p->getHandSize()) return false;

    Card* card = p->getHand()[handIndex];
    if (card->getColor() != pendingWildColor) return false;

    internalPlayCard(p, card);

    if (checkWin(p)) return true;

    if (phase == TurnPhase::CHOOSE_ACTION)
        advanceTurn();

    return true;
}

// --- Fallback: no matching colour cards exist — just end the turn ---
void GameManager::skipWildFollowUp()
{
    if (phase != TurnPhase::PLAY_WILD_CARD) return;
    phase = TurnPhase::CHOOSE_ACTION;
    advanceTurn();
}
