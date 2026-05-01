#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <time.h>
#include <stdlib.h> // For std::invalid_argument
using namespace std;

enum class Color;
enum class CardType;
string colorToString(Color c);
string cardTypeToString(CardType t);

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
string colorToString(Color c) {}

// Helper: converts a CardType to a printable string.
string cardTypeToString(CardType t) {}