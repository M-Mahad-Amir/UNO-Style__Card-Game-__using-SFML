// ============================================================
//  main.cpp  —  SFML UI Layer for Classic UNO (2-Player)
//
//  Screens / overlays managed here:
//    MENU        —  title, New Game, Rules, Quit
//    RULES       —  full rules text, Back button
//    PLAYING     —  game board (cards, piles, buttons)
//                   sub-states handled via TurnPhase from backend:
//                     CHOOSE_ACTION        normal turn
//                     AWAITING_DRAW_DECISION  drawn a card
//                     CHOOSE_COLOR         Wild overlay
//                     PLAY_WILD_CARD       filtered hand
//    UNO_POPUP   —  auto-shows 2 s when a player reaches 1 card
//    WIN         —  winner announcement
//
//  Build (from SFML-3.0.2 root):
//    g++ -Isrc/include -std=c++17 -c coding/UNO_Game_v5.cpp coding/main.cpp
//    g++ UNO_Game_v5.o main.o -o uno_game -Lsrc/lib
//        -lsfml-graphics -lsfml-window -lsfml-system -lopengl32
//
//  Font used: arialbd.ttf (Arial Bold) located next to the exe.
// ============================================================

#include "UNO_Game_v5.h"            // game backend (all logic)
#include <SFML/Graphics.hpp>         // SFML 3.0.2 graphics
#include <cmath>
#include <algorithm>
#include <string>

using namespace std;

// ============================================================
//  WINDOW & LAYOUT CONSTANTS
// ============================================================

static constexpr unsigned WIN_W = 1200;
static constexpr unsigned WIN_H = 750;

// Pile cards  (larger, center of screen)
static constexpr float PILE_W  = 100.f;
static constexpr float PILE_H  = 150.f;

// Hand cards  (player's own visible hand, bottom)
static constexpr float HAND_W  = 72.f;
static constexpr float HAND_H  = 108.f;

// Face-down cards shown for the opponent (top)
static constexpr float BACK_W  = 55.f;
static constexpr float BACK_H  = 82.f;

// Pile positions  (top-left corners of each pile rectangle)
static constexpr float DISCARD_X = 460.f;
static constexpr float DISCARD_Y = 265.f;

static constexpr float DRAW_X    = 620.f;
static constexpr float DRAW_Y    = 265.f;

// ============================================================
//  GAME STATE  (drives the main loop switch)
// ============================================================

enum class GameState { MENU, RULES, PLAYING, UNO_POPUP, WIN };

// ============================================================
//  COLOUR HELPERS
// ============================================================

// Returns the SFML fill colour that represents a card's game colour.
sf::Color sfCardColor(Color c)
{
    switch (c) {
        case Color::RED:    return { 210,  45,  45 };
        case Color::BLUE:   return {  45, 105, 210 };
        case Color::GREEN:  return {  40, 170,  70 };
        case Color::YELLOW: return { 240, 210,  25 };
        case Color::WILD:   return {  25,  25,  25 };   // near-black for Wild
        default:            return { 100, 100, 100 };
    }
}

// Darkens a colour by ~30 % (used for the inner oval on cards).
sf::Color darken(sf::Color c)
{
    return { static_cast<uint8_t>(c.r * 0.65f),
             static_cast<uint8_t>(c.g * 0.65f),
             static_cast<uint8_t>(c.b * 0.65f) };
}

// Lightens a colour slightly (button hover effect).
sf::Color lighten(sf::Color c, int amt = 45)
{
    auto clamp = [](int v) -> uint8_t { return static_cast<uint8_t>(min(255, v)); };
    return { clamp(c.r + amt), clamp(c.g + amt), clamp(c.b + amt), c.a };
}

// ============================================================
//  DRAW BACKGROUND  —  red-to-yellow diagonal gradient
// ============================================================

void drawBackground(sf::RenderTarget& target)
{
    // VertexArray of two triangles (= one gradient quad).
    sf::VertexArray bg(sf::PrimitiveType::TriangleStrip, 4);

    sf::Color topLeft     { 210,  40,  40 };   // deep red
    sf::Color topRight    { 245, 215,  35 };   // golden yellow
    sf::Color bottomLeft  { 215,  60,  15 };   // red-orange
    sf::Color bottomRight { 240, 235,  60 };   // bright yellow

    bg[0].position = { 0.f,          0.f           }; bg[0].color = topLeft;
    bg[1].position = { (float)WIN_W, 0.f           }; bg[1].color = topRight;
    bg[2].position = { 0.f,          (float)WIN_H  }; bg[2].color = bottomLeft;
    bg[3].position = { (float)WIN_W, (float)WIN_H  }; bg[3].color = bottomRight;

    target.draw(bg);
}

// ============================================================
//  DRAW TEXT HELPER  —  centered or left-aligned in one call
// ============================================================

// centerX/centerY:  the point the text is centered around.
// If 'centered' is false, (centerX, centerY) is the top-left corner.
void drawText(sf::RenderTarget& target, const sf::Font& font,
              const string& str, float cx, float cy,
              unsigned int size, sf::Color color = sf::Color::White,
              bool bold = false, bool centered = true)
{
    sf::Text txt(font, str, size);
    txt.setFillColor(color);
    if (bold) txt.setStyle(sf::Text::Bold);

    if (centered) {
        auto b = txt.getLocalBounds();
        txt.setOrigin({ b.position.x + b.size.x / 2.f,
                        b.position.y + b.size.y / 2.f });
    }
    txt.setPosition({ cx, cy });
    target.draw(txt);
}

// ============================================================
//  DRAW BUTTON  —  returns true if the button was clicked
// ============================================================
//
//  The button highlights when the mouse hovers over it and
//  pulses with an optional glow colour (for the "Draw Card" btn).

bool drawButton(sf::RenderTarget& target, const sf::Font& font,
                const string& label,
                float x, float y, float w, float h,
                sf::Vector2f mousePos, bool clicked,
                sf::Color baseColor  = { 50, 50, 50, 210 },
                sf::Color glowColor  = { 0,  0,  0,   0  },  // 0 alpha = no glow
                float     glowAmount = 0.f)                   // 0..1
{
    sf::FloatRect rect({ x, y }, { w, h });
    bool hovered   = rect.contains(mousePos);
    bool wasClicked = hovered && clicked;

    // Choose fill: glow blend when active, hover-lighten otherwise.
    sf::Color fill = baseColor;
    if (glowColor.a > 0 && glowAmount > 0.f) {
        // Blend baseColor toward glowColor by glowAmount.
        auto blend = [&](uint8_t a, uint8_t b, float t) -> uint8_t {
            return static_cast<uint8_t>(a + (b - a) * t);
        };
        fill = { blend(baseColor.r, glowColor.r, glowAmount),
                 blend(baseColor.g, glowColor.g, glowAmount),
                 blend(baseColor.b, glowColor.b, glowAmount),
                 baseColor.a };
    }
    if (hovered) fill = lighten(fill, 40);

    sf::RectangleShape btn({ w, h });
    btn.setPosition({ x, y });
    btn.setFillColor(fill);
    btn.setOutlineColor(hovered ? sf::Color::White : sf::Color{ 200, 200, 200, 180 });
    btn.setOutlineThickness(hovered ? 3.f : 1.5f);
    target.draw(btn);

    // Centered label text inside the button.
    sf::Text txt(font, label, 22);
    txt.setFillColor(sf::Color::White);
    txt.setStyle(sf::Text::Bold);
    auto b = txt.getLocalBounds();
    txt.setOrigin({ b.position.x + b.size.x / 2.f,
                    b.position.y + b.size.y / 2.f });
    txt.setPosition({ x + w / 2.f, y + h / 2.f });
    target.draw(txt);

    return wasClicked;
}

// ============================================================
//  DRAW ONE UNO CARD  (face-up)
// ============================================================
//
//  Renders a card at (x, y) with dimensions w×h.
//  'selected' = true raises the card slightly and adds a bright border.
//  'dimmed'   = true draws the card grey (non-playable in Wild phase).

void drawCard(sf::RenderTarget& target, const sf::Font& font,
              const Card* card,
              float x, float y, float w, float h,
              bool selected = false, bool dimmed = false)
{
    float yOff = selected ? -14.f : 0.f;   // raise selected card visually

    // --- Card body background ---
    sf::Color bodyColor = sfCardColor(card->getColor());
    // Dim non-playable cards by darkening their real colour (never grey).
    if (dimmed) bodyColor = darken(darken(bodyColor));

    sf::RectangleShape body({ w, h });
    body.setPosition({ x, y + yOff });
    body.setFillColor(bodyColor);
    body.setOutlineColor(selected ? sf::Color{ 255, 255, 0 }
                                  : sf::Color::White);
    body.setOutlineThickness(selected ? 3.5f : 1.5f);
    target.draw(body);

    // --- Special handling: Wild & Wild Draw Four ---
    if (card->getColor() == Color::WILD)
    {
        // Draw four coloured quadrants to mimic the real Wild card swirl.
        float hw = w / 2.f, hh = h / 2.f;
        struct { sf::Color col; float ox, oy; } quad[4] = {
            { sfCardColor(Color::RED),    0.f,  0.f  },
            { sfCardColor(Color::BLUE),   hw,   0.f  },
            { sfCardColor(Color::GREEN),  0.f,  hh   },
            { sfCardColor(Color::YELLOW), hw,   hh   }
        };
        for (auto& q : quad) {
            sf::RectangleShape half({ hw, hh });
            half.setPosition({ x + q.ox, y + yOff + q.oy });
            half.setFillColor(q.col);
            target.draw(half);
        }
        // Dark circle overlay in centre (UNO style).
        float cr = min(w, h) * 0.28f;
        sf::CircleShape centre(cr);
        centre.setOrigin({ cr, cr });
        centre.setFillColor({ 20, 20, 20 });
        centre.setPosition({ x + w / 2.f, y + yOff + h / 2.f });
        target.draw(centre);

        // Re-draw the card outline on top of all quadrants.
        sf::RectangleShape outline({ w, h });
        outline.setPosition({ x, y + yOff });
        outline.setFillColor(sf::Color::Transparent);
        outline.setOutlineColor(selected ? sf::Color{ 255, 255, 0 } : sf::Color::White);
        outline.setOutlineThickness(selected ? 3.5f : 1.5f);
        target.draw(outline);

        // Short abbreviation in the centre.
        string abbr = (card->getType() == CardType::WILD_DRAW_FOUR) ? "+4" : "W";
        sf::Text lbl(font, abbr, static_cast<unsigned>(h * 0.26f));
        lbl.setFillColor(sf::Color::White);
        lbl.setStyle(sf::Text::Bold);
        lbl.setOutlineColor(sf::Color::Black);
        lbl.setOutlineThickness(1.5f);
        auto bnd = lbl.getLocalBounds();
        lbl.setOrigin({ bnd.position.x + bnd.size.x / 2.f,
                        bnd.position.y + bnd.size.y / 2.f });
        lbl.setPosition({ x + w / 2.f, y + yOff + h / 2.f });
        target.draw(lbl);
        return;
    }

    // --- For coloured (non-Wild) cards: tilted oval + number/symbol ---

    // Outer white oval.
    float ovalR = min(w, h) * 0.38f;
    sf::CircleShape outerOval(ovalR);
    outerOval.setFillColor(sf::Color::White);
    outerOval.setOrigin({ ovalR, ovalR });
    outerOval.setScale({ 0.58f, 1.05f });
    outerOval.setRotation(sf::degrees(-22.f));
    outerOval.setPosition({ x + w / 2.f, y + yOff + h / 2.f });
    target.draw(outerOval);

    // Inner darker oval (slightly smaller, card colour).
    float innerR = ovalR * 0.80f;
    sf::CircleShape innerOval(innerR);
    innerOval.setFillColor(darken(bodyColor));   // bodyColor already darkened when dimmed
    innerOval.setOrigin({ innerR, innerR });
    innerOval.setScale({ 0.58f, 1.05f });
    innerOval.setRotation(sf::degrees(-22.f));
    innerOval.setPosition({ x + w / 2.f, y + yOff + h / 2.f });
    target.draw(innerOval);

    // Abbreviated centre label: number digit or symbol.
    string abbr;
    switch (card->getType()) {
        case CardType::NUMBER:   abbr = to_string(card->getValue()); break;
        case CardType::SKIP:     abbr = "S";   break;
        case CardType::REVERSE:  abbr = "R";   break;
        case CardType::DRAW_TWO: abbr = "+2";  break;
        default:                 abbr = "?";   break;
    }

    sf::Text lbl(font, abbr, static_cast<unsigned>(h * 0.28f));
    lbl.setFillColor(sf::Color::White);
    lbl.setStyle(sf::Text::Bold);
    lbl.setOutlineColor(sf::Color::Black);
    lbl.setOutlineThickness(1.5f);
    auto bnd = lbl.getLocalBounds();
    lbl.setOrigin({ bnd.position.x + bnd.size.x / 2.f,
                    bnd.position.y + bnd.size.y / 2.f });
    lbl.setPosition({ x + w / 2.f, y + yOff + h / 2.f });
    target.draw(lbl);

    // Small corner label (top-left).
    sf::Text corner(font, abbr, static_cast<unsigned>(h * 0.12f));
    corner.setFillColor(sf::Color::White);
    corner.setStyle(sf::Text::Bold);
    corner.setPosition({ x + 4.f, y + yOff + 3.f });
    target.draw(corner);
}

// ============================================================
//  DRAW FACE-DOWN CARD  (opponent's hand / draw pile)
// ============================================================

void drawFaceDown(sf::RenderTarget& target, const sf::Font& font,
                  float x, float y, float w, float h)
{
    // Dark navy background.
    sf::RectangleShape body({ w, h });
    body.setPosition({ x, y });
    body.setFillColor({ 18, 40, 120 });
    body.setOutlineColor({ 180, 180, 180 });
    body.setOutlineThickness(1.5f);
    target.draw(body);

    // White inner border stripe.
    float pad = w * 0.1f;
    sf::RectangleShape inner({ w - pad * 2.f, h - pad * 2.f });
    inner.setPosition({ x + pad, y + pad });
    inner.setFillColor(sf::Color::Transparent);
    inner.setOutlineColor({ 220, 220, 220, 200 });
    inner.setOutlineThickness(1.5f);
    target.draw(inner);

    // "UNO" text in red.
    sf::Text lbl(font, "UNO", static_cast<unsigned>(h * 0.17f));
    lbl.setFillColor({ 220, 50, 50 });
    lbl.setStyle(sf::Text::Bold);
    lbl.setOutlineColor(sf::Color::White);
    lbl.setOutlineThickness(1.f);
    auto b = lbl.getLocalBounds();
    lbl.setOrigin({ b.position.x + b.size.x / 2.f,
                    b.position.y + b.size.y / 2.f });
    lbl.setPosition({ x + w / 2.f, y + h / 2.f });
    target.draw(lbl);
}

// ============================================================
//  DRAW SEMI-TRANSPARENT DARK OVERLAY  (for popups)
// ============================================================

void drawDimOverlay(sf::RenderTarget& target)
{
    sf::RectangleShape overlay({ (float)WIN_W, (float)WIN_H });
    overlay.setFillColor({ 0, 0, 0, 160 });
    target.draw(overlay);
}

// ============================================================
//  SCREEN: MAIN MENU
// ============================================================
//
//  Returns the next GameState when the player presses a button.

GameState drawMenuScreen(sf::RenderTarget& target, const sf::Font& font,
                         sf::Vector2f mousePos, bool clicked,
                         GameManager* gm)
{
    drawBackground(target);

    // --- Title ---
    drawText(target, font, "Classic UNO Game!", WIN_W / 2.f, 110.f,
             62, sf::Color::White, true);

    // Shadow effect: draw same title in dark slightly offset.
    sf::Text shadow(font, "Classic UNO Game!", 62);
    shadow.setFillColor({ 0, 0, 0, 100 });
    shadow.setStyle(sf::Text::Bold);
    {
        auto b = shadow.getLocalBounds();
        shadow.setOrigin({ b.position.x + b.size.x / 2.f,
                           b.position.y + b.size.y / 2.f });
    }
    shadow.setPosition({ WIN_W / 2.f + 3.f, 113.f });
    target.draw(shadow);
    drawText(target, font, "Classic UNO Game!", WIN_W / 2.f, 110.f,
             62, sf::Color::White, true);

    // --- Subtitle ---
    drawText(target, font, "2-Player  C++ & SFML Edition",
             WIN_W / 2.f, 180.f, 26, { 255, 245, 180 }, false);

    // --- Decorative horizontal rule ---
    sf::RectangleShape rule({ 420.f, 2.f });
    rule.setFillColor({ 255, 255, 255, 120 });
    rule.setPosition({ WIN_W / 2.f - 210.f, 210.f });
    target.draw(rule);

    // --- Menu Buttons ---
    float btnW = 260.f, btnH = 58.f;
    float btnX = WIN_W / 2.f - btnW / 2.f;

    bool newGame = drawButton(target, font, "New Game",
                              btnX, 270.f, btnW, btnH, mousePos, clicked,
                              { 30, 100, 30, 220 });

    bool rules   = drawButton(target, font, "Rules",
                              btnX, 350.f, btnW, btnH, mousePos, clicked,
                              { 30,  60, 140, 220 });

    bool quit    = drawButton(target, font, "Quit",
                              btnX, 430.f, btnW, btnH, mousePos, clicked,
                              { 130, 30,  30, 220 });

    // --- Footer ---
    drawText(target, font, "Click a button to begin",
             WIN_W / 2.f, 530.f, 18, { 255, 255, 255, 150 }, false);

    if (newGame) {
        // Reset the game with fresh players and deal 7 cards each.
        gm->resetGame();
        return GameState::PLAYING;
    }
    if (rules)  return GameState::RULES;
    if (quit)   { exit(0); }

    return GameState::MENU;
}

// ============================================================
//  SCREEN: RULES
// ============================================================

GameState drawRulesScreen(sf::RenderTarget& target, const sf::Font& font,
                           sf::Vector2f mousePos, bool clicked)
{
    drawBackground(target);

    // --- Title ---
    drawText(target, font, "UNO - How to Play", WIN_W / 2.f, 55.f,
             40, sf::Color::White, true);

    // --- Rules panel background ---
    sf::RectangleShape panel({ 860.f, 530.f });
    panel.setPosition({ WIN_W / 2.f - 430.f, 95.f });
    panel.setFillColor({ 0, 0, 0, 140 });
    panel.setOutlineColor({ 255, 255, 255, 80 });
    panel.setOutlineThickness(1.5f);
    target.draw(panel);

    // --- Rules text (each line drawn separately for easy layout) ---
    const vector<pair<string, sf::Color>> lines = {
        { "OBJECTIVE",                                                 { 255, 220,  60 } },
        { "  Be the first player to empty your hand - you win!",      sf::Color::White },
        { "",                                                           sf::Color::White },
        { "SETUP",                                                     { 255, 220,  60 } },
        { "  Each player is dealt 7 cards.  A starting card is placed face-up.",
                                                                        sf::Color::White },
        { "",                                                           sf::Color::White },
        { "ON YOUR TURN",                                              { 255, 220,  60 } },
        { "  Play a card that matches the top card's COLOUR or NUMBER/TYPE.",
                                                                        sf::Color::White },
        { "  If you cannot play, press 'Draw Card' to take one from the deck.",
                                                                        sf::Color::White },
        { "  If the drawn card is playable, you may play it immediately.",
                                                                        sf::Color::White },
        { "",                                                           sf::Color::White },
        { "SPECIAL CARDS",                                             { 255, 220,  60 } },
        { "  Skip (S)     : Opponent loses their next turn.",          sf::Color::White },
        { "  Reverse (R)  : Same as Skip in 2-player mode.",          sf::Color::White },
        { "  Draw Two (+2): Opponent draws 2 cards (but still plays).", sf::Color::White },
        { "  Wild (W)     : Play on anything; choose a colour.",       sf::Color::White },
        { "                  Then play a card of that colour too!",     { 200,230,255 } },
        { "  Wild +4      : Opponent draws 4; then same as Wild.",     sf::Color::White },
        { "",                                                           sf::Color::White },
        { "UNO!",                                                      { 255, 220,  60 } },
        { "  When you are down to 1 card the game shouts 'UNO!' for you.",
                                                                        sf::Color::White },
    };

    float lineY = 108.f;
    for (auto& [text, color] : lines) {
        if (!text.empty())
            drawText(target, font, text, WIN_W / 2.f - 410.f, lineY,
                     17, color, false, /*centered=*/false);
        lineY += 22.f;
    }

    // --- Back button ---
    bool back = drawButton(target, font, "Back to Menu",
                           WIN_W / 2.f - 100.f, 650.f, 200.f, 50.f,
                           mousePos, clicked, { 60, 60, 60, 220 });

    if (back) return GameState::MENU;
    return GameState::RULES;
}

// ============================================================
//  SCREEN: WIN
// ============================================================

GameState drawWinScreen(sf::RenderTarget& target, const sf::Font& font,
                         sf::Vector2f mousePos, bool clicked,
                         GameManager* gm)
{
    drawBackground(target);

    // Announce the winner.
    string winnerName = gm->getWinner() ? gm->getWinner()->getName() : "Nobody";

    drawText(target, font, winnerName + "  Wins!", WIN_W / 2.f, 230.f,
             70, sf::Color::White, true);

    drawText(target, font, "Congratulations!",
             WIN_W / 2.f, 320.f, 32, { 255, 240, 100 }, false);

    // Decorative star row.
    drawText(target, font, "*   *   *   *   *",
             WIN_W / 2.f, 380.f, 30, { 255, 215, 0 }, false);

    float btnW = 240.f, btnH = 56.f;
    float btnX = WIN_W / 2.f - btnW / 2.f;

    bool newGame = drawButton(target, font, "Play Again",
                              btnX, 440.f, btnW, btnH, mousePos, clicked,
                              { 30, 100, 30, 220 });

    bool quit    = drawButton(target, font, "Quit to Menu",
                              btnX, 515.f, btnW, btnH, mousePos, clicked,
                              { 90,  40, 120, 220 });

    if (newGame) { gm->resetGame(); return GameState::PLAYING; }
    if (quit)    return GameState::MENU;
    return GameState::WIN;
}

// ============================================================
//  OVERLAY: COLOUR PICKER  (shown when TurnPhase == CHOOSE_COLOR)
// ============================================================
//
//  Drawn on top of the game board.
//  Calls gm->setChosenColor() and returns true when done.

bool drawColorPickOverlay(sf::RenderTarget& target, const sf::Font& font,
                           sf::Vector2f mousePos, bool clicked,
                           GameManager* gm)
{
    drawDimOverlay(target);

    // Panel.
    float px = WIN_W / 2.f - 230.f, py = 250.f;
    sf::RectangleShape panel({ 460.f, 240.f });
    panel.setPosition({ px, py });
    panel.setFillColor({ 20, 20, 20, 230 });
    panel.setOutlineColor(sf::Color::White);
    panel.setOutlineThickness(2.f);
    target.draw(panel);

    drawText(target, font, "Choose a Color for your Wild",
             WIN_W / 2.f, py + 30.f, 24, sf::Color::White, true);

    // Four colour buttons in a row.
    const struct { Color c; string name; } colours[4] = {
        { Color::RED,    "Red"    },
        { Color::BLUE,   "Blue"   },
        { Color::GREEN,  "Green"  },
        { Color::YELLOW, "Yellow" }
    };

    float btnW = 90.f, btnH = 90.f;
    float startX = WIN_W / 2.f - (btnW * 2.f + 10.f * 1.5f);

    for (int i = 0; i < 4; i++) {
        float bx = startX + i * (btnW + 10.f);
        float by = py + 80.f;

        sf::Color base = sfCardColor(colours[i].c);
        bool pressed = drawButton(target, font, colours[i].name,
                                  bx, by, btnW, btnH,
                                  mousePos, clicked, base);

        if (pressed) {
            gm->setChosenColor(colours[i].c);
            return true;   // colour was chosen, overlay done
        }
    }
    return false;
}

// ============================================================
//  OVERLAY: UNO POPUP  (shown for ~2 s when a player has 1 card)
// ============================================================
//
//  Returns true once the timer expires (caller dismisses it).

bool drawUnoPopup(sf::RenderTarget& target, const sf::Font& font,
                   float elapsedSeconds, const string& playerName)
{
    drawDimOverlay(target);

    // Bounce scale: starts large, settles to normal.
    float scale = 1.f + max(0.f, 0.4f - elapsedSeconds * 0.4f);

    // "UNO!" in big bold yellow.
    sf::Text unoTxt(font, "U N O !", static_cast<unsigned>(90 * scale));
    unoTxt.setFillColor({ 255, 220, 30 });
    unoTxt.setStyle(sf::Text::Bold);
    unoTxt.setOutlineColor({ 180, 30, 30 });
    unoTxt.setOutlineThickness(4.f);
    auto b = unoTxt.getLocalBounds();
    unoTxt.setOrigin({ b.position.x + b.size.x / 2.f,
                       b.position.y + b.size.y / 2.f });
    unoTxt.setPosition({ WIN_W / 2.f, 310.f });
    target.draw(unoTxt);

    drawText(target, font, playerName + " has ONE card left!",
             WIN_W / 2.f, 420.f, 28, sf::Color::White, true);

    drawText(target, font, "(continuing in a moment...)",
             WIN_W / 2.f, 465.f, 18, { 200, 200, 200 }, false);

    return elapsedSeconds >= 2.0f;   // dismiss after 2 seconds
}

// ============================================================
//  SCREEN: GAME BOARD  (main gameplay screen)
// ============================================================
//
//  Renders everything for the PLAYING state:
//    • Gradient background
//    • Player 2's hidden hand (top)
//    • Discard and Draw piles (centre)
//    • Player turn indicator and game-state message
//    • Player 1's visible hand (bottom) — clickable
//    • Context-sensitive buttons (Draw, Play Drawn, Keep)
//    • Wild colour-picker overlay  (CHOOSE_COLOR phase)
//    • Wild follow-up highlight    (PLAY_WILD_CARD phase)
//
//  Returns true if the UNO popup should be triggered this frame
//  (i.e. current player just reached 1 card).

bool drawGameScreen(sf::RenderTarget& target, const sf::Font& font,
                    GameManager* gm,
                    sf::Vector2f mousePos, bool clicked,
                    float gameClock,       // seconds since game started
                    string& statusMsg,     // in/out: updated each frame
                    bool& prevHadOne)      // tracks "just reached 1 card"
{
    drawBackground(target);

    Player* cur   = gm->getCurrentPlayer();
    Player* other = gm->getOtherPlayer();
    TurnPhase phase = gm->getTurnPhase();

    bool triggerUno = false;   // set true if UNO popup should show

    // --------------------------------------------------------
    //  TOP SECTION: Opponent (other player) face-down cards
    // --------------------------------------------------------

    // Opponent info bar.
    sf::RectangleShape topBar({ (float)WIN_W, 40.f });
    topBar.setFillColor({ 0, 0, 0, 80 });
    topBar.setPosition({ 0.f, 0.f });
    target.draw(topBar);

    drawText(target, font,
             other->getName() + "   |   " +
             to_string(other->getHandSize()) + " card" +
             (other->getHandSize() != 1 ? "s" : ""),
             WIN_W / 2.f, 20.f, 22, { 230, 230, 230 }, false);

    // Opponent's face-down cards, centered, overlapping.
    int  opCount   = other->getHandSize();
    float overlapX = min(BACK_W + 6.f, 900.f / max(1, opCount));
    float totalW   = (opCount - 1) * overlapX + BACK_W;
    float startX   = WIN_W / 2.f - totalW / 2.f;

    for (int i = 0; i < opCount; i++)
        drawFaceDown(target, font, startX + i * overlapX, 48.f, BACK_W, BACK_H);

    // --------------------------------------------------------
    //  LEFT PANEL: Current player label + game state message
    // --------------------------------------------------------

    // Semi-transparent left panel.
    sf::RectangleShape leftPanel({ 190.f, 340.f });
    leftPanel.setPosition({ 10.f, 200.f });
    leftPanel.setFillColor({ 0, 0, 0, 90 });
    leftPanel.setOutlineColor({ 255, 255, 255, 50 });
    leftPanel.setOutlineThickness(1.f);
    target.draw(leftPanel);

    // "CURRENT TURN" label.
    drawText(target, font, "CURRENT TURN", 105.f, 220.f,
             15, { 200, 200, 200 }, false);

    // Player name in bright colour.
    sf::Color nameColor = (gm->getCurrentPlayerIdx() == 0)
                          ? sf::Color{ 255, 240, 100 }   // P1 = yellow
                          : sf::Color{ 120, 220, 255 };  // P2 = cyan

    drawText(target, font, cur->getName(), 105.f, 250.f,
             24, nameColor, true);

    // Phase hint.
    string phaseHint;
    switch (phase) {
        case TurnPhase::CHOOSE_ACTION:
            phaseHint = "Pick a card\nor Draw";        break;
        case TurnPhase::AWAITING_DRAW_DECISION:
            phaseHint = "Play or\nKeep drawn\ncard?";  break;
        case TurnPhase::CHOOSE_COLOR:
            phaseHint = "Choose\na colour!";           break;
        case TurnPhase::PLAY_WILD_CARD:
            phaseHint = "Play a\n" + colorToString(gm->getPendingWildColor())
                        + "\ncard!";                    break;
        default:
            phaseHint = "";                             break;
    }
    // Draw multi-line hint (split on '\n').
    float hintY = 290.f;
    string tmp;
    for (char ch : phaseHint) {
        if (ch == '\n') {
            drawText(target, font, tmp, 105.f, hintY, 16,
                     sf::Color::White, true);
            hintY += 22.f;
            tmp.clear();
        } else tmp += ch;
    }
    if (!tmp.empty())
        drawText(target, font, tmp, 105.f, hintY, 16, sf::Color::White, true);

    // --------------------------------------------------------
    //  CENTRE: Discard pile label + card
    // --------------------------------------------------------

    drawText(target, font, "Discard Pile",
             DISCARD_X + PILE_W / 2.f, DISCARD_Y - 22.f, 16,
             { 230, 230, 230 }, true);

    drawCard(target, font, gm->getTopCard(),
             DISCARD_X, DISCARD_Y, PILE_W, PILE_H);

    // --------------------------------------------------------
    //  CENTRE: Draw pile label + face-down card
    // --------------------------------------------------------

    drawText(target, font, "Draw Pile",
             DRAW_X + PILE_W / 2.f, DRAW_Y - 22.f, 16,
             { 230, 230, 230 }, true);

    drawFaceDown(target, font, DRAW_X, DRAW_Y, PILE_W, PILE_H);

    // --------------------------------------------------------
    //  DRAW CARD button  (only active in CHOOSE_ACTION phase)
    // --------------------------------------------------------

    bool canDraw = (phase == TurnPhase::CHOOSE_ACTION);

    // Glow pulse using a sine wave driven by gameClock.
    float pulse  = canDraw ? (0.4f + 0.6f * abs(sin(gameClock * 2.5f))) : 0.f;
    sf::Color drawBase  = canDraw ? sf::Color{ 50,  50,  50, 220 }
                                  : sf::Color{ 50,  50,  50,  90 };
    sf::Color drawGlow  = { 240, 190,  20, 255 };   // golden yellow glow

    bool didDraw = drawButton(target, font, "Draw Card",
                              DRAW_X, DRAW_Y + PILE_H + 14.f,
                              PILE_W, 42.f,
                              mousePos, clicked,
                              drawBase, drawGlow, pulse);

    if (didDraw && canDraw) {
        gm->drawCard();
        statusMsg = "";   // reset message after action
    }

    // --------------------------------------------------------
    //  AWAITING_DRAW_DECISION: "Play it" / "Keep it" buttons
    // --------------------------------------------------------

    if (phase == TurnPhase::AWAITING_DRAW_DECISION) {
        Card* drawn = gm->getLastDrawnCard();
        string drawnLabel = drawn ? drawn->getLabel() : "?";

        drawText(target, font, "Drew:  " + drawnLabel,
                 DRAW_X + PILE_W / 2.f, DRAW_Y + PILE_H + 70.f,
                 17, { 255, 240, 180 }, true);

        bool play = drawButton(target, font, "Play It",
                               DRAW_X - 10.f, DRAW_Y + PILE_H + 92.f,
                               60.f, 40.f,
                               mousePos, clicked, { 30, 110, 30, 220 });

        bool keep = drawButton(target, font, "Keep It",
                               DRAW_X + 54.f, DRAW_Y + PILE_H + 92.f,
                               56.f, 40.f,
                               mousePos, clicked, { 100, 40, 40, 220 });

        if (play) { gm->playDrawnCard(); statusMsg = ""; }
        if (keep) { gm->keepDrawnCard(); statusMsg = ""; }
    }

    // --------------------------------------------------------
    //  PLAY_WILD_CARD hint  (chosen colour indicator)
    // --------------------------------------------------------

    if (phase == TurnPhase::PLAY_WILD_CARD) {
        sf::Color chosenSf = sfCardColor(gm->getPendingWildColor());
        string chosenStr   = colorToString(gm->getPendingWildColor());

        // Coloured banner in left panel area.
        sf::RectangleShape banner({ 190.f, 36.f });
        banner.setPosition({ 10.f, 450.f });
        banner.setFillColor(chosenSf);
        banner.setOutlineColor(sf::Color::White);
        banner.setOutlineThickness(1.5f);
        target.draw(banner);
        drawText(target, font, "Play a  " + chosenStr + "  card",
                 105.f, 468.f, 15, sf::Color::White, true);

        // "Skip (no match)" button if current player has no matching cards.
        if (!gm->hasCardsOfColor(gm->getPendingWildColor())) {
            bool skip = drawButton(target, font, "No match - Skip",
                                   10.f, 495.f, 190.f, 40.f,
                                   mousePos, clicked, { 80, 40, 40, 220 });
            if (skip) gm->skipWildFollowUp();
        }
    }

    // --------------------------------------------------------
    //  STATUS MESSAGE BAR  (bottom-left, above player's hand)
    // --------------------------------------------------------

    sf::RectangleShape statusBar({ (float)WIN_W, 32.f });
    statusBar.setPosition({ 0.f, 550.f });
    statusBar.setFillColor({ 0, 0, 0, 100 });
    target.draw(statusBar);

    if (!statusMsg.empty())
        drawText(target, font, statusMsg, 20.f, 566.f, 16,
                 { 255, 220, 100 }, false, /*centered=*/false);
    else {
        // Show a friendly default hint.
        string hint;
        if      (phase == TurnPhase::CHOOSE_ACTION)
            hint = "Your turn - click a card to play it, or press Draw Card.";
        else if (phase == TurnPhase::PLAY_WILD_CARD)
            hint = "Wild played - now pick a " +
                   colorToString(gm->getPendingWildColor()) + " card from your hand.";
        drawText(target, font, hint, 20.f, 566.f, 16,
                 { 200, 200, 200 }, false, /*centered=*/false);
    }

    // --------------------------------------------------------
    //  BOTTOM: Current player's face-up hand (clickable)
    // --------------------------------------------------------

    // Player info bar at the very bottom.
    sf::RectangleShape botBar({ (float)WIN_W, 40.f });
    botBar.setFillColor({ 0, 0, 0, 80 });
    botBar.setPosition({ 0.f, (float)WIN_H - 40.f });
    target.draw(botBar);

    drawText(target, font,
             cur->getName() + "   |   " +
             to_string(cur->getHandSize()) + " card" +
             (cur->getHandSize() != 1 ? "s" : ""),
             WIN_W / 2.f, (float)WIN_H - 20.f, 22, nameColor, true);

    // Layout hand cards evenly.
    int   handSize   = cur->getHandSize();
    float cardW      = min(HAND_W, (WIN_W - 60.f) / max(1, handSize));
    float cardH      = cardW * 1.5f;
    float gapX       = (handSize > 1)
                       ? min(6.f, (WIN_W - 60.f - handSize * cardW) / (handSize - 1))
                       : 0.f;
    float totalCards = handSize * cardW + (handSize - 1) * gapX;
    float hx         = WIN_W / 2.f - totalCards / 2.f;
    float hy         = 590.f;

    const auto& hand = cur->getHand();
    for (int i = 0; i < handSize; i++) {
        float cx = hx + i * (cardW + gapX);

        // Determine if this card is valid to play right now.
        bool playable = false;
        bool dimmed   = false;

        if (phase == TurnPhase::CHOOSE_ACTION)
            playable = gm->isPlayable(hand[i]);
        else if (phase == TurnPhase::PLAY_WILD_CARD)
            playable = (hand[i]->getColor() == gm->getPendingWildColor());

        // Dim cards that cannot be played in the current phase.
        if ((phase == TurnPhase::CHOOSE_ACTION || phase == TurnPhase::PLAY_WILD_CARD)
            && !playable)
            dimmed = true;

        // Highlight the last-drawn card during AWAITING_DRAW_DECISION.
        bool isDrawn = (phase == TurnPhase::AWAITING_DRAW_DECISION
                        && hand[i] == gm->getLastDrawnCard());

        drawCard(target, font, hand[i], cx, hy, cardW, cardH,
                 /*selected=*/isDrawn, /*dimmed=*/dimmed);

        // --- Handle click on this card ---
        sf::FloatRect cardRect({ cx, hy - 14.f }, { cardW, cardH + 14.f });
        if (clicked && cardRect.contains(mousePos)) {
            bool ok = false;
            if      (phase == TurnPhase::CHOOSE_ACTION)
                ok = gm->tryPlayCard(i);
            else if (phase == TurnPhase::PLAY_WILD_CARD)
                ok = gm->tryPlayWildFollowUp(i);

            if (!ok && phase == TurnPhase::CHOOSE_ACTION)
                statusMsg = "That card cannot be played right now - try another!";
            else
                statusMsg = "";
        }
    }

    // --------------------------------------------------------
    //  CHOOSE_COLOR overlay  (drawn on top of everything else)
    // --------------------------------------------------------

    if (phase == TurnPhase::CHOOSE_COLOR)
        drawColorPickOverlay(target, font, mousePos, clicked, gm);

    // --------------------------------------------------------
    //  UNO detection: trigger popup when hand reaches 1 card
    // --------------------------------------------------------

    bool hasOneNow = (cur->getHandSize() == 1);
    if (hasOneNow && !prevHadOne)
        triggerUno = true;
    prevHadOne = hasOneNow;

    return triggerUno;
}

// ============================================================
//  MAIN  —  Window, game loop, state machine
// ============================================================

int main()
{
    // --- Create window ---
    sf::RenderWindow window(
        sf::VideoMode({ WIN_W, WIN_H }),
        "Classic UNO - 2 Player Edition",
        sf::Style::Close   // fixed size, no resize
    );
    window.setFramerateLimit(60);

    // --- Load font (Arial Bold, located next to the executable) ---
    sf::Font font;
    if (!font.openFromFile("arialbd.ttf")) {
        // Fallback: try plain Arial.
        if (!font.openFromFile("arial.ttf")) {
            // Last resort: system font path on Windows.
            [[maybe_unused]] bool _ = font.openFromFile("C:/Windows/Fonts/arial.ttf");
        }
    }

    // --- Initialise GameManager singleton ---
    GameManager* gm = GameManager::getInstance();

    // Start with two players but DON'T deal yet
    // (startGame() is called when the player presses "New Game").
    gm->addPlayer(new Player("Player 1"));
    gm->addPlayer(new Player("Player 2"));

    // --- Application state ---
    GameState state = GameState::MENU;

    // UNO popup state.
    bool        unoShowing   = false;
    float       unoStartTime = 0.f;
    string      unoPlayer;

    // "had 1 card last frame" tracker for UNO detection.
    bool        prevHadOne   = false;

    // Status message (shown above player's hand, clears after next action).
    string      statusMsg;

    // Master clock: drives glow pulses and UNO timer.
    sf::Clock   clock;

    // ============================================================
    //  MAIN LOOP
    // ============================================================
    while (window.isOpen())
    {
        float now     = clock.getElapsedTime().asSeconds();
        bool  clicked = false;

        // --- Event handling ---
        while (const auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* btn = event->getIf<sf::Event::MouseButtonReleased>())
                if (btn->button == sf::Mouse::Button::Left)
                    clicked = true;
        }

        // Mouse position in world coordinates.
        sf::Vector2f mousePos = window.mapPixelToCoords(
            sf::Mouse::getPosition(window));

        // --- Clear frame ---
        window.clear();

        // --- State machine ---
        switch (state)
        {
        // ---- MENU ----
        case GameState::MENU:
            prevHadOne = false;
            statusMsg  = "";
            state = drawMenuScreen(window, font, mousePos, clicked, gm);
            break;

        // ---- RULES ----
        case GameState::RULES:
            state = drawRulesScreen(window, font, mousePos, clicked);
            break;

        // ---- PLAYING ----
        case GameState::PLAYING:
        {
            // Draw the game board; receive UNO trigger signal.
            bool triggerUno = drawGameScreen(window, font, gm,
                                             mousePos, clicked,
                                             now, statusMsg, prevHadOne);

            // Transition to WIN if the game just ended.
            if (gm->isGameOver()) {
                state = GameState::WIN;
                break;
            }

            // Show UNO popup if a player just dropped to 1 card.
            if (triggerUno && !unoShowing) {
                unoShowing   = true;
                unoStartTime = now;
                unoPlayer    = gm->getCurrentPlayer()->getName();
                state        = GameState::UNO_POPUP;
            }
            break;
        }

        // ---- UNO POPUP ----
        case GameState::UNO_POPUP:
        {
            // Draw the game board underneath the popup.
            drawGameScreen(window, font, gm,
                           mousePos, /*clicked=*/false,
                           now, statusMsg, prevHadOne);

            float elapsed = now - unoStartTime;
            bool done     = drawUnoPopup(window, font, elapsed, unoPlayer);

            if (done) {
                unoShowing = false;
                state      = GameState::PLAYING;
            }
            break;
        }

        // ---- WIN ----
        case GameState::WIN:
            state = drawWinScreen(window, font, mousePos, clicked, gm);
            break;
        }

        window.display();
    }

    // --- Cleanup ---
    delete gm;
    return 0;
}
