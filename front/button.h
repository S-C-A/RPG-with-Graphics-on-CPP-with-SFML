#pragma once
#include <SFML/Graphics.hpp>

struct Button {
    sf::Sprite sprite;
    sf::FloatRect bounds; // Sabit sınır - hover/click jitter'ını önler
    sf::Vector2f baseScale;
    sf::Vector2f targetSize; // Hedef boyut (texture değişince scale yeniden hesaplanır)
    sf::Vector2f center;     // Butonun merkezi (label ortalama için)
    std::string labelText;   // Butonun üstündeki yazı

    // Texture dışarıdan geliyor (Application'dan referans)
    Button(const sf::Texture& tex, float x, float y, float width, float height)
        : sprite(tex), targetSize({width, height}), center({x + width / 2.f, y + height / 2.f})
    {
        sf::Vector2u texSize = tex.getSize();

        // Merkez origin: küçülme/büyüme tam ortadan olsun
        sprite.setOrigin({static_cast<float>(texSize.x) / 2.f, static_cast<float>(texSize.y) / 2.f});
        sprite.setPosition(center);

        float scaleX = width / static_cast<float>(texSize.x);
        float scaleY = height / static_cast<float>(texSize.y);
        baseScale = {scaleX, scaleY};
        sprite.setScale(baseScale);

        // Sabit sınır: resim küçülse bile bu alan değişmez
        bounds = sf::FloatRect({x, y}, {width, height});
    }

    // Texture'ı değiştir (renkli ↔ gri geçişi için)
    // Yeni texture'ın boyutuna göre scale yeniden hesaplanır
    void setTexture(const sf::Texture& tex) {
        sprite.setTexture(tex);
        sf::Vector2u texSize = tex.getSize();
        sprite.setOrigin({static_cast<float>(texSize.x) / 2.f, static_cast<float>(texSize.y) / 2.f});
        baseScale = {targetSize.x / static_cast<float>(texSize.x),
                     targetSize.y / static_cast<float>(texSize.y)};
        sprite.setScale(baseScale);
    }

    // Yazıyı değiştir (state geçişlerinde çağrılır)
    void setLabel(const std::string& text) {
        labelText = text;
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

    // Font dışarıdan geçiliyor, Button içinde SAKLANMIYOR (segfault önlemi)
    void draw(sf::RenderWindow& window, const sf::Font& font) {
        window.draw(sprite);
        if (!labelText.empty()) {
            sf::Text label(font);
            label.setCharacterSize(14);
            label.setFillColor(sf::Color(110, 0, 0)); // BORDEAUX
            label.setString(labelText);
            // Yazıyı butonun tam ortasına oturt
            sf::FloatRect tb = label.getLocalBounds();
            label.setOrigin({tb.position.x + tb.size.x / 2.f, tb.position.y + tb.size.y / 2.f});
            label.setPosition(sprite.getPosition());
            window.draw(label);
        }
    }
};
