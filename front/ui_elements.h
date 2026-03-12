#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "button.h"
#include "gamestate.h"
#include "../room.h"
#include "../player.h"
#include "../NPC.h"

const float LEFT_WIDTH = 560.f; 
const float RIGHT_WIDTH = 200.f; 
const float TOTAL_GAME_WIDTH = LEFT_WIDTH + RIGHT_WIDTH; 
const float OFFSET_X = (960.f - TOTAL_GAME_WIDTH) / 2.f; // 100.f
const float GAME_START_X = OFFSET_X;
const float SPLIT_Y = 380.f;

// Main Screen (Arka plan - texture yok, sadece renk)
struct GamePanel {
    sf::RectangleShape background;

    GamePanel() {
        background.setSize({LEFT_WIDTH, SPLIT_Y});
        background.setPosition({GAME_START_X, 0.f});
        background.setFillColor(sf::Color(0, 0, 0));
    }

    void draw(sf::RenderWindow& window) {
        window.draw(background);
    }
};

// DialogBox - Texture referans olarak geliyor
struct DialogBox {
    sf::Sprite sprite;

    DialogBox(const sf::Texture& tex) : sprite(tex) {
        sprite.setPosition({GAME_START_X, SPLIT_Y});
        float scaleX = LEFT_WIDTH / static_cast<float>(tex.getSize().x);
        float scaleY = (540.f - SPLIT_Y) / static_cast<float>(tex.getSize().y);
        sprite.setScale({scaleX, scaleY});
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }
};

// StatBox - Texture referans olarak geliyor
struct StatBox {
    sf::Sprite sprite;
    std::string statContent; // Güncel stat yazısı

    StatBox(const sf::Texture& tex) : sprite(tex) {
        sprite.setPosition({GAME_START_X + LEFT_WIDTH, 0.f});
        float scaleX = RIGHT_WIDTH / static_cast<float>(tex.getSize().x);
        float scaleY = SPLIT_Y / static_cast<float>(tex.getSize().y);
        sprite.setScale({scaleX, scaleY});

        // Başlangıçta placeholder değerlerle doldur
        statContent  = "   KEYBEARER\n";
        statContent += "     (Lvl --)\n\n";
        statContent += "HP:   --/--\n\n";
        statContent += "ATK:  --\n";
        statContent += "DEF:  --\n\n";
        statContent += "GOLD: --\n";
        statContent += "EXP:  --\n\n";
        statContent += "Weapon\n[--]\n\n";
        statContent += "Armor\n[--]";
    }

    // Backend gelince bu fonksiyon çağrılacak:
    // statBox->updateStats("1", "40", "40", "5", "3", "17", "0", "Fists", "None");
    void updateStats(const std::string& lvl, const std::string& hp, const std::string& maxHp,
                     const std::string& atk, const std::string& def,
                     const std::string& gold, const std::string& exp,
                     const std::string& weapon, const std::string& armor) {
        statContent  = "   KEYBEARER\n";
        statContent += "     (Lvl " + lvl + ")\n\n";
        statContent += "HP:   " + hp + "/" + maxHp + "\n\n";
        statContent += "ATK:  " + atk + "\n";
        statContent += "DEF:  " + def + "\n\n";
        statContent += "GOLD: " + gold + "\n";
        statContent += "EXP:  " + exp + "\n\n";
        statContent += "Weapon\n[" + weapon + "]\n\n";
        statContent += "Armor\n[" + armor + "]";
    }

    void draw(sf::RenderWindow& window, const sf::Font& font) {
        window.draw(sprite);
        sf::Text text(font);
        text.setCharacterSize(16);
        text.setFillColor(sf::Color(110, 0, 0)); // BORDEAUX
        text.setString(statContent);
        text.setPosition({sprite.getPosition().x + 35.f, sprite.getPosition().y + 40.f});
        window.draw(text);
    }

    // Player verilerini okuyup stat panelini günceller.
    // Application'da her frame çağrılır: statBox->syncWithPlayer(game.getPlayer());
    void syncWithPlayer(Player& p) {
        updateStats(
            std::to_string(p.getLvl()),
            std::to_string(p.getHp()),  std::to_string(p.getMaxHp()),
            std::to_string(p.getAtk()), std::to_string(p.getDef()),
            std::to_string(p.getGold()),std::to_string(p.getExp()),
            p.getWeaponName(),          p.getArmorName()
        );
    }
};

// ButtonMenu - Tüm Texture'lar dışarıdan referans olarak geliyor
struct ButtonMenu {
    std::vector<Button> buttons;

    ButtonMenu(const sf::Texture& btnTex, const sf::Texture& mapTex, const sf::Texture& invTex) {
        float colWidth    = RIGHT_WIDTH / 2.f;
        float areaHeight  = 540.f - SPLIT_Y;
        float yellowH     = areaHeight / 4.f;
        float actionStartX = GAME_START_X + LEFT_WIDTH;
        float actionStartY = SPLIT_Y;

        // 4 Aksiyon Butonu
        for (int i = 0; i < 4; i++) {
            buttons.emplace_back(btnTex, actionStartX, actionStartY + (i * yellowH), colWidth, yellowH);
        }

        // MAP ve INV Butonları
        float purpleH = areaHeight / 2.f;
        buttons.emplace_back(mapTex, actionStartX + colWidth, actionStartY, colWidth, purpleH);
        buttons.emplace_back(invTex, actionStartX + colWidth, actionStartY + purpleH, colWidth, purpleH);
    }

    void update(sf::Vector2f mousePos, bool isMousePressed) {
        for (auto& btn : buttons) btn.update(mousePos, isMousePressed);
    }

    void draw(sf::RenderWindow& window, const sf::Font& font) {
        for (auto& btn : buttons) btn.draw(window, font);
    }

    // State'e göre buton yazılarını ve renklerini günceller
    // Tüm texture referansları Application'ın merkezi deposundan geliyor
    void applyState(GameState state,
                    const sf::Texture& btnTex,    const sf::Texture& btnGreyTex,
                    const sf::Texture& mapTex,    const sf::Texture& mapGreyTex,
                    const sf::Texture& invTex,    const sf::Texture& invGreyTex) {

        if (state == GameState::EXPLORING) {
            buttons[0].setLabel("NORTH"); buttons[0].setTexture(btnTex);
            buttons[1].setLabel("WEST");  buttons[1].setTexture(btnTex);
            buttons[2].setLabel("EAST");  buttons[2].setTexture(btnTex);
            buttons[3].setLabel("SOUTH"); buttons[3].setTexture(btnTex);
            buttons[4].setTexture(mapTex);
            buttons[5].setTexture(invTex);
        }
        else if (state == GameState::COMBAT) {
            buttons[0].setLabel("ATTACK");   buttons[0].setTexture(btnTex);
            buttons[1].setLabel("FOCUS"); buttons[1].setTexture(btnTex);
            buttons[2].setLabel("MAGIC"); buttons[2].setTexture(btnTex);
            buttons[3].setLabel("RUN");   buttons[3].setTexture(btnTex);
            buttons[4].setTexture(mapGreyTex);
            buttons[5].setTexture(invTex);
        }
        else if (state == GameState::DIALOGUE) {
            for (int i = 0; i < 4; i++) {
                buttons[i].setLabel("");
                buttons[i].setTexture(btnGreyTex);
            }
            buttons[4].setTexture(mapGreyTex);
            buttons[5].setTexture(invGreyTex);
        }
        else if (state == GameState::SHOP) {
            buttons[0].setLabel("BUY");   buttons[0].setTexture(btnTex);
            buttons[1].setLabel("SELL");  buttons[1].setTexture(btnTex);
            buttons[2].setLabel("TALK");  buttons[2].setTexture(btnTex);
            buttons[3].setLabel("EXIT");  buttons[3].setTexture(btnTex);
            buttons[4].setTexture(mapGreyTex);
            buttons[5].setTexture(invGreyTex);
        }
    }

    // State + oda çıkışlarına göre butonları tek çağrıda günceller.
    // EXPLORING modunda gidilemez yönler otomatik grileştir.
    // Eger odada henuz konusulmamis bir NPC varsa, tum yonleri gri yap.
    // Application'da: buttonMenu->applyStateWithRoom(state, room, npc, textures...);
    void applyStateWithRoom(GameState state, Room* room, NPC* roomNPC,
                            const sf::Texture& btnTex,    const sf::Texture& btnGreyTex,
                            const sf::Texture& mapTex,    const sf::Texture& mapGreyTex,
                            const sf::Texture& invTex,    const sf::Texture& invGreyTex) {
        applyState(state, btnTex, btnGreyTex, mapTex, mapGreyTex, invTex, invGreyTex);

        // EXPLORING modunda gidilemez yönleri grileştir
        if (state == GameState::EXPLORING && room) {
            bool blockMovement = (roomNPC && !roomNPC->hasMet());

            if (blockMovement || room->n == -1) { buttons[0].setTexture(btnGreyTex); buttons[0].setLabel(""); }
            if (blockMovement || room->w == -1) { buttons[1].setTexture(btnGreyTex); buttons[1].setLabel(""); }
            if (blockMovement || room->e == -1) { buttons[2].setTexture(btnGreyTex); buttons[2].setLabel(""); }
            if (blockMovement || room->s == -1) { buttons[3].setTexture(btnGreyTex); buttons[3].setLabel(""); }
        }
    }
};

// ============================================================
//  DIALOGUE MENU - NPC Konusma Secenekleri
// ============================================================
struct DialogueMenu {
    struct OptionText {
        sf::Text text;
        int index;
        
        OptionText(const sf::Font& font, const std::string& str, float x, float y, int idx)
            : text(font), index(idx) 
        {
            text.setString("> " + str);
            text.setCharacterSize(18);
            text.setFillColor(sf::Color(200, 200, 200));
            text.setPosition({x, y});
        }
    };
    std::vector<OptionText> options;

    void loadOptions(const std::vector<std::string>& opts, const sf::Font& font, sf::Vector2f startPos) {
        options.clear();
        for (size_t i = 0; i < opts.size(); i++) {
            options.emplace_back(font, opts[i], startPos.x, startPos.y + (i * 20.f), (int)i);
        }
    }

    void updateHover(sf::Vector2f mousePos) {
        for (auto& opt : options) {
            if (opt.text.getGlobalBounds().contains(mousePos)) {
                opt.text.setFillColor(sf::Color::Yellow);
            } else {
                opt.text.setFillColor(sf::Color(200, 200, 200));
            }
        }
    }

    // Hangisine tiklandiysa indexini doner. Hicbiriyse -1 doner.
    int getClickedOption(sf::Vector2f mousePos) const {
        for (const auto& opt : options) {
            if (opt.text.getGlobalBounds().contains(mousePos)) {
                return opt.index;
            }
        }
        return -1;
    }

    void draw(sf::RenderWindow& window) {
        for (const auto& opt : options) {
            window.draw(opt.text);
        }
    }

    void clear() {
        options.clear();
    }
    
    bool isEmpty() const {
        return options.empty();
    }
};

// ============================================================
//  SHOP MENU - Dükkan Eşyaları (BUY Modu)
// ============================================================
struct ShopMenu {
    struct OptionText {
        sf::Text text;
        int index; // Shop listesindeki sırası

        OptionText(const sf::Font& font, const std::string& str, float x, float y, int idx)
            : text(font), index(idx) 
        {
            text.setString("> " + str);
            text.setCharacterSize(18);
            text.setFillColor(sf::Color(200, 200, 200));
            text.setPosition({x, y});
        }
    };
    std::vector<OptionText> options;
    bool isSellingMode;

    ShopMenu() : isSellingMode(false) {}

    void loadOptions(const std::vector<std::string>& opts, const sf::Font& font, sf::Vector2f startPos) {
        options.clear();
        for (size_t i = 0; i < opts.size(); i++) {
            options.emplace_back(font, opts[i], startPos.x, startPos.y + (i * 20.f), (int)i);
        }
    }

    void updateHover(sf::Vector2f mousePos) {
        if (isSellingMode) return; // Satis modundayken envanter kullanilir, bu liste islevsiz
        
        for (auto& opt : options) {
            if (opt.text.getGlobalBounds().contains(mousePos)) {
                opt.text.setFillColor(sf::Color::Yellow);
            } else {
                opt.text.setFillColor(sf::Color(200, 200, 200));
            }
        }
    }

    // Hangisine tiklandiysa indexini doner. Hicbiriyse -1 doner.
    int getClickedOption(sf::Vector2f mousePos) const {
        if (isSellingMode) return -1;

        for (const auto& opt : options) {
            if (opt.text.getGlobalBounds().contains(mousePos)) {
                return opt.index;
            }
        }
        return -1;
    }

    void draw(sf::RenderWindow& window) {
        if (isSellingMode) return; // Satis modundaysa secenekler cizilmez
        
        for (const auto& opt : options) {
            window.draw(opt.text);
        }
    }

    void clear() {
        options.clear();
    }
    
    bool isEmpty() const {
        return options.empty();
    }
};
