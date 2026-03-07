#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include "ui_elements.h"
#include "typewriter.h"
#include "gamestate.h"
#include "inventory.h"
#include "../game.h"

class Application {
private:
    sf::RenderWindow window;
    sf::View gameView;

    // === MERKEZI DEPO: Tüm Texture'lar burada, bir kez yüklenir ===
    sf::Texture dialogTex;
    sf::Texture statTex;
    sf::Texture buttonTex;
    sf::Texture mapTex;
    sf::Texture invTex; 
    sf::Texture buttonGreyTex;
    sf::Texture mapGreyTex;
    sf::Texture invGreyTex;

    // UI Elemanları (Texture'ları referans olarak yukarıdan alır)
    std::optional<GamePanel>   gamePanel;
    std::optional<DialogBox>   dialogBox;
    std::optional<StatBox>     statBox;
    std::optional<ButtonMenu>  buttonMenu;

    // Font - Typewriter için gerekli, merkezi depoda tutuluyor
    sf::Font font;
    Typewriter typewriter;

    // Oyunun mevcut state'i (TEMPORARİ: Y/U/I/O tuşlarıyla değiştirilir)
    GameState currentState = GameState::EXPLORING;

    // Envanter paneli
    InventoryPanel inventory;

    // === BACKEND ===
    Game game;

    // Stat panelini güncel Player değerleriyle doldurur
    void refreshStats() {
        Player& p = game.getPlayer();
        statBox->updateStats(
            std::to_string(p.getLvl()),
            std::to_string(p.getHp()),  std::to_string(p.getMaxHp()),
            std::to_string(p.getAtk()), std::to_string(p.getDef()),
            std::to_string(p.getGold()),std::to_string(p.getExp()),
            p.getWeaponName(),          p.getArmorName()
        );
    }

    // Butonları state + oda çıkışlarına göre günceller
    void refreshButtons() {
        buttonMenu->applyState(currentState, buttonTex, buttonGreyTex,
                               mapTex, mapGreyTex, invTex, invGreyTex);

        // EXPLORING modunda gidilemez yönleri grileştir
        if (currentState == GameState::EXPLORING) {
            Room* room = game.getCurrentRoom();
            if (room) {
                if (room->n == -1) { buttonMenu->buttons[0].setTexture(buttonGreyTex); buttonMenu->buttons[0].setLabel(""); }
                if (room->w == -1) { buttonMenu->buttons[1].setTexture(buttonGreyTex); buttonMenu->buttons[1].setLabel(""); }
                if (room->e == -1) { buttonMenu->buttons[2].setTexture(buttonGreyTex); buttonMenu->buttons[2].setLabel(""); }
                if (room->s == -1) { buttonMenu->buttons[3].setTexture(buttonGreyTex); buttonMenu->buttons[3].setLabel(""); }
            }
        }
    }

public:
    Application() {
        // Fullscreen window
        window.create(sf::VideoMode::getDesktopMode(), "KEYBEARER", sf::State::Fullscreen);
        window.setFramerateLimit(60);

        // 960x540 çözünürlük oranını tam ekrana sündür
        gameView = sf::View(sf::FloatRect({0.f, 0.f}, {960.f, 540.f}));
        window.setView(gameView);

        // 1. Resimleri MERKEZİ DEPODA yükle
        if (!dialogTex.loadFromFile("textures/Textbox[Final].png"))   return;
        if (!statTex.loadFromFile("textures/Statbox[Final].png"))     return;
        if (!buttonTex.loadFromFile("textures/Button[Final].png"))    return;
        if (!mapTex.loadFromFile("textures/Map[Final].png"))          return;
        if (!invTex.loadFromFile("textures/Inventory[Final].png"))    return;
        if (!buttonGreyTex.loadFromFile("textures/Button[Grey].png")) return;
        if (!mapGreyTex.loadFromFile("textures/Map[Gray].png"))       return;
        if (!invGreyTex.loadFromFile("textures/Inventory[Gray].png")) return;

        // 2. UI Elemanlarını oluştur, resimleri referans olarak ver
        gamePanel.emplace();
        dialogBox.emplace(dialogTex);
        statBox.emplace(statTex);
        buttonMenu.emplace(buttonTex, mapTex, invTex);

        // 3. Font yükle ve test mesajı başlat
        if (!font.openFromFile("font.ttf")) return;
        typewriter.start(game.lookAtRoom(), font);

        // 4. Başlangıç butonlarını oda çıkışlarına göre ayarla
        refreshButtons();

        // 5. Stat panelini gerçek değerlerle doldur
        refreshStats();
    }

    void run() {
        while (window.isOpen()) {
            
            // Event Handling
            bool isMouseJustClicked = false; // Her frame başında sıfırla
            while (const std::optional eventOpt = window.pollEvent()) {
                const auto& event = *eventOpt;
                if (event.is<sf::Event::Closed>()) window.close();
                if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
                    if (key->code == sf::Keyboard::Key::Escape) window.close();
                    // --- TEST: STATE DEĞİŞTİRME (İLERİDE SİLİNECEK) ---
                    if (key->code == sf::Keyboard::Key::Y) currentState = GameState::EXPLORING;
                    if (key->code == sf::Keyboard::Key::U) currentState = GameState::COMBAT;
                    if (key->code == sf::Keyboard::Key::I) currentState = GameState::DIALOGUE;
                    if (key->code == sf::Keyboard::Key::O) currentState = GameState::SHOP;
                    // State değişince butonları güncelle
                    refreshButtons();
                }
                // Fare tıklaması: sadece basıldığı ANda, tek sefer tetiklenir
                if (event.is<sf::Event::MouseButtonPressed>())
                    isMouseJustClicked = true;
            }

            // Mouse
            sf::Vector2f worldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window), gameView);
            bool isMousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
            buttonMenu->update(worldPos, isMousePressed);

            // INV butonu tıklama kontrolü (buton 5 = INV)
            if (isMouseJustClicked && buttonMenu->buttons[5].bounds.contains(worldPos)) {
                // Gri state'lerde açılamaz mesajı
                if (currentState == GameState::DIALOGUE || currentState == GameState::SHOP) {
                    typewriter.start("Can't open inventory right now.", font);
                } else {
                    bool opened = inventory.toggle(currentState);
                    if (opened)
                        typewriter.start("Inventory Opened.", font);
                    else
                        typewriter.start("Inventory Closed.", font);
                }
            }

            // Envanter hover güncelleme
            inventory.updateHover(worldPos);

            // Typewriter: güncelle, tıklanınca animasyonu atla
            typewriter.update();
            if (isMouseJustClicked && !buttonMenu->buttons[5].bounds.contains(worldPos))
                typewriter.skip();

            // Yön butonlarına tıklama (EXPLORING modunda, envanter kapalıyken)
            if (isMouseJustClicked && currentState == GameState::EXPLORING && !inventory.isOpen) {
                Room* room = game.getCurrentRoom();
                std::string moveMsg;

                if (buttonMenu->buttons[0].bounds.contains(worldPos) && room->n != -1)
                    moveMsg = game.attemptMove(room->n);
                else if (buttonMenu->buttons[1].bounds.contains(worldPos) && room->w != -1)
                    moveMsg = game.attemptMove(room->w);
                else if (buttonMenu->buttons[2].bounds.contains(worldPos) && room->e != -1)
                    moveMsg = game.attemptMove(room->e);
                else if (buttonMenu->buttons[3].bounds.contains(worldPos) && room->s != -1)
                    moveMsg = game.attemptMove(room->s);

                if (!moveMsg.empty()) {
                    typewriter.start(moveMsg, font);
                    refreshButtons(); // Yeni odanın çıkışlarına göre güncelle
                }
            }

            // Her frame statları güncel tut
            refreshStats();

            // Render
            window.clear(sf::Color::Black);
            window.setView(gameView);
            gamePanel->draw(window);
            // Envanter açıksa GamePanel'in üzerine çizilir
            inventory.draw(window, font);
            dialogBox->draw(window);
            statBox->draw(window, font);
            buttonMenu->draw(window, font);
            // DialogBox konumunu draw'a geçiriyoruz ki yazı kutunun içine oturabilsin
            typewriter.draw(window, font, dialogBox->sprite.getPosition());
            // --- TEST: Sol üstte mevcut state'i göster (İLERİDE SİLİNECEK) ---
            sf::Text debugText(font);
            debugText.setCharacterSize(14);
            debugText.setFillColor(sf::Color(200, 200, 0));
            debugText.setString(stateToString(currentState));
            debugText.setPosition({5.f, 5.f});
            window.draw(debugText);
            window.display();
        }
    }
};
