#include "GameManager.h"

class GameManager
{
private:
    // --- Singleton ---
    static GameManager *instance;

    // --- Game State ---
    Card *topCard; // current top of discard pile
    // Color   currentColor;       // active color (changes after Wild is played)
    vector<Player *> players;
    int currentPlayerIdx; // 0 or 1 for 2-player
    bool gameOver;

    // Private constructor — no one can call `new GameManager()` from outside
    GameManager()
        // : topCard(nullptr), currentColor(Color::RED),
        //   currentPlayerIdx(0), gameOver(false) {}
        : topCard(nullptr), currentPlayerIdx(0), gameOver(false) {}

public:
    // --- Singleton Access ---
    static GameManager *getInstance()
    {
        if (!instance)
            instance = new GameManager();
        return instance;
    }

    // --- Setup ---
    void addPlayer(Player *p) { players.push_back(p); }

    // --- Navigation Helpers ---
    Player *getCurrentPlayer() { return players[currentPlayerIdx]; }
    Player *getOtherPlayer() { return players[1 - currentPlayerIdx]; }
    void advanceTurn() { currentPlayerIdx = 1 - currentPlayerIdx; }

    // --- Playability Check ---
    // Separate from Player::hasPlayableCard because here we check ONE specific card
    bool isPlayable(Card *card)//edited by Mutahir
    {
        if (card->getType() == CardType::WILD ||card->getType() == CardType::WILD_DRAW_FOUR) return true;
        if (card->getColor() == currentColor) return true;
        if (card->getType() == topCard->getType()) {
            if (card->getType() == CardType::NUMBER)
                return card->getValue() == topCard->getValue();
            return true; // same special type always matches
        }
        if (card->getColor() == topCard->getColor())
            return true;
        
        return false;
    }
    // return color==other.getColor()||type==other.getType();   // old operator overloading
    // return color==other.getColor()||value==other.getValue();

    // --- Playing a Card ---
    void playCard(Player *p, Card *card)
    {
        p->RemovefromHands(card);
        delete topCard;
        topCard = card;
        applyEffect(card);
    }

    // --- Effect Dispatcher ---
    void applyEffect(Card *card)
    {
        switch (card->getType())
        {
        case CardType::SKIP:
        case CardType::REVERSE: // Reverse = Skip in 2-player
            // currentColor = card->getColor();
            cout << "[SKIP] " << getOtherPlayer()->getName()
                 << " loses their turn!\n";
            advanceTurn(); // pre-advance → net double toggle = current player goes again
            break;

        case CardType::DRAW_TWO:
            // currentColor = card->getColor();
            cout << "[DRAW TWO] " << getOtherPlayer()->getName()
                 << " draws 2 and is skipped!\n";
            getOtherPlayer()->addCard(UnoDeck::GenerateOneCard());
            getOtherPlayer()->addCard(UnoDeck::GenerateOneCard());
            advanceTurn(); // pre-advance
            break;

        case CardType::WILD:
            // pickColor(); // sets currentColor, no skip
            Wild_and_WildDrawFour(card);
            break;
            
        case CardType::WILD_DRAW_FOUR:
            // pickColor();
            Wild_and_WildDrawFour(card);
            cout << "[WILD DRAW FOUR] " << getOtherPlayer()->getName()
                 << " draws 4 and is skipped!\n";
            for (int i = 0; i < 4; i++)
                getOtherPlayer()->addCard(UnoDeck::GenerateOneCard());
            advanceTurn(); // pre-advance
            break;
            default:
            break;
        }
    }

    // --- Wild Color Picker (console version — SFML popup replaces this later) ---
    // void pickColor()
    // {
    //     cout << "Pick a color — 0:Red  1:Blue  2:Green  3:Yellow: ";
    //     int choice;
    //     cin >> choice;
    //     switch (choice)
    //     {
    //         // case 0: currentColor = Color::RED;    break;
    //         // case 1: currentColor = Color::BLUE;   break;
    //         // case 2: currentColor = Color::GREEN;  break;
    //         // case 3: currentColor = Color::YELLOW; break;
    //         // default: currentColor = Color::RED;

    //         // note:
    //         // card must be removed from currentplayer and that card must be added to topcard
    //     }
    //     // void DrawFourAndWild(Card * card)
    //     // {
    //     //     // note:
            
    //     //     // card must be removed from currentplayer and that card must be added to topcard
    //     //     // cout << "Color chosen: " << colorToString(currentColor) << "\n";
    //     // }
    // }
    void Wild_and_WildDrawFou(Card * card)
        {
            if(isPlayable(card))
            {
             playCard(getCurrentPlayer(),card);
            }//if not playable then advance turn

            
            // cout << "Pick a color — 0:Red  1:Blue  2:Green  3:Yellow: ";
            // int choice;
            // cin >> choice;
            // playCard(getCurrentPlayer(), card);

            // // card must be removed from currentplayer and that card must be added to topcard
            // // cout << "Color chosen: " << colorToString(currentColor) << "\n";
        }

    // --- One Player's Turn ---
    void playerTurn()//why cant we do this in main running loop calling method which needed
    {
        Player *p = getCurrentPlayer();

        cout << "\n========================================\n";
        cout << p->getName() << "'s turn\n";
        cout << "Top card: " << topCard->getLabel()
             //  << "  [Active color: " << colorToString(currentColor) << "]\n";
             << "[Active color: " << colorToString(topCard->getColor()) << "]\n";
        cout << "Your hand:\n";
        const vector<Card *> &hand = p->getHand();
        for (int i = 0; i < (int)hand.size(); i++)
            cout << "  " << i << ": " << hand[i]->getLabel() << "\n";

        cout << "Enter card index to play, or -1 to draw: "; // TO BE DONE BETTER IN GUI (e.g. click card or button instead of typing index)
        int choice;
        cin >> choice;

        if (choice == -1)
        {
            // Draw a card
            Card *drawn = UnoDeck::GenerateOneCard();
            cout << "Drew: " << drawn->getLabel() << "\n";
            // if (isPlayable(drawn))
            // {
            //     cout << "Drawn card is playable — playing it automatically.\n";
            //     playCard(p, drawn);
            // }
            // else
            // {
            //     p->addCard(drawn);
            //     cout << "Not playable. Added to hand. Turn passes.\n";
            // }
            if (isPlayable(drawn))
            {
                cout << "Drawn card is playable. Play it now? (1=Yes / 0=Keep for later): ";
                int playChoice;
                cin >> playChoice;

                if (playChoice == 1)
                {
                    // Play it straight from hand
                    playCard(p, drawn);
                }
                else
                {
                    cout << "Kept in hand. Turn passes. Skip!\n";
                }
            }
            else
            {
                cout << "Not playable — added to hand. Turn passes.\n";
            }
        }
        else if (choice >= 0 && choice < (int)hand.size())
        {
            Card *selected = hand[choice];
            if (isPlayable(selected))
            {
                playCard(p, selected);
            }
            else
            {
                cout << "That card cannot be played. Try again.\n";
                playerTurn(); // re-prompt (fine for console; avoid in GUI)
                return;
            }
        }
        else
        {
            cout << "Invalid input. Try again.\n";
            playerTurn();
            return;
        }

        // Win check
        if (p->getHandSize() == 0)
        {
            cout << "\n*** " << p->getName() << " wins the round! ***\n";
            gameOver = true;
            return;
        }

        // UNO call (stub — penalty logic to add later)
        if (p->getHandSize() == 1)
            cout << "[UNO] " << p->getName() << " has one card left!\n";

        advanceTurn(); // normal end-of-turn advance
    }

    // --- Game Startup ---
    void startGame()
    {
        // Deal 7 cards to each player
        for (Player *p : players)
        {
            vector<Card *> dealt = UnoDeck::GenerateCards(7);
            for (Card *c : dealt)
                p->addCard(c);
        }
        // First top card must not be a Wild
        do
        {
            delete topCard;
            topCard = UnoDeck::GenerateOneCard();
        } while (topCard->getType() == CardType::WILD ||
                 topCard->getType() == CardType::WILD_DRAW_FOUR);

        // currentColor = topCard->getColor();
        cout << "Game started! First card: " << topCard->getLabel() << "\n";
    }

    // --- Main Game Loop ---
    void runGame()
    {
        startGame();
        while (!gameOver)
            playerTurn();
    }
};
