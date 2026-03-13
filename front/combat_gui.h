#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "../monster.h"
#include "../game.h"
#include "typewriter.h"

// ============================================================
//  COMBAT GUI - Savaş Mantığı ve UI Kontrolörü
// ============================================================
//  GÖREV: Savaşın tur sırasını, düşman hamlelerini ve hamleler
//         arasındaki okuma gecikmesini yönetir.
// ============================================================

class CombatGUI {
public:
    bool isPlayerTurn = true;
    int  enemyIndex = 0;
    std::vector<Monster*> activeEnemies;

    // Tur geçişleri arasındaki bekleme süresi için
    sf::Clock turnTimer;
    static constexpr float TURN_DELAY = 1.2f; // Saniye cinsinden bekleme

    CombatGUI() : isPlayerTurn(true), enemyIndex(0) {}

    // Savaş başlangıcında odadaki düşmanları klonlar
    void setup(Game& game) {
        cleanup(); // Önceki düşmanları temizle
        
        Room* r = game.getCurrentRoom();
        if (r) {
            for (int id : r->monsterID) {
                if (id != -1) {
                    Monster* m = game.getMonsterClone(id);
                    if (m) activeEnemies.push_back(m);
                }
            }
        }
        isPlayerTurn = true;
        enemyIndex = 0;
        turnTimer.restart();
    }

    // Savaş bittiğinde düşman nesnelerini siler
    void cleanup() {
        for (auto m : activeEnemies) delete m;
        activeEnemies.clear();
        enemyIndex = 0;
    }

    // Oyuncu sırasını atladığında (L tuşu vb.)
    void skipPlayerTurn(Typewriter& tw, const sf::Font& font) {
        if (!isPlayerTurn) return;
        
        isPlayerTurn = false;
        enemyIndex = 0;
        turnTimer.restart();
        tw.start("Your turn skipped. Enemies are moving...", font);
    }

    // Her frame application.h'tan çağrılır
    void updateTurn(Game& game, Typewriter& tw, const sf::Font& font) {
        if (isPlayerTurn) return; // Oyuncu sırasındayken bir şey yapma
        
        // Eğer daktilo hala yazıyorsa bekle
        if (tw.isBusy()) {
            turnTimer.restart(); // Yazı bittikten sonra süre başlasın
            return;
        }

        // Yazı bitti, peki üzerinden okuma süresi (delay) geçti mi?
        if (turnTimer.getElapsedTime().asSeconds() < TURN_DELAY) {
            return;
        }

        // Bekleme süresi doldu, sıradaki düşman hareket etsin
        if (enemyIndex < activeEnemies.size()) {
            Monster* m = activeEnemies[enemyIndex];
            
            if (m && !m->isDead()) {
                std::string moveMsg = m->makeMove(&game.getPlayer());
                tw.start(moveMsg, font);
                turnTimer.restart(); // Yeni mesajın bitmesi için reset
            }
            enemyIndex++;
        } else {
            // Tüm düşmanlar oynadı, sıra oyuncuya geçer
            isPlayerTurn = true;
            tw.start("Your turn! (Use buttons or 'L' to skip)", font);
        }
    }

    // Tüm düşmanlar öldü mü kontrolü (UI'dan silinenler dahil)
    bool allEnemiesDefeated(Room* r) {
        if (!r) return true;
        for (int id : r->monsterID) {
            if (id != -1) return false;
        }
        return true;
    }
};
