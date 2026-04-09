#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>      // std::min icin
#include "../game.h"
#include "ui_elements.h" // LEFT_WIDTH, SPLIT_Y, vs icin

// ============================================================
//  DUSMAN TEXTURE SETI
// ============================================================
//  Her dusman turune ait iki gorsel saklanir:
//    idle   -> Normal bekleme gorseli
//    attack -> Saldiri gorseli
//  Test asamasinda her ikisi de ayni PNG olabilir.
//  Ileride farkli animasyon PNG'leri atanir.
// ============================================================
struct EnemyTextureSet {
    sf::Texture idle;    // Bekleme animasyonu gorseli
    sf::Texture attack;  // Saldiri animasyonu gorseli
};

// ============================================================
//  ENEMY TARGET - Savas Ekranindaki Dusman Gorseli
// ============================================================
//  Eski kirmizi dikdortgen (RectangleShape) tamamen kaldirildi.
//  Artik gercek PNG texture'lari kullaniyor.
//
//  PIKSEL OLCEGI (PIXEL_SCALE = 0.5):
//    Teksturler 1120x760 tuvalinde tasarlanmistir (2x buyuk).
//    Oyunun mantiksal alani 560x380 oldugundan, tum boyutlar
//    0.5 katsayisiyla yari boyuta indirilir.
//    Oranlar bozulmaz: buyuk canavar kucukten buyuk kalir.
//
//  ZEMIN CILASI (Bottom-Center Pivot):
//    Her sprite'in origin noktasi alt-ortaya alinir.
//    Pozisyon her zaman FLOOR_LINE_Y'ye sabitlenir.
//    Sonuc: Hangi boyutta gorsel gelirse gelsin, ayaklari
//    zemin cizgisine tam oturur.
//
//  FLICKER ONLEME (Static Hitbox):
//    Hover animasyonu sadece sprite'in scale degerini degistirir.
//    Tiklama/hover algilama degismeyen statik hitbox uzerinden
//    yapilir. Fare kenarinda titreme olmaz.
// ============================================================
struct EnemyTarget {

    // Statik Hitbox - tiklama/hover icin, hic degismez
    sf::FloatRect hitbox;

    // Gorsel Sprite'lar
    sf::Sprite spriteIdle;    // Normal bekleme gorseli
    sf::Sprite spriteAttack;  // Saldiri gorseli

    // false = idle, true = saldiri animasyonu goster
    bool isAttacking = false;

    // Fare bu dusmanin uzerinde mi?
    bool isHovered = false;

    // Bu dusmanin r->monsterID[] dizisindeki indeksi
    int index;

    // -------------------------------------------------------
    //  HIT FLASH ANIMASYONU
    //  Oyuncu bu dusmanin vurdugunda (olmediyse) iki kez
    //  gorsel yanar-sonar: gorunur/gorunmez/gorunur/gorunmez.
    //  isVisible = false oldugunda draw() hicbir sey cizmez.
    // -------------------------------------------------------
    bool       isFlashing = false;  // Su an flash animasyonu gidiyor mu?
    bool       isVisible  = true;   // false iken sprite cizilmez
    sf::Clock  flashClock;          // Flash suresi icin kronomeye

    // Her yari-cycle (gorunur veya gorunmez) kac saniye?
    static constexpr float FLASH_INTERVAL = 0.10f;  // 100ms
    // Toplam flash suresi: 4 yari-cycle = 2 tam flash = 400ms
    static constexpr float FLASH_DURATION = 0.40f;

    // Gorseller 1120x760 tuvalinde tasarlandigi icin 0.5 ile oyun alanina indiriliyor
    static constexpr float PIXEL_SCALE        = 0.5f;
    static constexpr float HOVER_SCALE_FACTOR = 0.97f; // Hover'da %3 kuculme
    static constexpr float FLOOR_LINE_Y       = 280.f; // Ayaklarin bastigi Y cizgisi

    // -----------------------------------------------------------------
    //  Constructor
    //  centerX : Dusmanin yatay merkez koordinati
    //  _index  : r->monsterID[] dizisindeki sirasi (CombatGUI sync icin)
    //  texSet  : idle + attack texture cifti
    // -----------------------------------------------------------------
    EnemyTarget(float centerX, int _index, const EnemyTextureSet& texSet)
        : spriteIdle(texSet.idle), spriteAttack(texSet.attack), index(_index)
    {
        float idleW = static_cast<float>(texSet.idle.getSize().x);
        float idleH = static_cast<float>(texSet.idle.getSize().y);
        float atkW  = static_cast<float>(texSet.attack.getSize().x);
        float atkH  = static_cast<float>(texSet.attack.getSize().y);

        // Idle Sprite: origin = alt-orta, pozisyon = zemin cizgisi
        // setOrigin(width/2, height) ile sprite'in tabani pozisyona denk duser
        spriteIdle.setOrigin({idleW / 2.f, idleH});
        spriteIdle.setPosition({centerX, FLOOR_LINE_Y});
        spriteIdle.setScale({PIXEL_SCALE, PIXEL_SCALE});

        // Attack Sprite: ayni pivot mantigi
        spriteAttack.setOrigin({atkW / 2.f, atkH});
        spriteAttack.setPosition({centerX, FLOOR_LINE_Y});
        spriteAttack.setScale({PIXEL_SCALE, PIXEL_SCALE});

        // Statik Hitbox: idle texture'un olcekli boyutlarindan
        // BIR KEZ hesaplanir, hic degistirilmez (flicker onleme)
        float scaledW = idleW * PIXEL_SCALE;
        float scaledH = idleH * PIXEL_SCALE;
        hitbox = sf::FloatRect(
            {centerX - scaledW / 2.f, FLOOR_LINE_Y - scaledH},
            {scaledW, scaledH}
        );
    }

    // Animasyon durumunu degistirir (CombatGUI'dan cagrilabilir)
    void setAttacking(bool attacking) {
        isAttacking = attacking;
    }

    // -------------------------------------------------------
    //  startFlash
    //  Oyuncu bu dusmana vurdugunda (olmediyse) cagirilir.
    //  2 tam gorunur/gorunmez dongusu baslatir.
    // -------------------------------------------------------
    void startFlash() {
        isFlashing = true;
        isVisible  = true;
        flashClock.restart();
    }

    // Her frame cagirilir. Sadece gorsel olceklenir, hitbox dokunulmaz.
    void update(sf::Vector2f mousePos) {
        // --- FLASH ANIMASYONU GUNCELLE ---
        if (isFlashing) {
            float elapsed = flashClock.getElapsedTime().asSeconds();
            if (elapsed >= FLASH_DURATION) {
                // Animasyon bitti: gorsel kalici gorunur yap
                isFlashing = false;
                isVisible  = true;
            } else {
                // Her FLASH_INTERVAL'da gorsel acar/kapar
                // elapsed / FLASH_INTERVAL = o anki yari-cycle indeksi
                int cycleIdx = static_cast<int>(elapsed / FLASH_INTERVAL);
                isVisible = (cycleIdx % 2 == 0); // cift=gorunur, tek=gorunmez
            }
        }

        // Hover tespiti STATIK hitbox uzerinden
        isHovered = hitbox.contains(mousePos);
        float displayScale = isHovered ? (PIXEL_SCALE * HOVER_SCALE_FACTOR) : PIXEL_SCALE;
        spriteIdle.setScale({displayScale, displayScale});
        spriteAttack.setScale({displayScale, displayScale});
    }

    // Tiklama STATIK hitbox uzerinden kontrol edilir
    bool isClicked(sf::Vector2f mousePos) const {
        return hitbox.contains(mousePos);
    }

    // isAttacking'e gore dogru sprite'i cizer
    // isVisible = false ise hicbir sey cizilmez (hit-flash)
    void draw(sf::RenderWindow& window) {
        if (!isVisible) return; // Flash animasyonunun "gorunmez" karesi
        if (isAttacking) {
            window.draw(spriteAttack);
        } else {
            window.draw(spriteIdle);
        }
    }
};

// ============================================================
//  ITEM TARGET - Yerdeki Esyanin Gorseli (Loot[Final] Texture)
// ============================================================
//  FLICKER ONLEME: Hover/scale hesaplari icin STATIK bir hitbox
//  kullanilir. Gorsel sprite bu hitbox'tan bagimsiz olarak
//  olceklenir, boylece fare "kaymasi" titresime yol acmaz.
//
//  ZEMIN CIZGISI: Y=280
//  Sprite'in TABANI bu Y koordinatina sabitlenir.
// ============================================================
struct ItemTarget {
    // Static Hitbox (Hover/Click detection icin degismez)
    sf::FloatRect hitbox;

    // Visual Sprite (Sadece gorsel, olcek degisir ama hitbox etkilenmez)
    sf::Sprite sprite;

    bool isHovered = false;

    // Bounding box: Loot'un ekranda maksimum kaplayacagi alan
    static constexpr float TARGET_W          = 80.f;
    static constexpr float TARGET_H          = 60.f;
    static constexpr float HOVER_SCALE_FACTOR = 0.88f;
    static constexpr float FLOOR_LINE_Y      = 280.f;

    ItemTarget(float centerX, const sf::Texture& tex) : sprite(tex) {
        float texW = static_cast<float>(tex.getSize().x);
        float texH = static_cast<float>(tex.getSize().y);

        // En-boy orani koruyarak TARGET alana sigdir
        float scaleByW = TARGET_W / texW;
        float scaleByH = TARGET_H / texH;
        float baseScale = std::min(scaleByW, scaleByH);

        // Alt-orta pivot, zemin cizgisine oturt
        sprite.setOrigin({texW / 2.f, texH});
        sprite.setPosition({centerX, FLOOR_LINE_Y});
        sprite.setScale({baseScale, baseScale});

        // Statik hitbox (bir kez hesaplanir, degismez)
        float scaledW = texW * baseScale;
        float scaledH = texH * baseScale;
        hitbox = sf::FloatRect(
            {centerX - scaledW / 2.f, FLOOR_LINE_Y - scaledH},
            {scaledW, scaledH}
        );

        _baseScale = baseScale;
    }

    void update(sf::Vector2f mousePos) {
        isHovered = hitbox.contains(mousePos);
        float displayScale = isHovered ? (_baseScale * HOVER_SCALE_FACTOR) : _baseScale;
        sprite.setScale({displayScale, displayScale});
    }

    bool isClicked(sf::Vector2f mousePos) const {
        return hitbox.contains(mousePos);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }

private:
    float _baseScale = 1.f;
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
//  WORLD OBJECTS - Odadaki Tum Nesnelerin Yoneticisi
// ============================================================
//  application.h'ta sadece bu struct instantiate edilir.
//  Oda degistikce syncWithRoom cagirilir.
struct WorldObjects {
    std::optional<ItemTarget> groundItem;
    std::optional<NPCTarget>  npc;
    std::vector<EnemyTarget>  enemies;

    // Loot texture referansi (Application'dan bir kez set edilir)
    const sf::Texture* lootTex = nullptr;

    // Dusman texture haritasi:
    //   Anahtar = Monster adi ("Bandit Slasher")
    //   Deger   = idle + attack texture cifti
    // Application'dan bir kez set edilir, syncWithRoom okur.
    std::unordered_map<std::string, EnemyTextureSet>* enemyTexMapPtr = nullptr;

    void setLootTexture(const sf::Texture& tex) {
        lootTex = &tex;
    }

    // Application constructor'indan cagirilir.
    // syncWithRoom'dan ONCE cagrilmali (sync lookup yapar).
    void setEnemyTexMap(std::unordered_map<std::string, EnemyTextureSet>& map) {
        enemyTexMapPtr = &map;
    }

    // Odaya girildiginde veya dusman oldugunde cagirilir.
    // Tum gorsel nesneleri yeni oda durumuna gore sifirlar.
    void syncWithRoom(Game& game) {
        Room* r = game.getCurrentRoom();

        // --- DUSMAN GORSELLERINI YENIDEN OLUSTUR ---
        enemies.clear();
        if (r && !r->monsterID.empty()) {

            // Kac canli dusman var? (yatay ortalama icin)
            int totalAlive = 0;
            for (int id : r->monsterID) {
                if (id != -1) totalAlive++;
            }

            float centerX = GAME_START_X + (LEFT_WIDTH / 2.f);
            float spacing = 180.f; // Iki dusman arasi yatay mesafe
            float startX  = centerX - ((totalAlive - 1) * spacing / 2.f);
            int   placed  = 0;    // Kacinci canli dusmanin yerlestirildigini sayar

            for (size_t i = 0; i < r->monsterID.size(); i++) {
                if (r->monsterID[i] == -1) continue; // Olmus dusman, atla

                float enemyCenterX = startX + (placed * spacing);

                // Monster adini backend'den al (texture lookup icin)
                std::string monsterName = "";
                if (enemyTexMapPtr) {
                    Monster* temp = game.getMonsterClone(r->monsterID[i]);
                    if (temp) {
                        monsterName = temp->getName();
                        delete temp; // Bellek sizintisi olmamali
                    }
                }

                // Haritada bu isimde texture var mi?
                const EnemyTextureSet* texSet = nullptr;
                if (enemyTexMapPtr && !monsterName.empty()) {
                    auto it = enemyTexMapPtr->find(monsterName);
                    if (it != enemyTexMapPtr->end()) {
                        texSet = &it->second;
                    }
                }

                // Texture bulunduysa gorsel olustur.
                // index olarak `i` kullaniliyor (CombatGUI senkronizasyonu icin kritik).
                if (texSet) {
                    enemies.emplace_back(enemyCenterX, (int)i, *texSet);
                }

                placed++;
            }
        }

        bool hasEnemies = !enemies.empty();

        // Dusman varsa esya ve NPC gosterme (tiklama karismasi onlenir)
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

    // -------------------------------------------------------
    //  startEnemyFlash
    //  Oyuncu bir dusmana vurdugunda (ve dusman olmediyse)
    //  application.h'tan cagirilir. Gorsel yanip-sonme baslar.
    //  idx: r->monsterID[] dizisindeki gercek indeks
    // -------------------------------------------------------
    void startEnemyFlash(int idx) {
        for (auto& e : enemies) {
            if (e.index == idx) {
                e.startFlash();
                break;
            }
        }
    }

    // -------------------------------------------------------
    //  setEnemyAttacking
    //  Belirli bir dusmanin animasyon durumunu degistirir.
    //  CombatGUI'daki updateTurn tarafindan cagirilir:
    //    - Dusman hamle yapinca: setEnemyAttacking(idx, true)
    //    - Tur bittikten sonra:  resetAllAttacking()
    //
    //  idx: r->monsterID[] dizisindeki gercek indeks
    //       (EnemyTarget.index ile eslesilir)
    // -------------------------------------------------------
    void setEnemyAttacking(int idx, bool attacking) {
        for (auto& e : enemies) {
            if (e.index == idx) {
                e.setAttacking(attacking);
                break;
            }
        }
    }

    // -------------------------------------------------------
    //  resetAllAttacking
    //  Tum dusmanlar icin attack animasyonunu kapatir.
    //  Tur bittikten sonra hepsi idle'a doner.
    // -------------------------------------------------------
    void resetAllAttacking() {
        for (auto& e : enemies) {
            e.setAttacking(false);
        }
    }

    // Her frame hover kontrolu
    void update(sf::Vector2f mousePos) {
        for (auto& enemy : enemies) {
            enemy.update(mousePos);
        }
        if (groundItem) groundItem->update(mousePos);
        if (npc) npc->update(mousePos);
    }

    // Sol tik: esyaya tiklanirsa pickup yapar, mesaj doner
    std::string handleLeftClick(Game& game, sf::Vector2f mousePos) {
        if (groundItem && groundItem->isClicked(mousePos)) {
            std::string msg = game.tryPickupItem();
            syncWithRoom(game);
            return msg;
        }
        return "";
    }

    // Sol tik: NPC'ye tiklanirsa pointer doner
    NPC* handleLeftClickNPC(Game& game, sf::Vector2f mousePos) {
        if (npc && npc->isClicked(mousePos)) {
            return game.getRoomNPC();
        }
        return nullptr;
    }

    // Sol tik: dusmana tiklanirsa indeksi doner, degilse -1
    int handleLeftClickEnemy(sf::Vector2f mousePos) const {
        for (const auto& enemy : enemies) {
            if (enemy.isClicked(mousePos)) {
                return enemy.index;
            }
        }
        return -1;
    }

    // Cizim
    void draw(sf::RenderWindow& window) {
        for (auto& enemy : enemies) {
            enemy.draw(window);
        }
        if (groundItem) groundItem->draw(window);
        if (npc) npc->draw(window);
    }
};
