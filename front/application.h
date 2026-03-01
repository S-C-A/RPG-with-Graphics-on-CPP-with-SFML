#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include "ui_elements.h"

class Application {
private:
    sf::RenderWindow window;
    sf::View gameView;

    // Static UI Elements (Tertemiz, karmaşadan uzak)
    GamePanel gamePanel;
    DialogBox dialogBox;
    StatBox statBox;
    ButtonMenu buttonMenu; // 6 Butonumuzu barındıran kapsayıcı

public:
    Application() {
        // Fullscreen window
        window.create(sf::VideoMode::getDesktopMode(), "KEYBEARER", sf::State::Fullscreen);
        window.setFramerateLimit(60);

        // Textures set to be stretched from 960x540 base 
        gameView = sf::View(sf::FloatRect({0.f, 0.f}, {960.f, 540.f}));
        window.setView(gameView);
    }

    // (Game Loop)
    void run() {
        while (window.isOpen()) {
            
            // (Event Handling)
            while (const std::optional eventOpt = window.pollEvent()) {
                const auto& event = *eventOpt;

                // X button to close
                if (event.is<sf::Event::Closed>()) {
                    window.close();
                }
                
                // ESC button to close
                if (const auto* keyPress = event.getIf<sf::Event::KeyPressed>()) {
                    if (keyPress->code == sf::Keyboard::Key::Escape) {
                        window.close(); 
                    }
                }
            }

            // Update Mouse State
            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, gameView);
            bool isMousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

            // Sadece tek bir fonksiyon çağrısı ile tüm butonların fare kontrolünü yapıyoruz
            buttonMenu.update(worldPos, isMousePressed);

            // (Render) 
            window.clear(sf::Color::Black); // Clean the screen
            window.setView(gameView);       // Setting the coordinates for the textures

            // Static UI & Buttons
            gamePanel.draw(window);
            dialogBox.draw(window);
            statBox.draw(window);
            buttonMenu.draw(window);

            window.display();
        }
    }
};
