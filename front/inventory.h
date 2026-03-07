#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "gamestate.h"

// ============================================================
//  INVENTORY PANEL - Envanter Arayüzü
// ============================================================
//  GÖREV: Oyuncunun çantasını (backpack) ekranda gösterir.
//         INV butonuna tıklayınca açılır/kapanır.
//         Açıkken GamePanel'in üzerine yarı saydam koyu bir arka plan
//         ve 10 slotluk bir liste çizer.
//
//  BAĞLANTI: Application sınıfı bir InventoryPanel nesnesi tutar.
//            INV butonuna tıklayınca:
//              inventory.toggle(currentState)
//            Her frame:
//              inventory.updateHover(worldPos)   → hover renk değişimi
//              inventory.draw(window, font)      → ekrana çizim
//
//  BACKEND GELİNCE:
//    - slotNames vektörü Player::getInventory() ile doldurulacak
//    - handleClick() fonksiyonu eklenip item use/drop yapılacak
// ============================================================

// Eski koddaki envanter düzeni (graphic_test.cpp):
// Konum: gameStartX + 50, 50  (sol panelin içi)
// Başlık: "--- BACKPACK ---" altın sarısı, charSize 20
// Slot Aralığı: 25px
// Dolu slot rengi: Kırmızı (Red), hover: açık kırmızı (255,100,100)
// Boş slot rengi: Gri (100,100,100)

struct InventoryPanel {

    // --- Durum ---
    bool isOpen = false;  // Envanter şu an açık mı?

    // --- Görsel Elemanlar ---
    sf::RectangleShape background; // GamePanel üzerine çizilen yarı saydam koyu arka plan

    // --- Slot Verileri ---
    // Şimdilik 10 boş slot. Backend gelince bu vektör
    // Player::getInventory()'den dolacak.
    std::vector<std::string> slotNames;

    // --- Hover Takibi ---
    int hoveredSlot = -1; // Fare hangi slotun üzerinde? (-1 = hiçbiri)

    // --- Sabit Düzen Değerleri (graphic_test.cpp'den) ---
    static constexpr float START_X = 100.f + 50.f;  // GAME_START_X (100) + padding (50)
    static constexpr float START_Y = 50.f;           // Üstten boşluk
    static constexpr float TITLE_HEIGHT = 40.f;      // Başlık ile ilk slot arası mesafe
    static constexpr float LINE_HEIGHT = 25.f;       // Slotlar arası dikey mesafe
    static constexpr float LIST_WIDTH = 300.f;       // Tıklanabilir alan genişliği
    static constexpr int   MAX_SLOTS = 10;           // Maksimum slot sayısı

    // ============================================================
    //  CONSTRUCTOR
    // ============================================================
    //  Yarı saydam arka planı hazırlar.
    //  GamePanel boyutlarını kullanıyoruz (560x380) ki tam üstünü kaplasın.
    InventoryPanel() {
        background.setSize({560.f, 380.f});           // LEFT_WIDTH x SPLIT_Y
        background.setPosition({100.f, 0.f});         // GAME_START_X, 0
        background.setFillColor(sf::Color(0, 0, 0, 180)); // Siyah, %70 opak

        // 10 slotu boş olarak başlat
        slotNames.resize(MAX_SLOTS, "");
    }

    // ============================================================
    //  TOGGLE - Aç / Kapa
    // ============================================================
    //  INV butonuna tıklayınca çağrılır.
    //  State kontrolü burada yapılıyor:
    //    - DIALOGUE ve SHOP'ta INV butonu gri, bu fonksiyon çağrılmaz
    //      (ama yine de güvenlik için kontrol var)
    //    - EXPLORING ve COMBAT'ta toggle edilebilir
    //
    //  true dönerse "açıldı", false dönerse "kapandı" demek.
    //  Application bu dönüş değerine göre typewriter mesajı gösterebilir.
    bool toggle(GameState state) {
        // Gri state'lerde envanter açılamaz
        if (state == GameState::DIALOGUE || state == GameState::SHOP) {
            return false;
        }

        isOpen = !isOpen;
        hoveredSlot = -1; // Hover durumunu sıfırla
        return isOpen;
    }

    // ============================================================
    //  UPDATE HOVER - Fare Hangi Slotun Üzerinde?
    // ============================================================
    //  Her frame çağrılır. Farenin y koordinatına bakarak
    //  hangi slot satırının üzerinde olduğunu hesaplar.
    //
    //  Hesaplama mantığı:
    //    Slot listesi START_Y + TITLE_HEIGHT (= 90px) den başlar.
    //    Her slot 25px yüksekliğinde.
    //    (mouseY - listStartY) / LINE_HEIGHT = slot indeksi
    //
    //  Eğer fare listenin dışındaysa hoveredSlot = -1 olur.
    void updateHover(sf::Vector2f mousePos) {
        if (!isOpen) {
            hoveredSlot = -1;
            return;
        }

        float listStartY = START_Y + TITLE_HEIGHT;

        // Fare x ekseni: listenin yatay sınırları içinde mi?
        if (mousePos.x >= START_X && mousePos.x <= START_X + LIST_WIDTH) {
            // Fare y ekseni: hangi satıra denk geliyor?
            int index = static_cast<int>((mousePos.y - listStartY) / LINE_HEIGHT);

            if (index >= 0 && index < MAX_SLOTS) {
                hoveredSlot = index;
            } else {
                hoveredSlot = -1;
            }
        } else {
            hoveredSlot = -1;
        }
    }

    // ============================================================
    //  DRAW - Ekrana Çizim
    // ============================================================
    //  isOpen == false ise hiçbir şey çizmez.
    //  Açıksa sırayla çizer:
    //    1. Yarı saydam koyu arka plan (overlay)
    //    2. "--- BACKPACK ---" başlığı (altın sarısı)
    //    3. 10 slot satırı:
    //       - Dolu slot: "[1] Item Name" kırmızı (hover: açık kırmızı)
    //       - Boş slot:  "[1] - Empty -"  gri
    void draw(sf::RenderWindow& window, const sf::Font& font) {
        if (!isOpen) return;

        // 1. Arka plan overlay
        window.draw(background);

        // 2. Başlık
        sf::Text title(font);
        title.setString("--- BACKPACK ---");
        title.setCharacterSize(20);
        title.setFillColor(sf::Color(255, 215, 0)); // Altın sarısı
        title.setPosition({START_X, START_Y});
        window.draw(title);

        // 3. Slot listesi
        float currentY = START_Y + TITLE_HEIGHT;

        for (int i = 0; i < MAX_SLOTS; i++) {
            sf::Text slotText(font);
            slotText.setCharacterSize(18);
            slotText.setPosition({START_X, currentY});

            if (!slotNames[i].empty()) {
                // DOLU SLOT: Eşya ismi var
                slotText.setString("[" + std::to_string(i + 1) + "] " + slotNames[i]);

                // Hover renk değişimi: fare üstündeyse açık kırmızı, değilse koyu kırmızı
                if (i == hoveredSlot) {
                    slotText.setFillColor(sf::Color(255, 100, 100)); // Açık kırmızı
                } else {
                    slotText.setFillColor(sf::Color::Red);           // Kırmızı
                }
            } else {
                // BOŞ SLOT: Placeholder yazısı
                slotText.setString("[" + std::to_string(i + 1) + "] - Empty -");
                slotText.setFillColor(sf::Color(100, 100, 100));     // Gri
            }

            window.draw(slotText);
            currentY += LINE_HEIGHT;
        }
    }
};
