#pragma once
#include <iostream>
#include <string>
#include <vector>
using namespace std;

class Player; 

class Monster {
protected: 
    int ID;
    string name;
    string info;
    
    int max_hp;
    int hp;
    int atk;
    int def;
    int exp;
    int gold; 
    vector<int> loot; 

public:
    // --- CONSTRUCTORS ---
    Monster() : ID(0), name("Unknown"), info("Unknown"), max_hp(1), hp(1), atk(0), def(0), exp(0), gold(0) {}
    
    Monster(int _id, string _name, string _info, int _max_hp, int _atk, int _def, int _exp, int _gold) 
        : ID(_id), name(_name), info(_info), max_hp(_max_hp), hp(_max_hp), atk(_atk), def(_def), exp(_exp), gold(_gold) {}

    virtual ~Monster() {}

    virtual Monster* clone() const {
        return new Monster(*this);
    }

    virtual string makeMove(Player* target);

    string getName() const { return name; }
    string getInfo() const { return info; }
    int getAtk() const { return atk; }
    int getDef() const { return def; }
    int getExp() const { return exp; }
    int getGold() const { return gold; }
    int getHp() const { return hp; }
    int getMaxHp() const { return max_hp; }

    const vector<int>& getLootList() const {
        return loot;
    }

    void addLoot(int itemID) {
        loot.push_back(itemID);
    }

    void takeDamage(int amount){
        hp -= amount;
        if (hp < 0) hp = 0;
    }

    bool isDead() const {
        return hp <= 0;
    }
};


#include "player.h"

inline string Monster::makeMove(Player* target) {
    if (target != nullptr) {
        target->hurt(atk);
        return name + " attacks you for " + to_string(atk) + " damage!";
    }
    return "";
}