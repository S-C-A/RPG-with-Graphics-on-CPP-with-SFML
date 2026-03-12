#pragma once
#include <string>
#include <vector>
#include <map>
#include <iostream>

using namespace std;

enum EventType {
    EVENT_NONE,         // Hicbir sey yapma
    EVENT_GIVE_ITEM,    // Oyuncuya esya ver (Value = Item ID)
    EVENT_TAKE_ITEM,    // Oyuncudan esya al (Value = Item ID) - Ilerde eklenebilir
    EVENT_HEAL,         // Can doldur (Value = HP miktari)
    EVENT_START_COMBAT,  // Savas baslat (Value = Monster ID)
    EVENT_OPEN_SHOP
};

struct ShopItem {
    int itemID;
    int price;
    ShopItem(int id, int p) : itemID(id), price(p) {}
};

// Oyuncunun secebilecegi cevap secenegi
struct DialogueOption {
    string text;     // Ekranda gorunecek yazi: "Nasılsın?"
    int nextNodeID;  // Bu secilirse hangi ID'li cevaba gidilecek? (-1 ise konusma biter)

    DialogueOption(string t, int next) : text(t), nextNodeID(next) {}
};

// NPC'nin tek bir konusma balonu
struct DialogueNode {
    int id;
    string npcText; // NPC'nin soyledigi: "Merhaba yabanci."
    vector<DialogueOption> options; // Oyuncunun secebilecegi cevaplar listesi
    
    int changeRootTo;       // Diyalog akisi degisimi (Eski SET_ROOT)
    EventType actionType;   // Oyun dunyasi olayi (Orn: GIVE_ITEM)
    int actionValue;        // Olayin parametresi (Orn: Item ID 1)

    DialogueNode(int _id = 0, string _text = "") 
        : id(_id), npcText(_text), changeRootTo(-1) {} // Varsayilan -1 (Degistirme)
};

class NPC {
private:
    int id;             // NPC'nin numarasi (50, 51 vb.)
    string name;        // NPC'nin Adi (Yasli Bilge)
    bool metBefore;     // Daha once karsilastik mi? (Evet/Hayir)
    int currentRootNode; // YENI: Konusmanin baslayacagi varsayilan ID
    vector<ShopItem> shopInventory; // YENI: Bu NPC'nin sattigi seyler
    
    // Anahtar (Key): Node ID -> Deger (Value): DialogueNode
    map<int, DialogueNode> dialogueTree; 

public:
    // --- (Constructor) ---
    NPC(int _id, string _name) : id(_id), name(_name) {
        metBefore = false;
        currentRootNode = 0;
    }

    // --- GETTER / SETTER ---
    string getName() const { return name; }
    int getID() const { return id; }
    bool hasMet() const { return metBefore; }
    void setMet(bool status) { metBefore = status; }
    int getRootNode() const { return currentRootNode; }
    const vector<ShopItem>& getShopInventory() const {
        return shopInventory;
    }
   
    void setRootNode(int newRoot) { 
        currentRootNode = newRoot; 
    }

    void addShopItem(int id, int price) {
        shopInventory.push_back(ShopItem(id, price));
    }

    // --- DIYALOG YONETIM FONKSIYONLARI ---

    // 1. NPC'ye yeni bir cumle ogretir (Yonetici bunu kullanacak)
    void addDialogueNode(int nodeID, string text) {
        dialogueTree[nodeID] = DialogueNode(nodeID, text);
    }

    // 2. Var olan bir cumleye cevap secenegi ekler
    void addOption(int nodeID, string optionText, int nextNodeID) {
        // Once bu ID'de bir konusma var mi bakalim?
        if (dialogueTree.find(nodeID) != dialogueTree.end()) {
            dialogueTree[nodeID].options.push_back(DialogueOption(optionText, nextNodeID));
        } else {
            // Hata ayiklama icin (Eger ID yanlis girildiyse)
            cout << "HATA: NPC " << name << " icin Node ID " << nodeID << " bulunamadi!" << endl;
        }
    }

    void setNodeAction(int nodeID, int rootChange, EventType type, int val) {
        if (dialogueTree.find(nodeID) != dialogueTree.end()) {
            // Eger -1 degilse guncelle
            if (rootChange != -1) dialogueTree[nodeID].changeRootTo = rootChange;
            
            // Event varsa guncelle
            if (type != EVENT_NONE) {
                dialogueTree[nodeID].actionType = type;
                dialogueTree[nodeID].actionValue = val;
            }
        }
    }

    // 3. Oyun sirasinda istenilen cumleyi getirir
    // Game.h icinde: npc->getDialogue(0) dedigimizde ilk konusmayi dondurur.
    DialogueNode* getDialogue(int nodeID) {
        if (dialogueTree.find(nodeID) != dialogueTree.end()) {
            return &dialogueTree[nodeID];
        }
        return nullptr; // Eger boyle bir konusma yoksa bos doner
    }

    void setNodeAction(int nodeID, int newRoot) {
        if (dialogueTree.find(nodeID) != dialogueTree.end()) {
            dialogueTree[nodeID].changeRootTo = newRoot;
        }
    }
};