#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <optional>
#include "button.h"


const float LEFT_WIDTH = 560.f; 
const float RIGHT_WIDTH = 200.f; 
const float TOTAL_GAME_WIDTH = LEFT_WIDTH + RIGHT_WIDTH; 
const float OFFSET_X = (960.f - TOTAL_GAME_WIDTH) / 2.f; // 100.f
const float GAME_START_X = OFFSET_X;
const float SPLIT_Y = 380.f;

// Main Screen
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

// DialogBox
struct DialogBox {
    sf::Texture texture;
    std::optional<sf::Sprite> sprite;

    DialogBox() {
        if (!texture.loadFromFile("textures/Textbox[Final].png")) {
            std::cerr << "Textbox texture yuklenemedi!" << std::endl;
        }
        
        // Loading protection
        sprite.emplace(texture);
        sprite->setPosition({GAME_START_X, SPLIT_Y});

        float scaleX = LEFT_WIDTH / static_cast<float>(texture.getSize().x);
        float scaleY = (540.f - SPLIT_Y) / static_cast<float>(texture.getSize().y);
        sprite->setScale({scaleX, scaleY});
    }

    void draw(sf::RenderWindow& window) {
        if (sprite) {
            window.draw(*sprite);
        }
    }
};

// StatBox
struct StatBox {
    sf::Texture texture;
    std::optional<sf::Sprite> sprite;

    StatBox() {
        if (!texture.loadFromFile("textures/Statbox[Final].png")) {
            std::cerr << "Statbox texture yuklenemedi!" << std::endl;
        }
        
        // Loading protection
        sprite.emplace(texture);
        sprite->setPosition({GAME_START_X + LEFT_WIDTH, 0.f});

        float scaleX = RIGHT_WIDTH / static_cast<float>(texture.getSize().x);
        float scaleY = SPLIT_Y / static_cast<float>(texture.getSize().y);
        sprite->setScale({scaleX, scaleY});
    }

    void draw(sf::RenderWindow& window) {
        if (sprite) {
            window.draw(*sprite);
        }
    }
};

// --- BUTON MENÜSÜ KAPSAYICISI ---
struct ButtonMenu {
    std::vector<Button> buttons;

    ButtonMenu() {
        // SFML BEYAZ KARE HATASI ÇÖZÜMÜ:
        // Vektörün kapasitesini baştan 10 olarak ayırıyoruz ki eleman eklendikçe yeniden boyutlanıp
        // hafızadaki Texture'ların yerini değiştirmesin. Yerleri değişirse Sprite'lar boşluğa bakar.
        buttons.reserve(10);

        // --- BUTTON LAYOUT (from graphic_test.cpp) ---
        float colWidth = RIGHT_WIDTH / 2.f;       // 100.f
        float areaHeight = 540.f - SPLIT_Y;       // 160.f
        float yellowH = areaHeight / 4.f;         // 40.f
        float actionStartX = GAME_START_X + LEFT_WIDTH; // 660.f
        float actionStartY = SPLIT_Y;             // 380.f

        // First 4 Action Buttons
        for (int i = 0; i < 4; i++) {
            buttons.emplace_back(
                "textures/Button[Final].png",
                actionStartX, 
                actionStartY + (i * yellowH),
                colWidth, 
                yellowH
            );
        }

        // MAP and INV Buttons
        float purpleH = areaHeight / 2.f;         // 80.f
        buttons.emplace_back("textures/Map[Final].png", actionStartX + colWidth, actionStartY, colWidth, purpleH);
        buttons.emplace_back("textures/Inventory[Final].png", actionStartX + colWidth, actionStartY + purpleH, colWidth, purpleH);
    }

    void update(sf::Vector2f mousePos, bool isMousePressed) {
        for (auto& btn : buttons) {
            btn.update(mousePos, isMousePressed);
        }
    }

    void draw(sf::RenderWindow& window) {
        for (auto& btn : buttons) {
            btn.draw(window);
        }
    }
};
