#include "Enums.h"
#include "Card.h"
using namespace std;

class Player{
private:
    string name;
    vector<Card *> hand;
    bool saidUno;        
public:
    Player(const string &name);
    ~Player();
    // --- Getters ---
    string getName() const;
    bool getSaidUno() const;
    int getHandSize() const;
    const vector<Card *> &getHand() const;
    // --- Mutators ---
    void setSaidUno(bool val);
    void addCard(Card *card);
    void RemovefromHands(Card *c);
    bool hasPlayableCard(const Card *topCard) const;
    bool cmp(Card *c);
};