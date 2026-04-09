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
//  ITEM TARGET - Yerdeki Esyanin Gorseli (Loot[Final] Texture)
// ============================================================
//  FLICKER ONLEME: Hover/scale hesaplari icin STATIK bir hitbox
//  kullanilir. Gorsel sprite bu hitbox'tan bagimsiz olarak
//  olceklenir, boylece fare "kaymasi" titreşime yol acmaz.
//
//  ZEMIN CIZGISI: POV alaninin altindan 2/5 yukari = SPLIT_Y * 3/5
//  Sprite'in TABANI bu Y koordinatina sabitlenir.
// ============================================================
struct ItemTarget {
    // --- Static Hitbox (Hover/Click detection icin degismez) ---
    sf::FloatRect hitbox;

    // --- Visual Sprite (Sadece gorsel, olcek degisir ama hitbox etkilenmez) ---
    sf::Sprite sprite;

    // Hover durumu
    bool isHovered = false;

    // =================================================================
    //  BOUNDING BOX: Loot'un ekranda kaplamasi gereken maksimum alan.
    //  Texture ne kadar buyuk olursa olsun, bu alana sigirilir.
    //  450x330 gorseli: scale = 80/450 = ~0.178 -> ekranda 80x59 piksel
    // =================================================================
    static constexpr float TARGET_W         = 80.f;  // Hedef genislik (piksel)
    static constexpr float TARGET_H         = 60.f;  // Hedef yukseklik (piksel)
    static constexpr float HOVER_SCALE_FACTOR = 0.88f; // Hover'da gorunur kuculme orani

    // Zemin cizgisi: SPLIT_Y (380) * 3/5 = 228
    static constexpr float FLOOR_LINE_Y = 280.f;

    ItemTarget(float centerX, const sf::Texture& tex) : sprite(tex) {
        sf::Vector2u texSize = tex.getSize();
        float texW = static_cast<float>(texSize.x);
        float texH = static_cast<float>(texSize.y);

        // Texture'i bounding box'a sigdir (en boy orani korunur - "fit" mantigi)
        // Genislik ve yukseklik icin ayri scale hesapla, kucuk olani kullan
        float scaleByW = TARGET_W / texW;
        float scaleByH = TARGET_H / texH;
        float baseScale = std::min(scaleByW, scaleByH); // En-boy oranini koru

        // Sprite'i orijin = alt-orta noktaya sabitle (zemin cizgisine oturur)
        sprite.setOrigin({texW / 2.f, texH});
        sprite.setPosition({centerX, FLOOR_LINE_Y});
        sprite.setScale({baseScale, baseScale});

        // Statik hitbox: TARGET boyutundan hesaplanir (raw texture boyutundan degil)
        // Hover/scale degisikliklerinden ETKILENMEZ - flicker bu sayede olmaz
        float scaledW = texW * baseScale;
        float scaledH = texH * baseScale;
        hitbox = sf::FloatRect(
            {centerX - scaledW / 2.f, FLOOR_LINE_Y - scaledH},
            {scaledW, scaledH}
        );

        // baseScale'i ileride update()'de kullanabilmek icin sakla
        _baseScale = baseScale;
    }

    // Her frame cagirilir. Sadece gorsel sprite'i olcekler, hitbox dokunulmaz.
    void update(sf::Vector2f mousePos) {
        // Hover tespiti STATIK hitbox uzerinden yapilir (flicker yok)
        isHovered = hitbox.contains(mousePos);

        // Hover'da _baseScale * HOVER_SCALE_FACTOR, normalde sadece _baseScale
        float displayScale = isHovered ? (_baseScale * HOVER_SCALE_FACTOR) : _baseScale;
        sprite.setScale({displayScale, displayScale});
    }

    // Tiklama da STATIK hitbox uzerinden kontrol edilir
    bool isClicked(sf::Vector2f mousePos) const {
        return hitbox.contains(mousePos);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }

private:
    float _baseScale = 1.f; // Texture -> TARGET boyutuna indirgenmi hesaplanan scale
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
    std::optional<NPCTarget>  npc;
    std::vector<EnemyTarget>  enemies;

    // Loot texture referansi (Application'dan bir kez set edilir)
    const sf::Texture* lootTex = nullptr;

    void setLootTexture(const sf::Texture& tex) {
        lootTex = &tex;
    }

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
        if (r && r->itemID != -1 && !hasEnemies && lootTex) {
            float centerX = GAME_START_X + (LEFT_WIDTH / 2.f);
            groundItem.emplace(centerX, *lootTex);
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
