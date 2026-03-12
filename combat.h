#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "player.h"
#include "monster.h"
#include "ItemManager.h"

using namespace std;

// CombatResult: Bir hamlenin sonucunu tasiyan basit bir yapi
struct CombatAction {
    string message;
    bool isCrit = false;
    int damage = 0;
};

class CombatManager {
public:
    // UI icin loot toplama ve raporlama
    string collectLootUI(Player* hero, Monster* mob, ItemManager* itemMgr) {
        string report = "";
        
        // (GOLD)
        if (mob->getGold() > 0) {
            hero->goldChange(mob->getGold());
            report += "Found " + to_string(mob->getGold()) + " gold. ";
        }

        // (EXP)
        if (mob->getExp() > 0) {
            hero->addExp(mob->getExp());
            report += "Gained " + to_string(mob->getExp()) + " exp.\n";
        }

        // (ITEMS)
        const vector<int>& drops = mob->getLootList();
        for (int itemID : drops) {
            Item* newItem = itemMgr->getItem(itemID);
            if (newItem) {
                if (hero->addItem(newItem)) {
                    report += "Loot: [" + newItem->getName() + "]. ";
                } else {
                    report += "Bag Full! [" + newItem->getName() + "] lost. ";
                    delete newItem;
                }
            }
        }
        return report;
    }

    // Basit player saldirisi
    string attackTarget(Player* hero, Monster* target) {
        int damage = hero->getAtk() - target->getDef();
        if (damage < 0) damage = 0;
        
        target->takeDamage(damage);
        
        string msg = "Hit! Dealt " + to_string(damage) + " damage to " + target->getName() + ".";
        if (target->isDead()) {
            msg += "\n" + target->getName() + " defeated!";
        }
        return msg;
    }
    
    // NOT: Dusman turn'u Monster->makeMove uzerinden isletilecek.
    // Monster::makeMove artik string donmeli ki Typewriter'da basabilelim.
};