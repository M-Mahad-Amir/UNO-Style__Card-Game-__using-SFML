#include "Enums.h"
// only declarations here, 
class Card{
protected:
    Color color;   
    CardType type; 
    string id;     
public: 
    // only declarations
    Card(Color color, CardType type);
    virtual ~Card() = default;
    Color getColor() const;
    CardType getType() const;
    string getId() const;
    virtual string getLabel() const = 0;
    virtual void applyEffect() const = 0;
    virtual void print() const;
    virtual int getValue() const;
    virtual bool operator==(const Card &other) const = 0;
};
class NormalCard : public Card{
    private:
    int value;
    public:
    NormalCard(Color color, int v);
    int getValue() const override;
    string getLabel() const override;
    void applyEffect() const override;
    bool operator==(const Card &other) const override;
};
class SpecialCard : public Card{
    private:
    Color chosenColor;
    public:
    SpecialCard(Color color, CardType type);
    void setChosenColor(Color c);
    Color getChosenColor() const;
    string getLabel() const override;
    void applyEffect() const override;
    bool operator==(const Card &other) const override;
};