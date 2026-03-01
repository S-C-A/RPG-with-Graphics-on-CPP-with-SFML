#pragma once
#include <SFML/Graphics.hpp>

struct Button {
    sf::Sprite sprite;
    sf::FloatRect bounds; // Sabit sınır - hover/click jitter'ını önler
    sf::Vector2f baseScale;

    // Texture dışarıdan geliyor (Application'dan referans)
    Button(const sf::Texture& tex, float x, float y, float width, float height)
        : sprite(tex)
    {
        sf::Vector2u texSize = tex.getSize();

        // Merkez origin: küçülme/büyüme tam ortadan olsun
        sprite.setOrigin({static_cast<float>(texSize.x) / 2.f, static_cast<float>(texSize.y) / 2.f});
        sprite.setPosition({x + width / 2.f, y + height / 2.f});

        float scaleX = width / static_cast<float>(texSize.x);
        float scaleY = height / static_cast<float>(texSize.y);
        baseScale = {scaleX, scaleY};
        sprite.setScale(baseScale);

        // Sabit sınır: resim küçülse bile bu alan değişmez
        bounds = sf::FloatRect({x, y}, {width, height});
    }

    void update(sf::Vector2f mousePos, bool isMousePressed) {
        if (bounds.contains(mousePos)) {
            if (isMousePressed)
                sprite.setScale({baseScale.x * 0.85f, baseScale.y * 0.85f});
            else
                sprite.setScale({baseScale.x * 0.95f, baseScale.y * 0.95f});
        } else {
            sprite.setScale(baseScale);
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }
};
