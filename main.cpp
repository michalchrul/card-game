//
//  main.cpp
//  CardGame
//
//  Created by Michał Chrul on 02/12/2021.
//

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <random>
#include <stdio.h>

class Helpers {
public:
    static int getInteger(int a_min, int a_max) {
        int number;
        std::cin >> number;

        while (std::cin.fail() || number < a_min || number > a_max) {
            std::cout << "Please insert a number between " << a_min << " and " << a_max << std::endl;
            std::cin.clear();
            std::cin.ignore(256,'\n');
            std::cin >> number;
        }
        std::cout << std::endl;
        return number;
    }

    static bool getDecision() {
        std::string input;
        do {
            std::cin >> input;
            if (input != "Y" && input != "y" && input != "N" && input != "n")
                std::cout << "Press Y to play again, press N to exit." << std::endl;
        }  while (input != "Y" && input != "y" && input != "N" && input != "n");

        std::cout << std::endl;
        if (input == "Y" || input == "y")
            return true;
        else if (input == "N" || input == "n")
            return false;
        else
            return false;
    }
};

class Player;

class Card {
public:
    Card(std::string a_suit, std::string a_figure) {
        m_suit = a_suit;
        m_figure = a_figure;
    }
    
    std::string getSuit() {return m_suit;}
    std::string getFigure() {return m_figure;}
    void printCard() {
        std::cout << m_figure << " " << m_suit << std::endl;
    }
    
    static void printCards(std::vector<Card> a_cards) {
        int i = 1;
        for(std::vector<Card>::iterator it = a_cards.begin(); it != a_cards.end(); ++it) {
            std::cout << i++ << ": ";
            it->printCard();
        }
        std::cout << std::endl;
    }
    
    void setOwner(std::shared_ptr<Player> a_owner) {
        m_owner = a_owner;
    }
    
    std::shared_ptr<Player> getOwner() {
        return m_owner;
    }
    
private:
    std::string m_suit;
    std::string m_figure;
    std::shared_ptr<Player> m_owner = nullptr;
};

class CardDeck {
public:
    CardDeck(){
        std::vector<Card> cardDeck;
        for(auto const& suit: m_suits) {
            for(auto const& figure: m_figures) {
                cardDeck.push_back(Card(suit, figure));
            }
        }
        std::mt19937 g(static_cast<uint32_t>(time(0)));
        std::shuffle(std::begin(cardDeck), std::end(cardDeck), g);
        m_cardDeck = cardDeck;
    }
    
    std::vector<Card> getCardDeck() {
        return m_cardDeck;
    }
    
private:
    std::vector<Card> m_cardDeck;
    std::vector<std::string> m_suits = {"♠", "♥", "♣", "♦"};
    std::vector<std::string> m_figures = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"};
};

class Player : public std::enable_shared_from_this<Player>{
public:
    Player(int a_id) {
        m_id = a_id;
        m_isHuman = (a_id == 1) ? true : false;
    }
    
    void makeAMove(std::vector<Card>* a_table){
        int selectedCard = 0;
        std::vector<Card>::iterator selectedCardIter;
        if (m_isHuman) {
            std::cout << std::endl << "Your hand:" << std::endl;
            Card::printCards(m_cards);
            std::cout << "Select a card to play (1-" << m_cards.size() << "):" << std::endl;

            //check if table is empty
            if (a_table->empty()) {
                //yes? then any card is ok
                selectedCard = Helpers::getInteger(1, int(m_cards.size()));
                selectedCardIter = m_cards.begin() + (selectedCard - 1);
            }
            //table not empty?
            else {
                std::string currentSuit = a_table->front().getSuit();
                //check if player has a card of the same suit
                if (playerHasCardOfSuit(currentSuit)) {
                    //yes? then it has to be a card of the same suit
                    do {
                        selectedCard = Helpers::getInteger(1, int(m_cards.size()));
                        selectedCardIter = m_cards.begin() + (selectedCard - 1);
                        if (selectedCardIter->getSuit() != currentSuit) {
                            std::cout << "You must follow suit! (" << currentSuit << ")" << std::endl << std::endl;
                            std::cout << "Your hand:" << std::endl;
                            Card::printCards(m_cards);
                        }
                    }
                    while (selectedCardIter->getSuit() != currentSuit);
                }
                //player does not have a card of the same suit?
                else {
                    //then any card is ok
                    selectedCard = Helpers::getInteger (1, int(m_cards.size()));
                    selectedCardIter = m_cards.begin() + (selectedCard - 1);
                }
            }
        }
        //if player is not a human
        else {
            //check if table is empty
            if (a_table->empty()) {
                //yes? use lowest card of all
                selectedCardIter = getLowestCard();
            }
            //table not empty?
            else {
                std::string currentSuit = a_table->front().getSuit();
                //check if player has a card of the same suit
                if (playerHasCardOfSuit(currentSuit)) {
                    //yes? use the lowest card of this suit
                    selectedCardIter = getLowestCardOfSuit(currentSuit);
                }
                //player does not have a card of the same suit?
                else {
                    //use lowest card of all
                    selectedCardIter = getLowestCard();
                }
            }
        }
        std::cout << "Player " << m_id << " plays ";
        std::cout << selectedCardIter->getFigure() << " " << selectedCardIter->getSuit() << "." << std::endl;
        
        a_table->push_back(*selectedCardIter);
        m_cards.erase(selectedCardIter);
    }
    
    void setCards(std::vector<Card> a_cards) {
        for(std::vector<Card>::iterator it = a_cards.begin(); it != a_cards.end(); ++it) {
            it->setOwner(shared_from_this());
        }
        m_cards = a_cards;
    }
    
    int getId() {
        return m_id;
    }
    
    bool isFirstLower(std::string a_figA, std::string a_figB) {
           auto it_A = m_figuresOrder.find(a_figA);
           auto it_B = m_figuresOrder.find(a_figB);
           return (it_A->second < it_B->second);
       }
       
   std::vector<Card>::iterator getLowestCard() {
       std::vector<Card>::iterator lowestCard = m_cards.begin();
       for(std::vector<Card>::iterator it = m_cards.begin(); it != m_cards.end(); ++it) {
           if (isFirstLower(it->getFigure(), lowestCard->getFigure())) {
               lowestCard = it;
           }
       }
       return lowestCard;
   }

   std::vector<Card>::iterator getLowestCardOfSuit(std::string a_suit) {
       std::vector<Card>::iterator lowestCard;
       for(std::vector<Card>::iterator it = m_cards.begin(); it != m_cards.end(); ++it) {
           if (it->getSuit() == a_suit) {
               lowestCard = it;
           }
       }
       return lowestCard;
   }
    
    bool playerHasCardOfSuit(std::string a_suit) {
        bool hasCard = false;
        for(std::vector<Card>::iterator it = m_cards.begin(); it != m_cards.end(); ++it) {
            if (it->getSuit() == a_suit) {
                hasCard = true;
                break;
            }
        }
        return hasCard;
    }
    
private:
    int m_id;
    bool m_isHuman;
    std::vector<Card> m_cards;
    std::map<std::string, int> m_figuresOrder = {{"2", 0}, {"3", 1}, {"4", 2}, {"5", 3}, {"6", 4}, {"7", 5}, {"8", 6},
                                                {"9", 7}, {"10", 8}, {"J", 9}, {"Q", 10}, {"K", 11}, {"A", 12}};
};

class Game {
public:
    Game(int a_numPlayers) {
        for (int id = 1; id <= a_numPlayers; ++id){
            m_players.push_back(std::shared_ptr<Player>(new Player(id)));
        }
        m_Deck = CardDeck();
        distributeCards();
    };
    
    void distributeCards() {
        int interval = 0;
        for(auto & player: m_players) {
            std::vector<Card>::const_iterator first = m_Deck.getCardDeck().begin() + interval;
            std::vector<Card>::const_iterator last = first + 5;
            
            std::vector<Card> fiveCards(first, last);
            
            player->setCards(fiveCards);
            interval += 5;
        }
    }
    
    void run() {
        for (int round = 0; round < 5; ++round){
            std::cout << "Round " << round + 1 << " begins!" << std::endl << std::endl;
            int startingPlayer =  (round == 0) ? 1 : m_prevRoundWinner;
            std::vector<std::shared_ptr<Player>>::iterator startingPlayerPtr;
            
            //find the player to begin
            std::vector<std::shared_ptr<Player>>::iterator iter = m_players.begin();
            for (iter = m_players.begin(); iter != m_players.end(); ++iter) {
                std::shared_ptr<Player> currPlayer = *iter;
                if (currPlayer->getId() == startingPlayer) {
                    startingPlayerPtr = iter;
                    break;
                }
            }
            
            //iterate from starting player till the last one
            for (iter = startingPlayerPtr; iter != m_players.end(); ++iter ) {
                std::shared_ptr<Player> currPlayer = *iter;
                currPlayer->makeAMove(&m_table);
            }
            
            //iterate from first player till starting player
            for (iter = m_players.begin(); iter != startingPlayerPtr; ++iter) {
                std::shared_ptr<Player> currPlayer = *iter;
                currPlayer->makeAMove(&m_table);
            }
            
            //check who won
            std::vector<Card>::iterator highestCard = m_table.begin();
            for(std::vector<Card>::iterator it = m_table.begin(); it != m_table.end(); ++it) {
                if(it->getSuit() == highestCard->getSuit()) {
                    if (isFirstHigher(it->getFigure(), highestCard->getFigure())) {
                        highestCard = it;
                    }
                }
            }
            
            //print winner and update previous round winner
            int roundWinner = highestCard->getOwner()->getId();
            std::cout << "Player " << roundWinner << " wins the round." << std::endl << std::endl;
            if (round == 4) {
                std::cout << "Player " << roundWinner << " wins the game!" << std::endl;
            }
            m_prevRoundWinner = roundWinner;
            
            //clear the table
            m_table.clear();
        }
    }
    
    bool isFirstHigher(std::string a_figA, std::string a_figB) {
        auto it_A = m_figuresOrder.find(a_figA);
        auto it_B = m_figuresOrder.find(a_figB);
        return (it_A->second > it_B->second);
    }
    
private:
    CardDeck m_Deck;
    std::vector<Card> m_table;
    std::vector<std::shared_ptr<Player>> m_players;
    int m_prevRoundWinner;
    std::map<std::string, int> m_figuresOrder = {{"2", 0}, {"3", 1}, {"4", 2}, {"5", 3}, {"6", 4}, {"7", 5}, {"8", 6},
                                                {"9", 7}, {"10", 8}, {"J", 9}, {"Q", 10}, {"K", 11}, {"A", 12}};
};

int main(int argc, const char * argv[]) {
    bool playAgain;
    do{
        std::cout << "Please insert the number of players (3-5)..." << std::endl;
        int numPlayers = Helpers::getInteger(3, 5);
        
        Game* game = new Game(numPlayers);
        game->run();
        delete game;
        
        std::cout << std::endl << "Play again? Y/N" << std::endl;
        playAgain = Helpers::getDecision();
        std::cout << std::endl;
    }
    while (playAgain);
    return 0;
}
