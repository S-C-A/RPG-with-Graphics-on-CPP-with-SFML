#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include "../game.h"
#include "ui_elements.h" // LEFT_WIDTH, SPLIT_Y, vs icin

// ============================================================
//  ENEMY TARGET - Savaş Ekranındaki Düşman Görseli
// ============================================================
struct EnemyTarget {
    sf::RectangleShape shape;
    sf::Color normalColor;
    sf::Color hoverColor;
    int index; // Hangi siradaki dusman oldugunu tutabilmek icin

    EnemyTarget(float x, float y, int _index) : index(_index) {
        shape.setSize({100.f, 150.f}); 
        shape.setPosition({x, y});
        normalColor = sf::Color(50, 0, 0);   
        hoverColor = sf::Color(255, 50, 50); 
        shape.setFillColor(normalColor);
        shape.setOutlineThickness(2.f);
        shape.setOutlineColor(sf::Color::Black);
    }

    void update(sf::Vector2f mousePos) {
        if (shape.getGlobalBounds().contains(mousePos)) {
            shape.setFillColor(hoverColor); 
        } else {
            shape.setFillColor(normalColor); 
        }
    }

    bool isClicked(sf::Vector2f mousePos) const {
        return shape.getGlobalBounds().contains(mousePos);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(shape);
    }
};

// ============================================================
//  ITEM TARGET - Yerdeki Esyanin Gorseli
// ============================================================
struct ItemTarget {
    sf::RectangleShape shape;

    ItemTarget(float x, float y) {
        shape.setSize({40.f, 40.f});
        shape.setPosition({x, y});
        shape.setFillColor(sf::Color::Yellow);
        shape.setOutlineThickness(2.f);
        shape.setOutlineColor(sf::Color(200, 200, 0));
        shape.setRotation(sf::degrees(45.f));
        // Merkezini orijin yap ki donme duzgun olsun
        shape.setOrigin({20.f, 20.f});
    }

    void update(sf::Vector2f mousePos) {
        if (shape.getGlobalBounds().contains(mousePos)) {
            shape.setScale({1.1f, 1.1f});
            shape.setFillColor(sf::Color::White);
        } else {
            shape.setScale({1.0f, 1.0f});
            shape.setFillColor(sf::Color::Yellow);
        }
    }

    bool isClicked(sf::Vector2f mousePos) const {
        return shape.getGlobalBounds().contains(mousePos);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(shape);
    }
};

// ============================================================
//  NPC TARGET - Odadaki NPC'nin Gorseli
// ============================================================
struct NPCTarget {
    sf::RectangleShape shape;

    NPCTarget(float x, float y) {
        shape.setSize({50.f, 50.f});
        shape.setPosition({x, y});
        shape.setFillColor(sf::Color::Cyan);
        shape.setOutlineThickness(2.f);
        shape.setOutlineColor(sf::Color::Blue);
        shape.setRotation(sf::degrees(45.f));
        shape.setOrigin({25.f, 25.f});
    }

    void update(sf::Vector2f mousePos) {
        if (shape.getGlobalBounds().contains(mousePos)) {
            shape.setScale({1.1f, 1.1f});
            shape.setFillColor(sf::Color::White);
        } else {
            shape.setScale({1.0f, 1.0f});
            shape.setFillColor(sf::Color::Cyan);
        }
    }

    bool isClicked(sf::Vector2f mousePos) const {
        return shape.getGlobalBounds().contains(mousePos);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(shape);
    }
};

// ============================================================
//  WORLD OBJECTS - Odadaki Tüm Nesnelerin Yöneticisi
// ============================================================
//  application.h'ta sadece bu struct instantiate edilir.
//  Oda degistikce syncWithRoom cagirilir.
struct WorldObjects {
    std::optional<ItemTarget> groundItem;
    std::optional<NPCTarget> npc;
    std::vector<EnemyTarget> enemies; // Odadaki dusmanlar

    // Odaya girildiginde objeleri guncelle
    void syncWithRoom(Game& game) {
        Room* r = game.getCurrentRoom();
        
        // Önce düşmanları temizle ve yeniden oku
        enemies.clear();
        if (r && !r->monsterID.empty()) {
            int totalMonsters = r->monsterID.size();
            float centerX = GAME_START_X + (LEFT_WIDTH / 2.f);
            float centerY = SPLIT_Y / 2.f;
            
            // X ekseni yerlesim ayarlari
            float spacing = 150.f; // Iki dusman arasi mesafe
            float startX = centerX - ((totalMonsters - 1) * spacing) / 2.f;

            for (size_t i = 0; i < r->monsterID.size(); i++) {
                if (r->monsterID[i] != -1) {
                    // -50 f: 100(genislik) / 2, -75 f: 150(yukseklik) / 2
                    enemies.emplace_back(startX + (i * spacing) - 50.f, centerY - 75.f, (int)i);
                }
            }
        }

        bool hasEnemies = !enemies.empty();

        // Dusman VARSA baska hicbir seye (Esya, NPC) tiklanmasin diye onlari gosterme
        if (r && r->itemID != -1 && !hasEnemies) {
            float centerX = GAME_START_X + (LEFT_WIDTH / 2.f);
            float centerY = SPLIT_Y / 2.f;
            groundItem.emplace(centerX, centerY + 30.f);
        } else {
            groundItem.reset();
        }

        if (r && r->npcID != -1 && !hasEnemies) {
            float centerX = GAME_START_X + (LEFT_WIDTH / 2.f);
            float centerY = SPLIT_Y / 2.f;
            npc.emplace(centerX + 80.f, centerY + 30.f); 
        } else {
            npc.reset();
        }
    }

    // Her frame hover kontrolü
    void update(sf::Vector2f mousePos) {
        for (auto& enemy : enemies) {
            enemy.update(mousePos);
        }
        if (groundItem) groundItem->update(mousePos);
        if (npc) npc->update(mousePos);
    }

    // Tıklama kontrolü (Sol tık)
    // Eğer eşyaya tıklandıysa game.tryPickupItem() cagirir ve mesaji doner.
    // Tıklanmadiysa bos string döner.
    std::string handleLeftClick(Game& game, sf::Vector2f mousePos) {
        if (groundItem && groundItem->isClicked(mousePos)) {
            std::string msg = game.tryPickupItem();
            // Backend esyayi verdiyse (yer vardiysa) veya baska bir durum olduysa,
            // ekranin guncellenmesi lazim (hâlâ yerde mi, alindi mi?)
            syncWithRoom(game);
            return msg;
        }
        return "";
    }

    // NPC Tıklama kontrolü (Sol tık) - Tiklanirsa NPC'yi doner
    NPC* handleLeftClickNPC(Game& game, sf::Vector2f mousePos) {
        if (npc && npc->isClicked(mousePos)) {
            NPC* roomNPC = game.getRoomNPC();
            return roomNPC;
        }
        return nullptr;
    }

    // Dusman Tiklama kontrolu - Hangi dusmana tiklandiginin index'ini (0,1,2..) doner. Secilmediyse -1
    int handleLeftClickEnemy(sf::Vector2f mousePos) const {
        for (const auto& enemy : enemies) {
            if (enemy.isClicked(mousePos)) {
                return enemy.index;
            }
        }
        return -1;
    }

    // Çizim
    void draw(sf::RenderWindow& window) {
        for (auto& enemy : enemies) {
            enemy.draw(window);
        }
        if (groundItem) groundItem->draw(window);
        if (npc) npc->draw(window);
    }
};
