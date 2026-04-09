#pragma once
#include "monster.h"
#include "player.h"  
#include "condition.h"  
#include <iostream>
#include <cstdlib>   

class BanditSlasher : public Monster {
public:
    BanditSlasher(int id) 
        : Monster(id, "Bandit Slasher", "A dangerous bandit.", 24, 5, 3, 10, 15) 
    {loot.push_back(1);}

    Monster* clone() const override {
        return new BanditSlasher(*this);
    }

    string makeMove(Player* target) override {
        string msg = ">> " + name + ": ";
        if (target->hasStatus(MARK)) {
            target->hurt(atk);
            target->addStatus(StatusEffect(BLEED, 3, 3));
            msg += "sees the MARK and strikes wildly! CRITICAL! " + to_string(atk) + " damage and deep bleeding!";
        }
        else {
            srand(time(0));
            int choice = rand() % 3;

            switch (choice) {
            case 0:
                target->hurt(atk); 
                msg += "swings its blade! " + to_string(atk) + " damage dealt.";
                break;

            case 1:
                target->hurt(2);
                target->addStatus(StatusEffect(BLEED, 2, 3)); 
                msg += "makes a sneaky move! 2 damage and slight bleeding.";
                break;

            case 2:
                target->addStatus(StatusEffect(MARK, 0, 3)); 
                msg += "stares into your soul... YOU ARE MARKED!";
                break;
            }
        }
        return msg;
    }
};
