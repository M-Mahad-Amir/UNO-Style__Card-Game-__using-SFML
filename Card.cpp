#include "Card.h"

class Card
{
protected:
    Color color;   // The card's color (or WILD for colorless)
    CardType type; // What kind of card this is
    string id;     // Unique identifier, e.g. "RED_9_1", "WILD_0"
                   // Critical for the infinite draw pile tracking �
                   // the deck picks a random ID and "draws" that card.

public:
    // Constructor � sets the core identity of every card.
    Card(Color color, CardType type)
        : color(color), type(type) {}

    // Virtual destructor � always required when you have virtual functions.
    // Without this, deleting a Card* that points to a NormalCard would
    // only call Card's destructor, not NormalCard's. Memory leak.
    virtual ~Card() = default;

    // --- Getters (pure data access, no logic) ---
    Color getColor() const { return color; }
    CardType getType() const { return type; }
    string getId() const { return id; }
    // Returns a human-readable label: "Red 7", "Skip", "Wild", etc.
    // Used for console output now; will feed into SFML text rendering later.
    virtual string getLabel() const = 0;

    // Applies this card's effect to the game.
    // Takes GameManager by reference so it can modify game state.
    // We use a forward declaration trick here � GameManager is defined
    // NOTE: We're passing a placeholder int& for now just to show the
    // signature. Replace with GameManager& once that class exists.
    virtual void applyEffect() const = 0;

    // A convenient print method for debugging.
    // Virtual so derived classes can extend it if needed.
    virtual void print() const
    {
        cout << "[" << colorToString(color) << "] "
             << getLabel()
             << "  (ID: " << id << ")\n";
    }
    virtual int getValue() const { return -1; }
    virtual bool operator==(const Card &other) const = 0;
};

// -------------------------------------------------------------
// SECTION 3 � DERIVED CLASS: NormalCard  (Numbers 0�9)
//
// INHERITANCE: NormalCard IS-A Card. It gets color, type, id,
// and all the getters for free. It only adds what's unique:
// the numeric face value.
// -------------------------------------------------------------

class NormalCard : public Card
{
private:
    int value; // The face value: 0 through 9
public:
    bool operator==(const Card &other) const
    {
        return color == other.getColor() || value == other.getValue();
    }
    // Constructor � passes color/type/id up to Card, stores value here.
    NormalCard(Color color, int v)
        : Card(color, CardType::NUMBER), value(v)
    {

        // Defensive check � catch bad data at construction time.
        //        if (value < 0 || value > 9) {
        //            throw invalid_argument("NormalCard value must be 0�9");
        //        }
    }

    int getValue() const override { return value; }

    // OVERRIDE: Label is just "Color + Number", e.g. "Red 7".
    string getLabel() const override
    {
        return colorToString(color) + " " + to_string(value);
    }

    // OVERRIDE: Normal cards have no special effect.
    // applyEffect is a no-op here � the game just advances the turn normally.
    void applyEffect() const override
    {
        // No effect � GameManager handles turn advance for all cards.
        // Nothing to do here for a NormalCard.
    }
};

// -------------------------------------------------------------
// SECTION 4 � DERIVED CLASS: SpecialCard  (Action + Wild cards)
//
// One class handles ALL special cards: Skip, Reverse, Draw Two,
// Wild, and Wild Draw Four. The cardType enum field tells us
// which variant this instance is.
// -------------------------------------------------------------

class SpecialCard : public Card
{
private:
    // For Wild cards, the player gets to pick the color at play time.
    // chosenColor, or color, starts as WILD and gets set by GameManager when played.
    Color chosenColor;

public:
    bool operator==(const Card &other) const
    {
        return color == other.getColor() || type == other.getType();
    }
    // Constructor � note: Wild cards pass Color::WILD for color.
    SpecialCard(Color color, CardType type)
        : Card(color, type)
    {

        // Sanity check: NUMBER type doesn't belong in SpecialCard.
        if (type == CardType::NUMBER)
        {
            throw invalid_argument("SpecialCard cannot have type NUMBER");
        }
    }

    // Lets GameManager set the chosen color after a Wild is played.
    void setChosenColor(Color c) { chosenColor = c; }
    Color getChosenColor() const { return chosenColor; }

    // OVERRIDE: Human-readable label for each special card variant.
    string getLabel() const override
    {
        switch (type)
        {
        case CardType::SKIP:
            return colorToString(color) + " Skip";
        case CardType::REVERSE:
            return colorToString(color) + " Reverse";
        case CardType::DRAW_TWO:
            return colorToString(color) + " Draw Two";
        case CardType::WILD:
            return "Wild";
        case CardType::WILD_DRAW_FOUR:
            return "Wild Draw Four";
        default:
            return "Unknown Special";
        }
    }

    // OVERRIDE: Effect logic � the real game logic lives in GameManager,
    // so this is a stub for now. Each case will call into GameManager
    // (e.g. gm.skipNextPlayer(), gm.reverseDirection()).
    // The structure is already correct; we just flesh out the bodies later.
    void applyEffect() const override
    {
        switch (type)
        {
        case CardType::SKIP:
            cout << "  [Effect] Skip -> next player loses their turn.\n";
            break;
        case CardType::REVERSE:
            cout << "  [Effect] Reverse -> direction flipped.\n";
            break;
        case CardType::DRAW_TWO:
            cout << "  [Effect] Draw Two -> next player draws 2 and loses turn.\n";
            break;
        case CardType::WILD:
            cout << "  [Effect] Wild -> active player chooses a color.\n";
            break;
        case CardType::WILD_DRAW_FOUR:
            cout << "  [Effect] Wild Draw Four -> next player draws 4 and loses turn.\n";
            break;
        default:
            break;
        }
    }
};