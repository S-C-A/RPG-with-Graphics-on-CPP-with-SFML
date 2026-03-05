#pragma once
#include <SFML/Graphics.hpp>
#include <sstream>
#include <string>

// ============================================================
//  TYPEWRITER - Diyalog Kutusu Metin Yöneticisi
// ============================================================
//  GÖREV: Verilen metni, diyalog kutusuna sığacak şekilde
//         kelime kelime satırlara böler ve harf harf, belirli
//         bir hızda ekrana yazdırır (Daktilo efekti).
//
//  BAĞLANTI: Application sınıfı bir Typewriter nesnesi tutar.
//            Her frame:
//              1. typewriter.update()  çağrılır (bir harf ilerler)
//              2. typewriter.draw(window, font) çağrılır (metni çizer)
//            Yeni bir mesaj göstermek için:
//              typewriter.start("Mesaj buraya");
// ============================================================

struct Typewriter {

    // --- Sabit Düzen Değerleri (graphic_test.cpp'den alınmıştır) ---
    // Diyalog kutusunun sol-üst köşesinden ne kadar içeride başlayacağı
    static constexpr float PAD_X    = 47.f;
    static constexpr float PAD_Y    = 30.f;
    // Yazının sığacağı maksimum piksel genişliği (kutu içi boşluk kadar)
    static constexpr int   WRAP_WIDTH = 480;
    // Karakter boyutu
    static constexpr int   CHAR_SIZE  = 16;
    // Harf başına geçen süre (saniye). 0.025 = ~40 harf/saniye
    static constexpr float SPEED = 0.025f;

    // --- İç Durum ---
    std::string fullText;    // wrapText'ten geçirilmiş, sığdırılmış tam metin
    std::string currentText; // Şu ana kadar ekrana yansıyan kısım
    size_t      charIndex;   // Sıradaki harfin index'i
    bool        isFinished;  // Tüm metin yazılıp bitti mi?
    sf::Clock   charClock;   // Son harf eklenmesinden bu yana geçen süre

    Typewriter() : charIndex(0), isFinished(true) {}

    // --- wrapText ---
    // GÖREV: Uzun bir metni, verilen piksel genişliğine sığacak şekilde
    //        kelime kelime alt satıra böler.
    //        Önemli: Metinde zaten varsa '\n' karakterlerini KORUR.
    //        Böylece oyun motorunun koyduğu kasıtlı boşluklar kaybolmaz.
    // PARAMETRE: text      → Bölünecek ham metin
    //            font      → Piksel genişliği ölçmek için gerekli
    //            maxWidth  → Bu piksel sayısını aşınca alt satıra geç
    std::string wrapText(const std::string& text, const sf::Font& font, int maxWidth) {
        std::string result = "";
        std::string currentLine = "";
        std::stringstream lineStream(text);
        std::string segment;

        // Geçici ölçüm için bir sf::Text nesnesi
        sf::Text ruler(font);
        ruler.setCharacterSize(CHAR_SIZE);

        // Metni önce '\n' sınırlarına göre parçalara böl
        while (std::getline(lineStream, segment, '\n')) {
            std::stringstream wordStream(segment);
            std::string word;

            // Her kelimeyi tek tek satıra eklemeyi dene
            while (wordStream >> word) {
                std::string testLine = currentLine + (currentLine.empty() ? "" : " ") + word;
                ruler.setString(testLine);

                if (ruler.getLocalBounds().size.x > maxWidth) {
                    // Bu kelime satıra sığmadı: mevcut satırı kaydet, yeni satıra geç
                    result += currentLine + "\n";
                    currentLine = word;
                } else {
                    // Sığdı: kelimeyi satıra ekle
                    currentLine = testLine;
                }
            }
            // Bu '\n' parçası bitti: kalanı ekle ve orijinal satır sonunu koru
            result += currentLine + "\n";
            currentLine = "";
        }
        return result;
    }

    // --- start ---
    // GÖREV: Yeni bir mesajı daktilo efektiyle yazmaya başlatır.
    //        Önce metni wrapText'e gönderir, ardından sayaçları sıfırlar.
    // KULLANIM: typewriter.start("Kapıyı açtınız.");
    void start(const std::string& message, const sf::Font& font) {
        fullText    = wrapText(message, font, WRAP_WIDTH);
        currentText = "";
        charIndex   = 0;
        isFinished  = false;
        charClock.restart();
    }

    // --- update ---
    // GÖREV: Her frame çağrılır. Yeterli süre geçtiyse sıradaki harfi
    //        currentText'e ekler ve saati sıfırlar.
    //        isFinished true olduğunda artık hiçbir şey yapmaz.
    // BAĞLANTI: Application::run() döngüsünün update bölümünde çağrılır.
    void update() {
        if (isFinished) return;

        if (charClock.getElapsedTime().asSeconds() > SPEED) {
            currentText += fullText[charIndex];
            charIndex++;
            charClock.restart();

            if (charIndex >= fullText.size()) {
                isFinished = true;
            }
        }
    }

    // --- skip ---
    // GÖREV: Fare tıklandığında metni anında tamamlar (yazdırmayı atlar).
    // KULLANIM: if (isMouseJustClicked) typewriter.skip();
    void skip() {
        currentText = fullText;
        charIndex   = fullText.size();
        isFinished  = true;
    }

    // --- draw ---
    // GÖREV: currentText'i DialogBox'ın doğru konumuna çizer.
    // PARAMETRE: dialogPos → DialogBox sprite'ının sol-üst köşesi (getPosition())
    //            Bu değer + PAD_X/PAD_Y kaydırılarak yazı başlar.
    void draw(sf::RenderWindow& window, const sf::Font& font, sf::Vector2f dialogPos) {
        sf::Text label(font);
        label.setCharacterSize(CHAR_SIZE);
        label.setFillColor(sf::Color(110, 0, 0)); // BORDEAUX_COLOR
        label.setString(currentText);
        label.setPosition({dialogPos.x + PAD_X, dialogPos.y + PAD_Y});
        window.draw(label);
    }
};
