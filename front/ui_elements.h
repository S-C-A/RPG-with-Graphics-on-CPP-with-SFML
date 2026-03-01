#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "button.h"

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

    StatBox(const sf::Texture& tex) : sprite(tex) {
        sprite.setPosition({GAME_START_X + LEFT_WIDTH, 0.f});
        float scaleX = RIGHT_WIDTH / static_cast<float>(tex.getSize().x);
        float scaleY = SPLIT_Y / static_cast<float>(tex.getSize().y);
        sprite.setScale({scaleX, scaleY});
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
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

    void draw(sf::RenderWindow& window) {
        for (auto& btn : buttons) btn.draw(window);
    }
};
