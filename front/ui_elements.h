#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "button.h"
#include "gamestate.h"

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
};
