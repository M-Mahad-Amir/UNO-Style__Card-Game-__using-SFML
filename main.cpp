#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "UNO_Game.h"
#include <string>
#include <algorithm>

// ============================================================
//  CONSTANTS
// ============================================================
const unsigned WIN_W = 1366;
const unsigned WIN_H = 820;

// ============================================================
//  SCREEN STATE
// ============================================================
enum class Screen
{
    MENU,
    RULES,
    GAME
};

// ============================================================
//  CARD COLOUR -> sf::Color
// ============================================================
sf::Color toSFColor(Color c)
{
    switch (c)
    {
    case Color::RED:
        return sf::Color(210, 45, 45);
    case Color::BLUE:
        return sf::Color(40, 100, 210);
    case Color::GREEN:
        return sf::Color(40, 170, 70);
    case Color::YELLOW:
        return sf::Color(220, 190, 20);
    default:
        return sf::Color(55, 55, 55); // WILD = dark grey
    }
}

// ============================================================
//  DRAW ONE UNO CARD  (coloured rect + centred label)
// ============================================================
void drawUnoCard(sf::RenderWindow &win, sf::Font &font,
                 const Card *card,
                 float x, float y,
                 float w = 108.f, float h = 145.f,
                 bool highlight = false)
{
    sf::RectangleShape rect({w, h});
    rect.setPosition({x, y});
    rect.setFillColor(toSFColor(card->getColor()));
    rect.setOutlineColor(highlight ? sf::Color::Yellow : sf::Color::White);
    rect.setOutlineThickness(highlight ? 3.5f : 1.5f);
    win.draw(rect);

    sf::Text lbl(font, card->getLabel(), 18);
    lbl.setFillColor(sf::Color::White);
    sf::FloatRect lb = lbl.getLocalBounds();
    lbl.setOrigin({lb.size.x / 2.f, lb.size.y / 2.f});
    lbl.setPosition({x + w / 2.f, y + h / 2.f});
    win.draw(lbl);
}

// ============================================================
//  DRAW A BUTTON  (filled rect + centred text)
//  Returns the FloatRect used, so the event handler can reuse
//  the same coordinates without duplicating numbers.
// ============================================================
sf::FloatRect drawButton(sf::RenderWindow &win, sf::Font &font,
                         const std::string &text,
                         float x, float y, float w, float h,
                         sf::Color fill, unsigned charSize = 20)
{
    sf::RectangleShape box({w, h});
    box.setPosition({x, y});
    box.setFillColor(fill);
    box.setOutlineColor(sf::Color::White);
    box.setOutlineThickness(2.f);
    win.draw(box);

    sf::Text lbl(font, text, charSize);
    lbl.setFillColor(sf::Color::White);
    sf::FloatRect lb = lbl.getLocalBounds();
    lbl.setOrigin({lb.size.x / 2.f, lb.size.y / 2.f});
    lbl.setPosition({x + w / 2.f, y + h / 2.f});
    win.draw(lbl);

    return sf::FloatRect({x, y}, {w, h});
}

// ============================================================
//  MAIN
// ============================================================
Screen screen = Screen::MENU;

// for UnPlayable Draw Card Indicator
bool showMsg = false;
sf::Clock msgClock;

// UNO popup: show when any player has exactly 1 card
bool showUno = false;
sf::Clock unoClock;
int lastUnoShownFor = -1; // player index for which UNO was last shown

int main()
{
    // ---- Window -----------------------------------------------
    sf::RenderWindow window(
        sf::VideoMode({WIN_W, WIN_H}),
        "Card Game Arena",
        sf::Style::Default);
    window.setFramerateLimit(60);

    // ---- Font --------------------------------------------------
    sf::Font font;
    if (!font.openFromFile("C:\\SFML\\SFML-3.0.2-windows-gcc-14.2.0-mingw-32-bit\\SFML-3.0.2\\coding\\arial.ttf"))
        return -1;

    // ---- Background Music --------------------------------------
    sf::Music bgMusic;
    if (!bgMusic.openFromFile("C:\\SFML\\SFML-3.0.2-windows-gcc-14.2.0-mingw-32-bit\\SFML-3.0.2\\coding\\Let'sPlay.mp3"))
        return -1;
    bgMusic.setLooping(true);
    bgMusic.setVolume(100.f);
    bgMusic.play();

    // ---- Game manager (singleton) ------------------------------
    GameManager *gm = GameManager::getInstance();
    gm->addPlayer(new Player("David"));
    gm->addPlayer(new Player("Sarah"));
    gm->startGame();

    Screen screen = Screen::MENU;

    // ============================================================
    //  MAIN LOOP
    // ============================================================
    while (window.isOpen())
    {
        // ========================================================
        //  EVENTS
        // ========================================================
        while (auto evOpt = window.pollEvent())
        {
            if (evOpt->is<sf::Event::Closed>())
            {
                window.close();
                break;
            }

            auto *mp = evOpt->getIf<sf::Event::MouseButtonPressed>();
            if (!mp || mp->button != sf::Mouse::Button::Left)
                continue;

            sf::Vector2f mouse = window.mapPixelToCoords(
                {mp->position.x, mp->position.y});

            // ---- MENU ------------------------------------------
            if (screen == Screen::MENU)
            {
                float bx = (WIN_W - 340.f) / 2.f;
                if (sf::FloatRect({bx, 290}, {340, 56}).contains(mouse))
                {
                    gm->resetGame();
                    screen = Screen::GAME;
                }
                else if (sf::FloatRect({bx, 368}, {340, 56}).contains(mouse))
                    screen = Screen::RULES;
                else if (sf::FloatRect({bx, 446}, {340, 56}).contains(mouse))
                    window.close();
            }

            // ---- RULES -----------------------------------------
            else if (screen == Screen::RULES)
            {
                if (sf::FloatRect({583, 754}, {200, 46}).contains(mouse))
                    screen = Screen::MENU;
            }

            // ---- GAME ------------------------------------------
            else if (screen == Screen::GAME)
            {
                TurnPhase phase = gm->getTurnPhase();

                // Win screen: only "New Game" is clickable
                if (phase == TurnPhase::GAME_OVER)
                {
                    if (sf::FloatRect({370, 390}, {280, 52}).contains(mouse))
                        gm->resetGame();
                    continue;
                }

                Player *cur = gm->getCurrentPlayer();
                int handSz = cur->getHandSize();

                // DRAW CARD button
    
                if (phase == TurnPhase::CHOOSE_ACTION &&
                    sf::FloatRect({1080, 290}, {200, 140}).contains(mouse))
                {
                    gm->drawCard();
                    if (gm->get_lastDrawnCard() == nullptr)
                    {
                        showMsg = true;
                        msgClock.restart();
                    }
                    continue;
                }

                // Play It / Keep It after drawing
            
                if (phase == TurnPhase::AWAITING_DRAW_DECISION)
                {
                    if (sf::FloatRect({360, 510}, {180, 48}).contains(mouse))
                        gm->playDrawnCard();
                    else if (sf::FloatRect({570, 510}, {180, 48}).contains(mouse))
                        gm->keepDrawnCard();
                    continue;
                }

                // Colour picker after Wild
                if (phase == TurnPhase::CHOOSE_COLOR)
                {
                    Color cols[4] = {Color::RED, Color::BLUE,
                                     Color::GREEN, Color::YELLOW};
            
                    float cx = 310.f, cy = 280.f,
                          bw = 110.f, bh = 110.f, bgap = 15.f;
                    for (int i = 0; i < 4; i++)
                    {
                        if (sf::FloatRect({cx + i * (bw + bgap), cy}, {bw, bh})
                                .contains(mouse))
                        {
                            gm->setChosenColor(cols[i]);
                            if (!gm->hasCardsOfColor(cols[i])) // No matching colour cards
                                gm->advanceTurn();             // Skip the "follow-up" phase and move to next player
                            break;
                        }
                    }
                    continue;
                }

                // Hand card clicks
                if (phase == TurnPhase::CHOOSE_ACTION ||
                    phase == TurnPhase::Play_Choosen_Card)
                {
                    // Replicate the same layout logic used in the draw section
                
                    float cw = 118.f, gap = 8.f;
                    float totalW = handSz * (cw + gap) - gap;
                    float availW = float(WIN_W) - 100.f;

                    if (totalW > availW)
                    {
                        cw = (availW - gap * (handSz - 1)) / handSz;
                        gap = (handSz > 1)
                                  ? (availW - cw * handSz) / (handSz - 1)
                                  : 0.f;
                        totalW = handSz * (cw + gap) - gap;
                    }

                    float startX = std::max(50.f,
                                            (float(WIN_W) - totalW) / 2.f);
                
                    float cardY = float(WIN_H) - 195.f;

                    for (int i = 0; i < handSz; i++)
                    {
                        
                        if (sf::FloatRect({startX + i * (cw + gap), cardY}, {cw, 160.f}).contains(mouse))
                        {
                            if (phase == TurnPhase::CHOOSE_ACTION)
                                gm->tryPlayCard(i);
                            else
                                gm->tryPlayWildFollowUp(i);
                            break;
                        }
                    }
                }
            }
        } // end event loop

        // ========================================================
        //  DRAW
        // ========================================================
        window.clear(sf::Color(54, 69, 79)); // slate grey background

        // ---- MENU SCREEN ----------------------------------------
        if (screen == Screen::MENU)
        {
            // Panel 
            sf::RectangleShape panel({600.f, 380.f});
            panel.setPosition({(WIN_W - 600.f) / 2.f, 150.f});
            panel.setFillColor(sf::Color(22, 52, 22));
            // panel.setOutlineColor(sf::Color(110, 110, 210));
            panel.setOutlineColor(sf::Color(110, 110, 210));
            panel.setOutlineThickness(3.f);
            window.draw(panel);

            // Heading
            sf::Text heading(font, "CARD MANIA", 48);
            heading.setFillColor(sf::Color::Red);
            heading.setStyle(sf::Text::Bold);
            sf::FloatRect hb = heading.getLocalBounds();
            heading.setOrigin({hb.size.x / 2.f, 0.f});
            heading.setPosition({float(WIN_W) / 2.f, 178.f});
            window.draw(heading);

            sf::Text tag(font, "Ready to Dive in?", 24);
            tag.setFillColor(sf::Color(160, 160, 220));
            sf::FloatRect tb = tag.getLocalBounds();
            tag.setOrigin({tb.size.x / 2.f, 0.f});
            tag.setPosition({float(WIN_W) / 2.f, 228.f});
            window.draw(tag);

            // Three buttons — x/y/w/h MUST match the hit-rects above   
            float bx = (WIN_W - 340.f) / 2.f;   // button x, always centered
            drawButton(window, font, "Start Game", bx, 290, 340, 56, sf::Color(60, 60, 140), 22);
            drawButton(window, font, "Rules",      bx, 368, 340, 56, sf::Color(40, 100, 100), 22);
            drawButton(window, font, "Quit Game",  bx, 446, 340, 56, sf::Color(130, 40, 40), 22);
        }

        // ---- RULES SCREEN ---------------------------------------
        else if (screen == Screen::RULES)
        {
            sf::RectangleShape panel({float(WIN_W) - 100.f, float(WIN_H) - 80.f});
            panel.setPosition({40.f, 30.f});
            panel.setFillColor(sf::Color(22, 22, 58));
            panel.setOutlineColor(sf::Color(110, 110, 210));
            panel.setOutlineThickness(3.f);
            window.draw(panel);

            sf::Text title(font, "UNO  RULES", 50);
            title.setFillColor(sf::Color(200, 200, 255));
            title.setStyle(sf::Text::Bold);
            sf::FloatRect tb = title.getLocalBounds();
            title.setOrigin({tb.size.x / 2.f, 0.f});
            title.setPosition({float(WIN_W) / 2.f, 90.f});
            window.draw(title);

            const char *rules[] = {
                                   "1.  Each player starts with 7 cards.",
                                   "2.  Match the top card by COLOR or NUMBER.",
                                   "3.  SKIP:             Next player loses their turn.",
                                   "4.  REVERSE:    Acts as Skip in a 2-player game.",
                                   "5.  DRAW TWO: Next player draws 2 cards.",
                                   "6.  WILD:             Choose any color to continue play.", 
                                   "                          If you donot have any cards of that color, your turn ends immediately.",
                                   "7.  WILD DRAW 4: Choose a color; next player draws 4; similar wild logic.",
                                   "8.  If you cannot play, draw one card from the deck.",
                                   "9.  First player to empty their hand WINS!"};

            float ry = 190.f;
            for (auto &r : rules)
            {
                sf::Text rt(font, r, 24);
                rt.setFillColor(sf::Color(215, 215, 215));
                rt.setPosition({180.f, ry});
                window.draw(rt);
                ry += 36.f;
            }

            // Back button — x/y/w/h MUST match hit-rect above
            drawButton(window, font, "Back",
                       583, 754, 200, 46, sf::Color(70, 70, 140));
        }

        // ---- GAME SCREEN ----------------------------------------
        else if (screen == Screen::GAME)
        {
            TurnPhase phase = gm->getTurnPhase();

            // ---- WIN OVERLAY ------------------------------------
            if (phase == TurnPhase::GAME_OVER)
            {
                // Dark full-screen tint
                sf::RectangleShape tint({float(WIN_W), float(WIN_H)});
                tint.setFillColor(sf::Color(0, 0, 0, 185));
                window.draw(tint);

                // Winner panel
                sf::RectangleShape panel({480.f, 220.f});
                panel.setPosition({310.f, 205.f});
                panel.setFillColor(sf::Color(22, 55, 22));
                panel.setOutlineColor(sf::Color::Yellow);
                panel.setOutlineThickness(4.f);
                window.draw(panel);

                std::string winMsg = gm->getWinner()->getName() + "  WINS!";
                sf::Text wt(font, winMsg, 44);
                wt.setFillColor(sf::Color::Yellow);
                wt.setStyle(sf::Text::Bold);
                sf::FloatRect wb = wt.getLocalBounds();
                wt.setOrigin({wb.size.x / 2.f, 0.f});
                wt.setPosition({float(WIN_W) / 2.f, 228.f});
                window.draw(wt);

                sf::Text sub(font, "Congratulations!", 22);
                sub.setFillColor(sf::Color::White);
                sf::FloatRect sb = sub.getLocalBounds();
                sub.setOrigin({sb.size.x / 2.f, 0.f});
                sub.setPosition({float(WIN_W) / 2.f, 298.f});
                window.draw(sub);

                sf::Text hint(font, "Press  'New Game'  to play again.", 16);
                hint.setFillColor(sf::Color(180, 180, 180));
                sf::FloatRect hb = hint.getLocalBounds();
                hint.setOrigin({hb.size.x / 2.f, 0.f});
                hint.setPosition({float(WIN_W) / 2.f, 336.f});
                window.draw(hint);

                // New Game button — x/y/w/h MUST match hit-rect above
                drawButton(window, font, "New Game",
                           370, 390, 280, 52, sf::Color(45, 110, 45));

                window.display();
                continue; // skip the normal game draw below
            }

            // ---- NORMAL GAME DRAW -------------------------------
            Player *cur = gm->getCurrentPlayer();
            Player *other = gm->getOtherPlayer();

            // Automatic UNO detection: show a centred "UNO!" dialog
            // when a player's hand size becomes exactly 1 (only show once per player)
            int ci = gm->getCurrentPlayerIdx();
            int szCur = cur->getHandSize();
            int szOther = other->getHandSize();
            if (szCur == 1 && lastUnoShownFor != ci)
            {
                showUno = true;
                unoClock.restart();
                lastUnoShownFor = ci;
            }
            else if (szOther == 1 && lastUnoShownFor != (1 - ci))
            {
                showUno = true;
                unoClock.restart();
                lastUnoShownFor = 1 - ci;
            }

            // Green felt background
            sf::RectangleShape felt({float(WIN_W) - 80.f, float(WIN_H) - 60.f});
            felt.setPosition({40.f, 30.f});
            felt.setFillColor(sf::Color(128, 0, 0));
            felt.setOutlineColor(sf::Color::Yellow);
            felt.setOutlineThickness(3.f);
            window.draw(felt);

            // Title
            sf::Text title(font, "GAME IN SESSION", 24);
            title.setFillColor(sf::Color(170, 255, 170));
            title.setStyle(sf::Text::Bold);
            sf::FloatRect titleB = title.getLocalBounds();
            title.setOrigin({titleB.size.x / 2.f, 0.f});
            title.setPosition({float(WIN_W) / 2.f, 40.f});
            window.draw(title);

            // Turn indicator
            sf::Text turnTxt(font, cur->getName() + "'s  Turn", 17);
            turnTxt.setFillColor(sf::Color(255, 215, 80));
            sf::FloatRect ttb = turnTxt.getLocalBounds();
            turnTxt.setOrigin({ttb.size.x / 2.f, 0.f});
            turnTxt.setPosition({float(WIN_W) / 2.f, 72.f});
            window.draw(turnTxt);

            // ---- LEFT: Player name buttons ----------------------
            // p0 = player added first (index 0), always at y=160
            // p1 = player added second (index 1), always at y=255
            // Active player gets purple highlight + bold name
            {
                int ci = gm->getCurrentPlayerIdx();
                Player *p0 = (ci == 0) ? cur : other;
                Player *p1 = (ci == 0) ? other : cur;

                auto drawPBtn = [&](Player *p, float y)
                {
                    bool active = (p == cur);

                    
                    sf::RectangleShape pb({200.f, 65.f});
                    pb.setPosition({55.f, y});
                    pb.setFillColor(active
                                        ? sf::Color(75, 35, 115)
                                        : sf::Color(35, 35, 75));
                    pb.setOutlineColor(active
                                           ? sf::Color(175, 95, 255)
                                           : sf::Color(75, 75, 120));
                    pb.setOutlineThickness(active ? 3.f : 1.5f);
                    window.draw(pb);

                
                    sf::Text pt(font, p->getName(), 20);
                    if (active)
                        pt.setStyle(sf::Text::Bold);
                    pt.setFillColor(active
                                        ? sf::Color(230, 175, 255)
                                        : sf::Color(175, 175, 175));
                    sf::FloatRect ptb = pt.getLocalBounds();
                    pt.setOrigin({ptb.size.x / 2.f, ptb.size.y / 2.f});
                
                    pt.setPosition({155.f, y + 30.f});
                    window.draw(pt);

                
                    sf::Text cnt(font,
                                 "Cards: " + std::to_string(p->getHandSize()), 15);
                    cnt.setFillColor(sf::Color(190, 190, 190));
                    cnt.setPosition({57.f, y + 70.f});
                    window.draw(cnt);
                };

                
                //   p0 at y=155, p1 at y=265 — more breathing room between them
                drawPBtn(p0, 155.f);
                drawPBtn(p1, 265.f);
            }

            // ---- CENTRE: Top Card -------------------------------
        
            sf::Text tcLbl(font, "DISCARD PILE  ", 20);
            tcLbl.setFillColor(sf::Color(190, 255, 190));
            tcLbl.setPosition({570.f, 150.f});
            window.draw(tcLbl);


            if (gm->getTopCard())
                drawUnoCard(window, font, gm->getTopCard(), 550.f, 175.f, 150.f, 200.f);

            // ---- RIGHT: Draw Card button ------------------------
            
        
            {
                sf::RectangleShape db({200.f, 140.f});
                db.setPosition({1080.f, 290.f});
                db.setFillColor(sf::Color(80, 28, 28));
                db.setOutlineColor(sf::Color(200, 75, 75));
                db.setOutlineThickness(2.f);
                window.draw(db);

    
                sf::Text dt(font, "DRAW\nCARD", 26);
                dt.setFillColor(sf::Color::White);
                dt.setStyle(sf::Text::Bold);
                sf::FloatRect dtb = dt.getLocalBounds();
                dt.setOrigin({dtb.size.x / 2.f, dtb.size.y / 2.f});
            
                dt.setPosition({1180.f, 360.f});
                window.draw(dt);
            }

            // ---- BOTTOM: Current player's hand ------------------
            {
                const auto &hand = cur->getHand();
                int sz = (int)hand.size();

            
                float cw = 118.f;
                float gap = 8.f;
                float totalW = sz * (cw + gap) - gap;
                float availW = float(WIN_W) - 100.f;

                // Shrink cards proportionally if hand is too wide
                if (totalW > availW)
                {
                    cw = (availW - gap * (sz - 1)) / sz;
                    gap = (sz > 1)
                              ? (availW - cw * sz) / (sz - 1)
                              : 0.f;
                    totalW = sz * (cw + gap) - gap;
                }

                float startX = std::max(50.f,
                                        (float(WIN_W) - totalW) / 2.f);
                
                float cardY = float(WIN_H) - 200.f;

                
                sf::Text handLbl(font, cur->getName() + "'s  Hand:", 17);
                handLbl.setFillColor(sf::Color(170, 255, 170));
                handLbl.setPosition({startX, float(WIN_H) - 222.f});
                window.draw(handLbl);

                for (int i = 0; i < sz; i++)
                {
                    bool hi = (hand[i] == gm->getLastDrawnCard());
                    
                    drawUnoCard(window, font, hand[i],
                                startX + i * (cw + gap), cardY,
                                cw, 160.f, hi);
                }
            }

            // ---- PHASE OVERLAYS ---------------------------------

            // AWAITING_DRAW_DECISION
            if (phase == TurnPhase::AWAITING_DRAW_DECISION)
            {
            
                sf::RectangleShape ov({440.f, 90.f});
                ov.setPosition({320.f, 470.f});
                ov.setFillColor(sf::Color(12, 12, 52, 238));
                ov.setOutlineColor(sf::Color::White);
                ov.setOutlineThickness(1.5f);
                window.draw(ov);

                
                sf::Text msg(font, "You drew a playable card!", 18);
                msg.setFillColor(sf::Color::White);
                sf::FloatRect mb = msg.getLocalBounds();
                msg.setOrigin({mb.size.x / 2.f, 0.f});
                msg.setPosition({540.f, 477.f});
                window.draw(msg);

            
                drawButton(window, font, "Play It",
                           360, 510, 180, 48, sf::Color(35, 100, 35), 20);
                drawButton(window, font, "Keep It",
                           570, 510, 180, 48, sf::Color(110, 60, 15), 20);
            }

            // CHOOSE_COLOR
            if (phase == TurnPhase::CHOOSE_COLOR)
            {
        
                sf::RectangleShape ovBg({530.f, 210.f});
                ovBg.setPosition({310.f, 230.f});
                ovBg.setFillColor(sf::Color(8, 8, 38, 242));
                ovBg.setOutlineColor(sf::Color::White);
                ovBg.setOutlineThickness(2.f);
                window.draw(ovBg);

    
                sf::Text pick(font, "Choose a Color:", 24);
                pick.setFillColor(sf::Color::White);
                sf::FloatRect pb = pick.getLocalBounds();
                pick.setOrigin({pb.size.x / 2.f, 0.f});
                pick.setPosition({float(WIN_W) / 2.f, 240.f});
                window.draw(pick);

                // Colour squares — x/y/w/h MUST match hit-rects above
                Color cols[4] = {Color::RED, Color::BLUE,
                                 Color::GREEN, Color::YELLOW};
                const char *nms[4] = {"Red", "Blue", "Green", "Yellow"};

        
                float cx = 310.f, cy = 280.f,
                      bw = 110.f, bh = 110.f, bgap = 15.f;

                for (int i = 0; i < 4; i++)
                {
                    sf::RectangleShape cb({bw, bh});
                    cb.setPosition({cx + i * (bw + bgap), cy});
                    cb.setFillColor(toSFColor(cols[i]));
                    cb.setOutlineColor(sf::Color::White);
                    cb.setOutlineThickness(2.f);
                    window.draw(cb);
            
                    sf::Text nl(font, nms[i], 17);
                    nl.setFillColor(sf::Color::White);
                    sf::FloatRect nb = nl.getLocalBounds();
                    nl.setOrigin({nb.size.x / 2.f, nb.size.y / 2.f});
                    nl.setPosition({cx + i * (bw + bgap) + bw / 2.f,
                                    cy + bh / 2.f});
                    window.draw(nl);
                }
            }

            // Play_Choosen_Card: colour hint
            if (phase == TurnPhase::Play_Choosen_Card)
            {
                std::string hint = "Play a  " +
                                   colorToString(gm->getPendingWildColor()) +
                                   "  card  (or any Wild)";

            
                sf::Text info(font, hint, 20);
                info.setFillColor(toSFColor(gm->getPendingWildColor()));
                info.setStyle(sf::Text::Bold);
                sf::FloatRect ib = info.getLocalBounds();
                info.setOrigin({ib.size.x / 2.f, 0.f});
                
                info.setPosition({float(WIN_W) / 2.f, 450.f});
                window.draw(info);
            }

            if (showMsg)
            {
                if (msgClock.getElapsedTime().asSeconds() >= 2.f)
                    showMsg = false;
                else
                {
        
                    //   cardY = WIN_H - 200 = 620, so we place the box at y = 620 - 70 = 550
                    //   Box centred horizontally: x = (WIN_W - 500) / 2 = 433
                    drawButton(window, font, "Unplayable card! Turn passed.",
                               433, 550, 500, 52, sf::Color(20, 20, 60));
                }
            }

            if (showUno)
            {
                if (unoClock.getElapsedTime().asSeconds() >= 3.f)
                    showUno = false;
                else
                {
                    float bw = 420.f, bh = 140.f;
                    float bx = (WIN_W - bw) / 2.f;
                    float by = (WIN_H - bh) / 2.f;
                    drawButton(window, font, "UNO!", bx, by, bw, bh, sf::Color(22, 55, 22), 72);
                }
            }

        } // end GAME screen draw

        window.display();

    } // end main loop
    
return 0;
}