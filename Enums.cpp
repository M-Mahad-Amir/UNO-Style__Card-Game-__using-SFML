#include "Enums.h"

enum class Color
{
    RED,
    BLUE,
    GREEN,
    YELLOW,
    WILD // Colorless cards (Wild, Wild Draw Four)
};

enum class CardType
{
    NUMBER,
    SKIP,
    REVERSE,
    DRAW_TWO,
    WILD,
    WILD_DRAW_FOUR
};

// Helper: converts a Color value to a printable string.
// We'll use this constantly for debugging and console output.
string colorToString(Color c)
{
    switch (c)
    {
    case Color::RED:
        return "Red";
    case Color::BLUE:
        return "Blue";
    case Color::GREEN:
        return "Green";
    case Color::YELLOW:
        return "Yellow";
    case Color::WILD:
        return "Wild";
    default:
        return "Unknown";
    }
}

// Helper: converts a CardType to a printable string.
string cardTypeToString(CardType t)
{
    switch (t)
    {
    case CardType::NUMBER:
        return "Number";
    case CardType::SKIP:
        return "Skip";
    case CardType::REVERSE:
        return "Reverse";
    case CardType::DRAW_TWO:
        return "Draw Two";
    case CardType::WILD:
        return "Wild";
    case CardType::WILD_DRAW_FOUR:
        return "Wild Draw Four";
    default:
        return "Unknown";
    }
}