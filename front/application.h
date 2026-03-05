#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include "ui_elements.h"
#include "typewriter.h"
#include "gamestate.h"

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
        typewriter.start("Kapiyi actiniz. Kuzey, dogu ve bati yonlerinde yollar var.", font);

        // 4. Başlangıç state'ini uygula
        buttonMenu->applyState(currentState, buttonTex, buttonGreyTex, mapTex, mapGreyTex, invTex, invGreyTex);
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
                    buttonMenu->applyState(currentState, buttonTex, buttonGreyTex, mapTex, mapGreyTex, invTex, invGreyTex);
                }
                // Fare tıklaması: sadece basıldığı ANda, tek sefer tetiklenir
                if (event.is<sf::Event::MouseButtonPressed>())
                    isMouseJustClicked = true;
            }

            // Mouse
            sf::Vector2f worldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window), gameView);
            bool isMousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
            buttonMenu->update(worldPos, isMousePressed);

            // Typewriter: güncelle, tıklanınca animasyonu atla
            typewriter.update();
            if (isMouseJustClicked) typewriter.skip();

            // Render
            window.clear(sf::Color::Black);
            window.setView(gameView);
            gamePanel->draw(window);
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
