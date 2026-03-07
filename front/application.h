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
        buttonMenu->applyStateWithRoom(currentState, game.getCurrentRoom(),
            buttonTex, buttonGreyTex, mapTex, mapGreyTex, invTex, invGreyTex);

        // 5. Stat panelini gerçek değerlerle doldur
        statBox->syncWithPlayer(game.getPlayer());
    }

    void run() {
        while (window.isOpen()) {
            
            // Event Handling
            bool isLeftClick = false;
            bool isRightClick = false;
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
                    buttonMenu->applyStateWithRoom(currentState, game.getCurrentRoom(),
                        buttonTex, buttonGreyTex, mapTex, mapGreyTex, invTex, invGreyTex);
                }
                // Fare tıklaması: hangi buton olduğunu da sakla
                if (const auto* mousePress = event.getIf<sf::Event::MouseButtonPressed>()) {
                    if (mousePress->button == sf::Mouse::Button::Left)
                        isLeftClick = true;
                    else if (mousePress->button == sf::Mouse::Button::Right)
                        isRightClick = true;
                }
            }

            // Mouse
            sf::Vector2f worldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window), gameView);
            bool isMousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
            buttonMenu->update(worldPos, isMousePressed);

            bool isMouseJustClicked = isLeftClick || isRightClick;

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
            if (isLeftClick && !buttonMenu->buttons[5].bounds.contains(worldPos))
                typewriter.skip();

            // --- ENVANTER EŞYA ETKİLEŞİMİ ---
            std::string itemResult = inventory.handleClick(game, isLeftClick, isRightClick);
            if (!itemResult.empty())
                typewriter.start(itemResult, font);

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
                    // Yeni odanın çıkışlarına göre butonları güncelle
                    buttonMenu->applyStateWithRoom(currentState, game.getCurrentRoom(),
                        buttonTex, buttonGreyTex, mapTex, mapGreyTex, invTex, invGreyTex);
                }
            }

            // Her frame statları güncel tut
            statBox->syncWithPlayer(game.getPlayer());

            // Envanter slotlarını backend'den güncelle
            inventory.syncSlots(game);

            // Render
            window.clear(sf::Color::Black);
            window.setView(gameView);
            gamePanel->draw(window);
            // Envanter açıksa GamePanel'in üzerine çizilir
            inventory.draw(window, font);
            dialogBox->draw(window);
            statBox->draw(window, font);
            buttonMenu->draw(window, font);
            // Diyalog kutusu: hover varsa eşya açıklaması, yoksa typewriter
            if (!inventory.drawHoverDesc(window, font, dialogBox->sprite.getPosition(), game))
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
