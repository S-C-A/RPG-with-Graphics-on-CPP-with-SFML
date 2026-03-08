#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <algorithm> 

// BAGIMLILIKLAR
#include "Player.h"
#include "ItemManager.h"
#include "enemyManager.h"
#include "room.h"
#include "combat.h" 
#include "NPCManager.h" 

using namespace std;

class Game {
private:
    Player hero;
    ItemManager itemMgr;
    EnemyManager mobMgr;
    MapManager mapMgr;
    CombatManager combatMgr;
    NPCManager npcMgr;

    Room* currentRoom;

    string formatText(string text) {
        std::replace(text.begin(), text.end(), '_', ' ');
        return text;
    }

    NPC* activeNPC = nullptr;           // Su an kiminle konusuyoruz?
    DialogueNode* activeNode = nullptr; // Hangi cumledeyiz?
    
    // (YENI) SHOP STATE
    NPC* activeMerchant = nullptr; // Su an kimden alisveris yapiyoruz?

public:
    Game() {
        mapMgr.loadMap("Rooms.txt");
        currentRoom = mapMgr.getRoom(1); 
        if (!currentRoom) std::cerr << "CRITICAL ERROR: Room 1 not found!" << std::endl;

        // BASLANGIC ESYALARI
        Item* sword = itemMgr.getItem(100);
        if (sword) hero.addItem(sword);

        Item* armor = itemMgr.getItem(200);
        if (armor) hero.addItem(armor);

        Item* potion = itemMgr.getItem(1);
        if (potion) hero.addItem(potion);

        Item* key = itemMgr.getItem(300);
        if (key) hero.addItem(key);

        // TEST ICIN ENVANTERI DOLDURMA (Saglikli yontem)
    for (int i = 0; i < 6; i++) {
        Item* newPotion = itemMgr.getItem(1);
        if (newPotion) hero.addItem(newPotion);
    }
    }

    string getItemDesc(int index) {
        if (index < 0 || index >= hero.getInventory().size()) return "";
        return hero.getInventory()[index]->getInfo();
    }

    int getItemValue(int index) {
        if (index < 0 || index >= hero.getInventory().size()) return 0;
        return hero.getInventory()[index]->getPrice();
    }

    string playerUseItem(int index) {
        if (index < 0 || index >= hero.getInventory().size()) return ""; 
        return hero.useItem(index);
    }

    string playerDropItem(int index) {
        return hero.removeItem(index);
    }

    Player& getPlayer() { return hero; }
    Room* getCurrentRoom() { return currentRoom; }

    // --- ACCESSORS for Main.cpp ---
    CombatManager* getCombatManager() { return &combatMgr; }
    EnemyManager* getEnemyManager() { return &mobMgr; }
    ItemManager* getItemManager() { return &itemMgr; }
    
    // Spesifik dusman verisi cekmek icin
    Monster* getMonsterClone(int id) { return mobMgr.getEnemy(id); }

    NPC* getRoomNPC() {
        if (currentRoom && currentRoom->npcID != -1) {
            return npcMgr.getNPC(currentRoom->npcID);
        }
        return nullptr;
    }

    // --- DIYALOG SISTEMI ---

    NPC* checkForAutoDialogue() {
        if (currentRoom && currentRoom->npcID != -1) {
            NPC* npc = npcMgr.getNPC(currentRoom->npcID);
            if (npc && !npc->hasMet()) {
                npc->setMet(true);
                return npc; 
            }
        }
        return nullptr; 
    }

    string startDialogue(NPC* npc) {
        if (!npc) return "";
        activeNPC = npc;
        activeNode = activeNPC->getDialogue(activeNPC->getRootNode());
        if (activeNode) return activeNode->npcText;
        return "...";
    }

    vector<string> getDialogueOptions() {
        vector<string> labels;
        if (!activeNode) return labels;
        for (const auto& opt : activeNode->options) {
            labels.push_back(opt.text); 
        }
        if (labels.empty()) {
            labels.push_back("Continue...");
        }
        return labels;
    }

    string selectDialogueOption(int index) {
        if (!activeNPC || !activeNode) return "";
        int nextID = -1;
        if (!activeNode->options.empty()) {
            if (index >= 0 && index < activeNode->options.size()) {
                nextID = activeNode->options[index].nextNodeID;
            } else {
                return ""; 
            }
        } else {
             nextID = -1; 
        }

        if (nextID == -1) {
            activeNPC = nullptr;
            activeNode = nullptr;
            return ""; 
        }

        activeNode = activeNPC->getDialogue(nextID);
        return processNodeEvents(activeNode);
    }

    string processNodeEvents(DialogueNode* node) {
        if (!node) return "";

        if (node->changeRootTo != -1) {
            activeNPC->setRootNode(node->changeRootTo);
        }

        if (node->actionType == EVENT_GIVE_ITEM) {
            Item* gift = itemMgr.getItem(node->actionValue);
            if (gift) {
                if (hero.addItem(gift)) {
                } else {
                    currentRoom->itemID = node->actionValue; 
                    delete gift;
                }
            }
        }
        else if (node->actionType == EVENT_HEAL) {
            hero.heal(node->actionValue);
        }
        else if (node->actionType == EVENT_START_COMBAT) {
            return "COMBAT_START"; 
        }
        else if (node->actionType == EVENT_OPEN_SHOP) {
            return "SHOP_OPEN"; // Main.cpp bunu yakalayip SHOP moduna gececek
        }

        return node->npcText;
    }

    // --- (YENI) SHOP SISTEMI ---

    // 1. Shop Moduna Gecis (Hazirlik)
    void enterShop() {
        if (activeNPC) {
            activeMerchant = activeNPC;
        }
    }

    // 2. Dukkandan Cikis
    void exitShop() {
        activeMerchant = nullptr;
    }

    // 3. Urun Listesini String Olarak Al (UI icin)
    vector<string> getShopItems() {
        vector<string> list;
        if (!activeMerchant) return list;

        const auto& stock = activeMerchant->getShopInventory();
        for (const auto& itemData : stock) {
            Item* temp = itemMgr.getItem(itemData.itemID);
            if (temp) {
                // Use temp->getPrice() intrinsic value instead of itemData.price
                string entry = temp->getName() + " (" + to_string(temp->getPrice()) + " G)";
                list.push_back(entry);
                delete temp; 
            }
        }
        return list;
    }

    // 4. Satin Alma Islemi
    string buyShopItem(int index) {
        if (!activeMerchant) return "Shop closed.";

        const auto& stock = activeMerchant->getShopInventory();
        if (index < 0 || index >= stock.size()) return "Invalid item.";

        // Instantiate item first to check its true value
        Item* newItem = itemMgr.getItem(stock[index].itemID);
        if (!newItem) return "Item error.";

        int price = newItem->getPrice(); 

        if (hero.getGold() < price) {
            delete newItem; // Clean up since we won't buy it
            return "Not enough gold!";
        }

        if (hero.addItem(newItem)) {
            hero.goldChange(-price);
            return "Bought " + newItem->getName() + "!";
        } else {
            delete newItem;
            return "Inventory full!";
        }
    }

    string sellShopItem(int index) {
        if (!activeMerchant) return "Shop closed.";
        
        if (index < 0 || index >= hero.getInventory().size()) return "Invalid item.";
        
        Item* itemToSell = hero.getInventory()[index];
        
        // Esya silinmeden once ismini ve fiyati aliyoruz!
        string itemName = itemToSell->getName();
        int itemVal = itemToSell->getPrice();
        int sellPrice = (itemVal * 3) / 5;
        
        // Esyayi sil
        string removeMsg = hero.removeItem(index);
        
        if (removeMsg.find("discarded") != string::npos) {
             hero.goldChange(sellPrice);
             // Artik itemToSell pointer'ina erismiyoruz, cunku silindi.
             // Onceden aldigimiz itemName'i kullaniyoruz.
             return "Sold " + itemName + " for " + to_string(sellPrice) + " G.";
        } 
        else {
            return "Cannot sell this item!"; 
        }
    }

    // --- ACTIONS ---

    string attemptMove(int nextRoomID) {
        if (nextRoomID == -1) {
            return "The path is blocked.";
        }
        Room* next = mapMgr.getRoom(nextRoomID);
        if (next) {
            currentRoom = next;
            return lookAtRoom(); 
        }
        return "Unknown path.";
    }

    string tryPickupItem() {
        if (currentRoom->itemID != -1) {
            Item* item = itemMgr.getItem(currentRoom->itemID);
            if (item) {
                if (hero.addItem(item)) {
                    currentRoom->itemID = -1; 
                    return "Picked up [" + item->getName() + "]";
                } else {
                    delete item; 
                    return "Inventory is full. Right click an item to discard it.";
                }
            }
        }
        return "Nothing here.";
    }

    string lookAtRoom() {
        if(!currentRoom) return "Void.";
        string desc = formatText(currentRoom->info);
        if (currentRoom->itemID != -1) {
            Item* tempItem = itemMgr.getItem(currentRoom->itemID);
            if (tempItem) {
                desc += "\nYou see a [" + tempItem->getName() + "] here.";
                delete tempItem; 
            }
        }
        NPC* npc = getRoomNPC();
        if (npc) {
            desc += "\n\n[" + npc->getName() + "] is standing here.";
        }
        return desc;
    }
    
    void addGoldCheat(int amount) {
        hero.goldChange(amount);
    }
};