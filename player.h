#pragma once
#include <string>
#include <iostream>
#include <vector>
#include "condition.h"

class Item;
class Weapon;
class Armor;

class Player {
private:
    int max_hp;
    int hp;
    int atk;
    int def;
    int exp;
    int lvl;
    int gold;
    int current_room;
    Weapon* equippedWeapon;
    Armor* equippedArmor;

    std::vector<StatusEffect> statusActive;
    std::vector<Item*> inventory;

    const int MAX_LVL = 5;

    const int lvl_need[5] = {20, 45, 75, 120, 180};
    const int hp_table[5] = {40, 65, 100, 150, 230};
    const int base_atk[5] = {5, 9, 15, 22, 30};
    const int base_def[5] = {3, 7, 11, 16, 22};

    void lvlUp() {
        if (lvl < MAX_LVL) {
            lvl++;
            max_hp = hp_table[lvl - 1];
            atk = base_atk[lvl - 1];
            def = base_def[lvl -1];
            std::cout << ">>> TEBRIKLER! SEVIYE " << lvl << " OLDUNUZ! <<<" << std::endl;
        }
    }

public:
    Player() {
        lvl = 1;
        exp = 0;
        gold = 17;
        max_hp = hp_table[0];
        hp = max_hp;
        atk = base_atk[0];
        def = base_def[0];
        current_room = 0;
        equippedWeapon = nullptr;
        equippedArmor = nullptr;
    }

    int getHp() { return hp; }
    int getMaxHp() { return max_hp; }
    int getLvl() { return lvl; }
    int getExp() { return exp; }
    int getGold() { return gold; }
    int getAtk();
    int getDef();
    std::string getWeaponName();
    std::string getArmorName();
    

    void addStatus(StatusEffect effect){
        statusActive.push_back(effect);
    }

    bool hasStatus(StatusType typeCheck){
        for (size_t i = 0; i < statusActive.size(); i++)
        {
            if (statusActive[i].type == typeCheck) return true;
        }
        return false;
    }

    std::string updateStatus() {
        std::string report = "";
        for (int i = (int)statusActive.size() - 1; i >= 0; i--) 
        {
            StatusEffect& effect = statusActive[i];
            bool damageApplied = false;

            switch (effect.type) 
            {
            case BLEED:
                hurt(effect.power);
                report += "You bleed away " + std::to_string(effect.power) + " HP...\n";
                damageApplied = true;
                break;
            
            case POISON:
                hurt(effect.power);
                report += "Poison eats " + std::to_string(effect.power) + " HP...\n";
                damageApplied = true;
                break;

            case BURN:
                hurt(effect.power);
                report += "Fire burns " + std::to_string(effect.power) + " HP...\n";
                damageApplied = true;
                break;
            
            default:
                break;
            }
            effect.duration--;

            if (effect.duration <= 0) {
                StatusType type = effect.type;
                std::string typeName = "An effect";
                if (type == BLEED) typeName = "Bleeding";
                else if (type == POISON) typeName = "Poison";
                else if (type == BURN) typeName = "Burn";
                else if (type == STR_BUFF) typeName = "Strength buff";
                else if (type == DEF_BUFF) typeName = "Defense buff";
                else if (type == MARK) typeName = "Mark";
                
                report += typeName + " has worn off.\n";
                statusActive.erase(statusActive.begin() + i);
            }
        }
        return report;
    }

    const std::vector<Item*>& getInventory() const {
        return inventory;
    }

    bool addItem(Item* item);

    std::string equipArmor(Armor* newArmor);   
    std::string equipWeapon(Weapon* newWeapon);
    std::string useItem(int index);           

    std::string removeItem(int index);

    void addExp(int amount) {
        if (lvl >= MAX_LVL) return;

        exp += amount;

        while (lvl < MAX_LVL && exp >= lvl_need[lvl - 1]) {
            exp -= lvl_need[lvl - 1];
            lvlUp();
        }
    }

    void roomChange(int room){
        current_room = room;
    }
   
    bool isAlive() const {
        return hp <= 0;
    }

    void goldChange(int amount){
        gold = gold + amount;
        if (gold < 0)
        {
            gold = 0;
        }
    }

    void heal(int amount){
        hp = hp + amount;
        if (hp > max_hp)
        {
            hp = max_hp;
        }
    }

    void hurt(int amount){
        hp = hp - amount;
        if (hp < 0)
        {
            hp = 0;
        }
    }

    void buff(int amount){
        atk = atk + amount;
    }

    void printInventory();
    void printEquipment();

    void printStats() {
        std::cout << "--- Status ---" << std::endl;
        std::cout << "Hp: " << hp << "/" << max_hp << std::endl;
        std::cout << "Strength: " << getAtk() << std::endl;
        std::cout << "Defense: " << getDef() << std::endl;
        std::cout << "Level: " << lvl << (lvl == MAX_LVL ? " (MAX)" : "") << std::endl;
        std::cout << "Gold: " << gold << std::endl;

        if (lvl < MAX_LVL)
            std::cout << "XP: " << exp << " / " << lvl_need[lvl - 1] << std::endl;
        else
            std::cout << "XP: -" << std::endl;

        std::cout << "-------------" << std::endl;
    }
};

#include "item.h"

inline int Player::getAtk() { 
    int totalAtk = atk; 
    
    if (equippedWeapon != nullptr) {
        totalAtk += equippedWeapon->getPower();
    }

    for (size_t i = 0; i < statusActive.size(); i++) {
        if (statusActive[i].type == STR_BUFF) {
            totalAtk += statusActive[i].power; 
        }
    }
    return totalAtk; 
}

inline int Player::getDef() { 
    int totalDef = def;
    
    if (equippedArmor != nullptr) {
        totalDef += equippedArmor->getDefense();
    }

    for (size_t i = 0; i < statusActive.size(); i++) {
        if (statusActive[i].type == DEF_BUFF) {
            totalDef += statusActive[i].power; 
        }
    }

    return totalDef; 
}

inline bool Player::addItem(Item* item){
    if (inventory.size() < 10)
    {
        inventory.push_back(item);
        return true;
    } else {        
        return false;
    }
}

inline std::string Player::useItem(int index){
    if (index >= inventory.size()){
        return ""; // Hatali index, bos don
    }
    Item* itemToUse = inventory[index];
    
    // Eşyanın ürettiği mesajı yakalıyoruz
    std::string resultMessage = itemToUse->use(this);
    
    // Tüketilebilir ise silme mantığı (Aynı kalıyor)
    if (dynamic_cast<Consumable*>(itemToUse)) {
        delete itemToUse;
        inventory.erase(inventory.begin() + index);
    }
    
    return resultMessage; // Mesajı yukarı iletiyoruz
}

inline std::string Player::getWeaponName() {
    if (equippedWeapon != nullptr) {
        return equippedWeapon->getName();
    }
    return "None";
}

inline std::string Player::getArmorName() {
    if (equippedArmor != nullptr) {
        return equippedArmor->getName();
    }
    return "None";
}

inline std::string Player::equipWeapon(Weapon* newWeapon){
    if (newWeapon == nullptr) return "";

    // Eski silahı çantadan listeden çıkar (silmeden)
    for (size_t i = 0; i < inventory.size(); i++) {
        if (inventory[i] == newWeapon) {
            inventory.erase(inventory.begin() + i);
            break;
        }
    }
    
    // Mesajı hazırlamaya başla
    std::string msg = "Equipped " + newWeapon->getName() + ".";
    
    Weapon* oldWeapon = equippedWeapon;
    equippedWeapon = newWeapon;

    // Eski silah varsa çantaya koy ve mesaja ekle
    if (oldWeapon != nullptr)
    {
        inventory.push_back(oldWeapon);
        msg += "\n" + oldWeapon->getName() + " returned to bag.";
    }
    
    return msg; // Mesajı döndür
}

inline std::string Player::equipArmor(Armor* newArmor){
    if (newArmor == nullptr) return "";

    for (size_t i = 0; i < inventory.size(); i++) {
        if (inventory[i] == newArmor) {
            inventory.erase(inventory.begin() + i);
            break;
        }
    }
    
    std::string msg = "Equipped " + newArmor->getName() + ".";
    
    Armor* oldArmor = equippedArmor;
    equippedArmor = newArmor;

    if (oldArmor != nullptr)
    {
        inventory.push_back(oldArmor);
        msg += "\n" + oldArmor->getName() + " returned to bag.";
    }
    
    return msg;
}

inline std::string Player::removeItem(int index) {
    // 1. Gecersiz index kontrolu
    if (index < 0 || index >= inventory.size()) {
        return ""; 
    }

    Item* itemToDelete = inventory[index];

    // 2. KORUMA MANTIGI (BURASI AYNI KALIYOR)
    // Eger esya atilamaz (KeyItem) ise, silme islemi yapilmadan fonksiyon biter.
    if (itemToDelete->canDrop() == false) {
        return "You cannot discard [" + itemToDelete->getName() + "]!";
    }

    // 3. SILME ISLEMI (Sadece yukaridaki if gecilirse calisir)
    std::string itemName = itemToDelete->getName();
    
    delete itemToDelete; // Hafizadan sil
    inventory.erase(inventory.begin() + index); // Listeden sil
    
    return "[" + itemName + "] discarded and lost.";
}

inline void Player::printInventory() {
    std::cout << "--- BACKPACK (" << inventory.size() << "/10) ---" << std::endl;
    
    if (inventory.empty()) {
        std::cout << "Cantan bos." << std::endl;
        return;
    }

    for (size_t i = 0; i < inventory.size(); i++) {
        std::cout << "[" << i+1 << "] " << inventory[i]->getName() << std::endl;
    }
    std::cout << "----------------------" << std::endl;
}

inline void Player::printEquipment() {

        std::cout << "-------------" << std::endl;
        std::cout << "Weapon: " << (equippedWeapon == nullptr ? "No Weapon" : equippedWeapon->getName()) << std::endl;
        std::cout << "Armor: " << (equippedArmor == nullptr ? "No Armor" : equippedArmor->getName()) << std::endl;
        std::cout << "-------------" << std::endl;
}

