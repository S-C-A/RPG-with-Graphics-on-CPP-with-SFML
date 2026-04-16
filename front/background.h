#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include "ui_elements.h" // LEFT_WIDTH, TOTAL_GAME_WIDTH, SPLIT_Y, GAME_START_X vs icin

// ============================================================
//  ATMOSPHERIC ELEMENT - Agac gibi arka plan nesneleri
// ============================================================
struct AtmosphericElement {
    sf::Sprite sprite;
    
    // Gorseller genel sistemle uyumlu olmasi icin kucultuluyor
    static constexpr float PIXEL_SCALE = 0.5f;

    AtmosphericElement(float x, float y, const sf::Texture& tex) : sprite(tex) {
        float h = static_cast<float>(tex.getSize().y);
        float w = static_cast<float>(tex.getSize().x);
        
        // Zemin pivot (ayaklar yerdedir)
        sprite.setOrigin({w / 2.f, h});
        sprite.setPosition({x, y});
        sprite.setScale({PIXEL_SCALE, PIXEL_SCALE});
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }
};

// ============================================================
//  BACKGROUND MANAGER
// ============================================================
class BackgroundManager {
private:
    std::vector<AtmosphericElement> elements;
    
    struct TreePair {
        const sf::Texture* left;
        const sf::Texture* right;
    };
    std::vector<TreePair> treeVarieties;

    // Tam rastgelelik motoru
    std::mt19937 rng;

public:
    BackgroundManager() {
        std::random_device rd;
        rng.seed(rd());
    }

    // Farkli agac turlerini (varyasyonlari) listeye ekler
    void addTreeVariety(const sf::Texture& left, const sf::Texture& right) {
        treeVarieties.push_back({&left, &right});
    }

    void syncWithRoom(int roomId) {
        elements.clear();

        if (roomId < 500) {
            randomize();
        } else {
            // Ileride burada tam ekran sabit bir gorsel (loadFixed) cagirilir
        }
    }

    // 5 yatay serit uzerine (perspektif yaratacak sekilde) rastgele agac dizer
    void randomize() {
        if (treeVarieties.empty()) return;
        
        // Agac turunu secmek icin rastgelelik
        std::uniform_int_distribution<int> typeDist(0, treeVarieties.size() - 1);

        // Oyun alani yatay merkezi
        float centerX = GAME_START_X + (LEFT_WIDTH / 2.f);
        
        // Dikey eksende agaclarin basip durabilecegi sinirlar (SPLIT_Y=380 altina gecmemeli)
        float startY = 180.f;  // En ust agacin tabani (Uzaklik)
        float endY   = 360.f;  // En alt agacin tabani (Yakinlik)
        float stepY  = (endY - startY) / 4.f; // 5 serit icin 4 bosluk birakiyoruz

        // Perspektif V-sekli aralik (Yukarisi dar, asagisi genis bir yol gibi)
        float topGapHalf = 120.f; // Uzak/Ust seritte agaclar merkeze 120px'den fazla yaklasamaz
        float botGapHalf = 200.f; // Yakin/Alt seritte agaclar merkeze anca 200px kadar yaklasabilir

        for (int i = 0; i < 5; ++i) {
            float t = static_cast<float>(i) / 4.f; // 0.0'dan 1.0'a oran
            float yPos = startY + (i * stepY);
            
            // O anki serit icin bos birakilacak merkez yolun yarisi hesaplaniyor
            float currentGapHalf = topGapHalf + t * (botGapHalf - topGapHalf);

            // AGAC YERLESIM SINIRLARI
            // Sol agaclar ekranin solundan (GAME_START_X) baslayip, yola kadar gelebilir.
            float leftMinX = GAME_START_X + 20.f;
            float leftMaxX = centerX - currentGapHalf;

            // Sag agaclar yolun bittigi yerden baslayip, ekranin sagina kadar gidebilir.
            float rightMinX = centerX + currentGapHalf;
            float rightMaxX = GAME_START_X + LEFT_WIDTH - 20.f;

            // Cok nadir ihtimallere karsi sinir korumasi
            if (leftMinX > leftMaxX) leftMinX = leftMaxX;
            if (rightMaxX < rightMinX) rightMaxX = rightMinX;

            // Sola rastgele bir X sec ve rastgele bir agac turu yarat
            int leftTypeIdx = typeDist(rng);
            std::uniform_real_distribution<float> leftDist(leftMinX, leftMaxX);
            float xLeft = leftDist(rng);
            elements.emplace_back(xLeft, yPos, *(treeVarieties[leftTypeIdx].left));

            // Eger agac orta yola (icerilere) cok yaklastiysa ve alan genisse, 
            // ekranin uzaktaki kenari cok bos kalmasin diye en sola tampon bir agac daha ekle
            if (leftMaxX - xLeft < 50.f && (leftMaxX - leftMinX) > 80.f) {
                int extraLeftTypeIdx = typeDist(rng);
                // Biraz rastgelelik katarak en sola yerlestir
                std::uniform_real_distribution<float> edgeLeftDist(leftMinX, leftMinX + 15.f);
                elements.emplace_back(edgeLeftDist(rng), yPos, *(treeVarieties[extraLeftTypeIdx].left));
            }

            // Saga rastgele bir X sec ve rastgele bir agac turu yarat
            int rightTypeIdx = typeDist(rng);
            std::uniform_real_distribution<float> rightDist(rightMinX, rightMaxX);
            float xRight = rightDist(rng);
            elements.emplace_back(xRight, yPos, *(treeVarieties[rightTypeIdx].right));

            // Eger agac orta yola cok yaklastiysa, en saga tampon bir agac daha ekle
            if (xRight - rightMinX < 50.f && (rightMaxX - rightMinX) > 80.f) {
                int extraRightTypeIdx = typeDist(rng);
                std::uniform_real_distribution<float> edgeRightDist(rightMaxX - 15.f, rightMaxX);
                elements.emplace_back(edgeRightDist(rng), yPos, *(treeVarieties[extraRightTypeIdx].right));
            }
        }
    }

    void draw(sf::RenderWindow& window) {
        for (auto& el : elements) {
            el.draw(window);
        }
    }
};
