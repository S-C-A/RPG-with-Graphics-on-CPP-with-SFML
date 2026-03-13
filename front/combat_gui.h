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
    bool isSelectingTarget = false;
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

    // Oyuncu bir aksiyonu (item, attack vb.) tamamladiginda cagrilir
    void endPlayerTurn() {
        if (!isPlayerTurn) return;
        isPlayerTurn = false;
        isSelectingTarget = false;
        enemyIndex = 0;
        turnTimer.restart();
    }

    // Hedef secme modunu ac/kapat
    void startTargetSelection(Typewriter& tw, const sf::Font& font) {
        if (!isPlayerTurn || isSelectingTarget) return;
        isSelectingTarget = true;
        tw.start("Select a target by clicking on an enemy.", font);
    }

    // Belirli bir dusmana saldir
    void handleAttack(int targetIdx, Game& game, Typewriter& tw, const sf::Font& font) {
        if (!isSelectingTarget || targetIdx < 0 || targetIdx >= (int)activeEnemies.size()) return;
        
        Monster* target = activeEnemies[targetIdx];
        if (!target || target->isDead()) {
            tw.start("That enemy is already dead! Select another target.", font);
            return;
        }

        // Backend saldiri islemi
        std::string attackMsg = game.getCombatManager()->attackTarget(&game.getPlayer(), target);
        
        // Eger dusman oldu ise loot raporu ekle
        if (target->isDead()) {
            std::string lootMsg = game.getCombatManager()->collectLootUI(&game.getPlayer(), target, game.getItemManager());
            if (!lootMsg.empty()) {
                attackMsg += "\n" + lootMsg;
            }
            
            Room* r = game.getCurrentRoom();
            if (r && targetIdx < (int)r->monsterID.size()) {
                r->monsterID[targetIdx] = -1;
            }
            turnTimer.restart(); // Olum mesajının okunması icin timer reset
        }

        tw.start(attackMsg, font);
        endPlayerTurn(); 
    }

    // Oyuncu sırasını atladığında (L tuşu vb.)
    void skipPlayerTurn(Typewriter& tw, const sf::Font& font) {
        if (!isPlayerTurn) return;
        
        isPlayerTurn = false;
        isSelectingTarget = false;
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

    // Odadaki canlı düşmanları listeleyen giriş mesajı
    std::string getCombatIntro() {
        std::string result = "";
        for (auto m : activeEnemies) {
            if (m && !m->isDead()) {
                if (!result.empty()) result += "\n";
                result += "[" + m->getName() + "] blocks your path!";
            }
        }
        
        if (result.empty()) return "Enemies appear!";
        return result;
    }
};
