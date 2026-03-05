#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include "ui_elements.h"
#include "typewriter.h"

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

    // UI Elemanları (Texture'ları referans olarak yukarıdan alır)
    std::optional<GamePanel>   gamePanel;
    std::optional<DialogBox>   dialogBox;
    std::optional<StatBox>     statBox;
    std::optional<ButtonMenu>  buttonMenu;

    // Font - Typewriter için gerekli, merkezi depoda tutuluyor
    sf::Font font;
    Typewriter typewriter;

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

        // 2. UI Elemanlarını oluştur, resimleri referans olarak ver
        gamePanel.emplace();
        dialogBox.emplace(dialogTex);
        statBox.emplace(statTex);
        buttonMenu.emplace(buttonTex, mapTex, invTex);

        // 3. Font yükle ve test mesajı başlat
        if (!font.openFromFile("font.ttf")) return;
        typewriter.start("Kapiyi actiniz. Kuzey, dogu ve bati yonlerinde yollar var. Nigga Nigga Nigga Nigga Nigga Nigga", font);
    }

    void run() {
        while (window.isOpen()) {
            
            // Event Handling
            while (const std::optional eventOpt = window.pollEvent()) {
                const auto& event = *eventOpt;
                if (event.is<sf::Event::Closed>()) window.close();
                if (const auto* key = event.getIf<sf::Event::KeyPressed>())
                    if (key->code == sf::Keyboard::Key::Escape) window.close();
            }

            // Mouse
            sf::Vector2f worldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window), gameView);
            bool isMousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
            buttonMenu->update(worldPos, isMousePressed);
            typewriter.update();

            // Render
            window.clear(sf::Color::Black);
            window.setView(gameView);
            gamePanel->draw(window);
            dialogBox->draw(window);
            statBox->draw(window);
            buttonMenu->draw(window);
            // DialogBox konumunu draw'a geçiriyoruz ki yazı kutunun içine oturabilsin
            typewriter.draw(window, font, dialogBox->sprite.getPosition());
            window.display();
        }
    }
};
