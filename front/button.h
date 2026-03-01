#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <optional>

struct Button {
    sf::Texture texture;
    std::optional<sf::Sprite> sprite;
    sf::FloatRect bounds; // Mouse hover/click için KESİN SINIRLAR (sizin istediğiniz özellik)
    sf::Vector2f baseScale;

    // x, y konumu ve o konumda ne kadarlık bir alan (width, height) kaplayacağı
    Button(const std::string& texturePath, float x, float y, float width, float height) {
        
        if (!texture.loadFromFile(texturePath)) {
            std::cerr << "Button texture yuklenemedi: " << texturePath << std::endl;
        }

        // Texture yüklendi, artık Sprite'a verebiliriz
        sprite.emplace(texture);

        // Resmin merkezini tam ortasına alıyoruz ki küçülme/büyüme ortadan merkeze doğru olsun
        sf::Vector2u texSize = texture.getSize();
        sprite->setOrigin({static_cast<float>(texSize.x) / 2.f, static_cast<float>(texSize.y) / 2.f});

        // Verilen (x,y) sol üst köşe olduğu için, Sprite'ı merkeze göre kaydırıyoruz
        sprite->setPosition({x + width / 2.f, y + height / 2.f});

        // Hedeflenen genişlik ve yüksekliğe göre Scale oranını hesaplama
        float scaleX = width / static_cast<float>(texSize.x);
        float scaleY = height / static_cast<float>(texSize.y);
        baseScale = {scaleX, scaleY};
        
        sprite->setScale(baseScale);

        // KESİN SINIRLAR: Animasyon sırasında değişmeyen sabit alanımız
        // Bu sayede fare üstündeyken buton küçülse bile, fare sınırın dışına çıkmış sayılmayacak.
        bounds = sf::FloatRect({x, y}, {width, height});
    }

    void update(sf::Vector2f mousePos, bool isMousePressed) {
        if (!sprite) return; // Henüz yüklenmediyse bir şey yapma

        // Tıklama kontrolünü, küçülen resmin sınırlarıyla DEĞİL, baştan belirlediğimiz sabit 'bounds' ile yapıyoruz!
        if (bounds.contains(mousePos)) {
            if (isMousePressed) {
                // Tıklanma Animasyonu (%85 boyut)
                sprite->setScale({baseScale.x * 0.85f, baseScale.y * 0.85f});
            } else {
                // Hover Animasyonu (%95 boyut)
                sprite->setScale({baseScale.x * 0.95f, baseScale.y * 0.95f});
            }
        } else {
            // Fare butonun üstünde değilse normal boyut (%100)
            sprite->setScale(baseScale);
        }
    }

    void draw(sf::RenderWindow& window) {
        if (sprite) {
            window.draw(*sprite);
        }
    }
};
