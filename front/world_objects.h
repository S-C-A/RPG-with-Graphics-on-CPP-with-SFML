#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include "../game.h"
#include "ui_elements.h" // LEFT_WIDTH, SPLIT_Y, vs icin

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
//  WORLD OBJECTS - Odadaki Tüm Nesnelerin Yöneticisi
// ============================================================
//  application.h'ta sadece bu struct instantiate edilir.
//  Oda degistikce syncWithRoom cagirilir.
struct WorldObjects {
    std::optional<ItemTarget> groundItem;

    // Odaya girildiginde (veya esya alindiginda) objeleri guncelle
    void syncWithRoom(Game& game) {
        Room* r = game.getCurrentRoom();
        if (r && r->itemID != -1) {
            // Ekranin ortasina yerlestir
            float centerX = GAME_START_X + (LEFT_WIDTH / 2.f);
            float centerY = SPLIT_Y / 2.f;
                
            groundItem.emplace(centerX, centerY + 30.f);
        } else {
            groundItem.reset();
        }
    }

    // Her frame hover kontrolü
    void update(sf::Vector2f mousePos) {
        if (groundItem) {
            groundItem->update(mousePos);
        }
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

    // Çizim
    void draw(sf::RenderWindow& window) {
        if (groundItem) {
            groundItem->draw(window);
        }
    }
};
