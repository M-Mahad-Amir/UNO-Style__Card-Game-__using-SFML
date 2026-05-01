#include "Enums.h"
#include "Card.h"
#include "Deck.h"
#include "Player.h"
using namespace std;

// only declarations here, definitions in GameManager.cpp

class GameManager{
    private:
    static GameManager *instance;
    vector<Player *> players;
    int currentPlayerIdx;
    Card *topCard;
    bool gameOver;

    public:
     static GameManager *getInstance();
     void addPlayer(Player *p);
     Player *getCurrentPlayer();
     Player *getOtherPlayer();
     void advanceTurn();
     bool isPlayable(Card *card);
     void playCard(Player *p, Card *card);
     void applyEffect(Card *card);
     void playerTurn();
     void startGame();
     void runGame();
};