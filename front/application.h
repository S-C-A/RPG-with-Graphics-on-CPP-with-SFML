#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include <unordered_map>
#include <string>
#include "ui_elements.h"
#include "typewriter.h"
#include "gamestate.h"
#include "inventory.h"
#include "world_objects.h"
#include "combat_gui.h"
#include "../game.h"

class Application {
private:
    sf::RenderWindow window;
    sf::View gameView;

    // === MERKEZI DEPO: Tüm Texture'lar burada, bir kez yüklenir ===
    sf::Texture dialogTex;
    sf::Texture statTex;
    sf::Texture buttonTex;
    sf::Texture mapTex;
    sf::Texture invTex; 
    sf::Texture buttonGreyTex;
    sf::Texture mapGreyTex;
    sf::Texture invGreyTex;
    sf::Texture lootTex;      // Yerdeki ganimet gorseli

    // -------------------------------------------------------
    //  DUSMAN TEXTURE HARITASI
    //  Anahtar : Monster::getName() sonucu ("Bandit Slasher")
    //  Deger   : Idle + Attack texture cifti (EnemyTextureSet)
    //
    //  Yeni bir dusman turu eklendiginde:
    //    1. enemies.h'ta sinifi tanimla
    //    2. Buraya yeni bir blok ekle ve PNG'leri yukle
    //    3. syncWithRoom otomatik olarak yeni texture'yi kullanir
    // -------------------------------------------------------
    std::unordered_map<std::string, EnemyTextureSet> enemyTexMap;

    // -------------------------------------------------------
    //  NPC TEXTURE HARITASI
    //  Anahtar : NPC adi ("Old Sage", "Mysterious Merchant")
    //  Deger   : NPC texture seti (idle gorseli)
    // -------------------------------------------------------
    std::unordered_map<std::string, NPCTextureSet> npcTexMap;

    // UI Elemanları (Texture'ları referans olarak yukarıdan alır)
    std::optional<GamePanel>   gamePanel;
    std::optional<DialogBox>   dialogBox;
    std::optional<StatBox>     statBox;
    std::optional<ButtonMenu>  buttonMenu;

    // Font - Typewriter için gerekli, merkezi depoda tutuluyor
    sf::Font font;
    Typewriter typewriter;

    // Oyunun mevcut state'i (TEMPORARİ: Y/U/I/O tuşlarıyla değiştirilir)
    GameState currentState = GameState::EXPLORING;

    // Envanter paneli
    InventoryPanel inventory;

    // === BACKEND ===
    Game game;

    // Odadaki Nesneler (Yerdeki esya, NPC, vb.)
    WorldObjects worldObjects;

    // Diyalog Menusu
    DialogueMenu dialogueMenu;

    // Dukkan Menusu
    ShopMenu shopMenu;

    // --- COMBAT GUI ---
    CombatGUI combat;

public:
    Application() {
        // Fullscreen window
        window.create(sf::VideoMode::getDesktopMode(), "KEYBEARER", sf::State::Fullscreen);
        window.setFramerateLimit(60);

        // 960x540 çözünürlük oranını tam ekrana sündür
        gameView = sf::View(sf::FloatRect({0.f, 0.f}, {960.f, 540.f}));
        window.setView(gameView);

        // 1. Resimleri MERKEZİ DEPODA yükle
        if (!dialogTex.loadFromFile("textures/Textbox[Final].png"))   return;
        if (!statTex.loadFromFile("textures/Statbox[Final].png"))     return;
        if (!buttonTex.loadFromFile("textures/Button[Final].png"))    return;
        if (!mapTex.loadFromFile("textures/Map[Final].png"))          return;
        if (!invTex.loadFromFile("textures/Inventory[Final].png"))    return;
        if (!buttonGreyTex.loadFromFile("textures/Button[Grey].png")) return;
        if (!mapGreyTex.loadFromFile("textures/Map[Gray].png"))       return;
        if (!invGreyTex.loadFromFile("textures/Inventory[Gray].png")) return;
        if (!lootTex.loadFromFile("textures/Loot[Final].png"))        return;

        // --- DUSMAN TEXTURE'LARI ---
        // Her dusman blogu ayni yapidadir:
        //   1. haritada o dusmanin adina ait bir slot ac
        //   2. idle PNG'yi yukle
        //   3. attack PNG'yi yukle
        //      (Test asamasinda ikisi de ayni dosyayi kullanir.)
        {
            // Bandit Slasher: idle ve attack icin artik ayri PNG'ler
            EnemyTextureSet& bandit = enemyTexMap["Bandit Slasher"];
            if (!bandit.idle.loadFromFile("textures/Enemies/Bandit[Final].png"))   return;
            if (!bandit.attack.loadFromFile("textures/Enemies/Bandit[Attack].png")) return;
        }
        // Yeni dusman turu icin ornek:
        // {
        //     EnemyTextureSet& wolf = enemyTexMap["Forest Wolf"];
        //     if (!wolf.idle.loadFromFile("textures/Enemies/Wolf[Final].png"))   return;
        //     if (!wolf.attack.loadFromFile("textures/Enemies/Wolf[Attack].png")) return;
        // }

        // --- NPC TEXTURE'LARI ---
        {
            // Old Sage (yasli adam) icin oracle png'sini kullaniyoruz
            NPCTextureSet& sage = npcTexMap["Old Sage"];
            if (!sage.idle.loadFromFile("textures/NPC/Oracle[Final].png")) return;
            
            // Mysterious Merchant icin vagon png'si
            NPCTextureSet& merchant = npcTexMap["Mysterious Merchant"];
            if (!merchant.idle.loadFromFile("textures/NPC/Wagon[Final].png")) return;
        }

        // 2. UI Elemanlarını oluştur, resimleri referans olarak ver
        gamePanel.emplace();
        dialogBox.emplace(dialogTex);
        statBox.emplace(statTex);
        buttonMenu.emplace(buttonTex, mapTex, invTex);

        // 3. Font yükle ve test mesajı başlat
        if (!font.openFromFile("font.ttf")) return;
        typewriter.start(game.lookAtRoom(), font);

        // 4. Loot, Dusman ve NPC texture'lerini WorldObjects'e tanitiyoruz,
        //    ardindan baslangic odasini yukluyoruz.
        //    SIRA ONEMLI: once set, sonra sync (sync lookup yapar).
        worldObjects.setLootTexture(lootTex);
        worldObjects.setEnemyTexMap(enemyTexMap);
        worldObjects.setNPCTexMap(npcTexMap);
        worldObjects.syncWithRoom(game);

        // Odada dusman varsa savas modunda basla
        bool hasAliveEnemies = false;
        if (game.getCurrentRoom()) {
            for (int id : game.getCurrentRoom()->monsterID) {
                if (id != -1) hasAliveEnemies = true;
            }
        }
        if (hasAliveEnemies) {
            currentState = GameState::COMBAT;
            combat.setup(game);
            typewriter.start(combat.getCombatIntro(), font);
        }

        // 5. Başlangıç butonlarını oda çıkışlarına göre ayarla
        buttonMenu->applyStateWithRoom(currentState, game.getCurrentRoom(), game.getRoomNPC(),
            buttonTex, buttonGreyTex, mapTex, mapGreyTex, invTex, invGreyTex);

        // 6. Stat panelini gerçek değerlerle doldur
        statBox->syncWithPlayer(game.getPlayer());
    }

    void run() {
        while (window.isOpen()) {
            
            // Event Handling
            bool isLeftClick = false;
            bool isRightClick = false;
            while (const std::optional eventOpt = window.pollEvent()) {
                const auto& event = *eventOpt;
                if (event.is<sf::Event::Closed>()) window.close();
                if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
                    if (key->code == sf::Keyboard::Key::Escape) window.close();
                    // --- TEST: STATE DEĞİŞTİRME (İLERİDE SİLİNECEK) ---
                    if (key->code == sf::Keyboard::Key::O) currentState = GameState::SHOP;
                    
                    // State değişince butonları güncelle
                    buttonMenu->applyStateWithRoom(currentState, game.getCurrentRoom(), game.getRoomNPC(),
                        buttonTex, buttonGreyTex, mapTex, mapGreyTex, invTex, invGreyTex);
                }
                // Fare tıklaması: hangi buton olduğunu da sakla
                if (const auto* mousePress = event.getIf<sf::Event::MouseButtonPressed>()) {
                    if (mousePress->button == sf::Mouse::Button::Left)
                        isLeftClick = true;
                    else if (mousePress->button == sf::Mouse::Button::Right)
                        isRightClick = true;
                }
            }

            // Mouse
            sf::Vector2f worldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window), gameView);
            bool isMousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
            buttonMenu->update(worldPos, isMousePressed);

            bool isMouseJustClicked = isLeftClick || isRightClick;

            // INV butonu tıklama kontrolü (buton 5 = INV)
            if (isMouseJustClicked && buttonMenu->buttons[5].bounds.contains(worldPos)) {
                // Sadece Diyalogda kapali kalsin
                if (currentState == GameState::DIALOGUE) {
                    typewriter.start("Can't open inventory during dialogue.", font);
                } else {
                    bool opened = inventory.toggle(currentState);
                    if (opened)
                        typewriter.start("Inventory Opened.", font);
                    else {
                        if (currentState == GameState::EXPLORING)
                            typewriter.start(game.lookAtRoom(), font);
                        else
                            typewriter.start(combat.getCombatIntro(), font);
                    }
                }
            }

            // Envanter hover güncelleme
            inventory.updateHover(worldPos);

            // Typewriter: güncelle, tıklanınca animasyonu atla
            typewriter.update();
            if (isLeftClick && !buttonMenu->buttons[5].bounds.contains(worldPos))
                typewriter.skip();

            // --- ENVANTER EŞYA ETKİLEŞİMİ ---
            bool isSelling = (currentState == GameState::SHOP && shopMenu.isSellingMode);
            std::string itemResult = inventory.handleClick(game, isLeftClick, isRightClick, isSelling);
            if (!itemResult.empty()) {
                typewriter.start(itemResult, font);
                if (currentState == GameState::COMBAT) {
                    inventory.isOpen = false; // Cantayi otomatik kapat
                    combat.endPlayerTurn();
                }
            }

            // Yön butonlarına tıklama (EXPLORING modunda, envanter kapalıyken)
            if (isMouseJustClicked && currentState == GameState::EXPLORING && !inventory.isOpen) {
                Room* room = game.getCurrentRoom();
                NPC* roomNPC = game.getRoomNPC();
                std::string moveMsg;

                // Engelleme kontrolü (hasMet değilse butonlara tıklanması hareket ettirmez)
                bool isMovementBlocked = (roomNPC && !roomNPC->hasMet());

                if (buttonMenu->buttons[0].bounds.contains(worldPos) && room->n != -1) {
                    if (isMovementBlocked) moveMsg = "There is someone who catches your interest.";
                    else moveMsg = game.attemptMove(room->n);
                }
                else if (buttonMenu->buttons[1].bounds.contains(worldPos) && room->w != -1) {
                    if (isMovementBlocked) moveMsg = "There is someone who catches your interest.";
                    else moveMsg = game.attemptMove(room->w);
                }
                else if (buttonMenu->buttons[2].bounds.contains(worldPos) && room->e != -1) {
                    if (isMovementBlocked) moveMsg = "There is someone who catches your interest.";
                    else moveMsg = game.attemptMove(room->e);
                }
                else if (buttonMenu->buttons[3].bounds.contains(worldPos) && room->s != -1) {
                    if (isMovementBlocked) moveMsg = "There is someone who catches your interest.";
                    else moveMsg = game.attemptMove(room->s);
                }

                if (!moveMsg.empty()) {
                    typewriter.start(moveMsg, font);
                    
                    // Yeni odadaki objeleri (eşya vs.) yükle
                    worldObjects.syncWithRoom(game);

                    // Odada dusman varsa savasa gir
                    bool hasAliveEnemies = false;
                    Room* r = game.getCurrentRoom();
                    if (r) {
                        for (int id : r->monsterID) {
                            if (id != -1) hasAliveEnemies = true;
                        }
                    }

                    if (hasAliveEnemies) {
                        currentState = GameState::COMBAT;
                        combat.setup(game);
                        typewriter.start(combat.getCombatIntro(), font);
                    }

                    // Yeni odanın çıkışlarına göre butonları güncelle
                    buttonMenu->applyStateWithRoom(currentState, game.getCurrentRoom(), game.getRoomNPC(),
                        buttonTex, buttonGreyTex, mapTex, mapGreyTex, invTex, invGreyTex);

                    isMouseJustClicked = false; 
                }
            }

            // ==========================================
            // COMBAT SISTEMI (Hedef Secme ve Saldiri)
            // ==========================================
            if (isMouseJustClicked && currentState == GameState::COMBAT && !inventory.isOpen) {
                // 1. ATTACK butonuna basildi mi?
                if (buttonMenu->buttons[0].bounds.contains(worldPos) && combat.isPlayerTurn) {
                    combat.startTargetSelection(typewriter, font);
                }
                
                // 2. Hedef secme modundaysak ve bir dusmana tiklandiysa
                else if (combat.isSelectingTarget) {
                    int clickedIdx = worldObjects.handleLeftClickEnemy(worldPos);
                    if (clickedIdx != -1) {
                        // Saldiridan once dusmanin mevcut durumunu kaydet
                        Room* r = game.getCurrentRoom();
                        bool wasDead = (r && clickedIdx < (int)r->monsterID.size())
                                       ? (r->monsterID[clickedIdx] == -1)
                                       : true;

                        combat.handleAttack(clickedIdx, game, typewriter, font);

                        // Saldiridan sonra: oldu mu?
                        bool isDead = (r && clickedIdx < (int)r->monsterID.size())
                                      ? (r->monsterID[clickedIdx] == -1)
                                      : true;

                        if (!wasDead && isDead) {
                            // Dusman oldu: gorseli sahnedem kaldir
                            worldObjects.syncWithRoom(game);
                        } else if (!isDead) {
                            // Dusman hayatta: hit-flash animasyonu baslat
                            worldObjects.startEnemyFlash(clickedIdx);
                        }

                        statBox->syncWithPlayer(game.getPlayer());
                    }
                }
            }

            // Savas bitti mi kontrolu (Her tık sonrası veya tur sonrası)
            if (currentState == GameState::COMBAT && combat.allEnemiesDefeated(game.getCurrentRoom()) 
                && !typewriter.isBusy() && combat.turnTimer.getElapsedTime().asSeconds() > combat.TURN_DELAY) 
            {
                currentState = GameState::EXPLORING;
                combat.cleanup();
                typewriter.start("Enemies defeated! " + game.lookAtRoom(), font);
                buttonMenu->applyStateWithRoom(currentState, game.getCurrentRoom(), game.getRoomNPC(),
                    buttonTex, buttonGreyTex, mapTex, mapGreyTex, invTex, invGreyTex);
            }

            // ==========================================
            // COMBAT TURN LOGIC (Sırayla hamle yapma)
            // ==========================================
            if (currentState == GameState::COMBAT) {
                combat.updateTurn(game, typewriter, font, worldObjects);
                statBox->syncWithPlayer(game.getPlayer());
            }

            // ==========================================
            // SHOP SISTEMI (Secenek Butonlari)
            // ==========================================
            if (isMouseJustClicked && currentState == GameState::SHOP) {
                // 0: BUY, 1: SELL, 2: TALK, 3: EXIT
                if (buttonMenu->buttons[0].bounds.contains(worldPos)) {
                    if (inventory.isOpen) inventory.toggle(GameState::EXPLORING); // Envanteri kapat
                    shopMenu.isSellingMode = false;
                    typewriter.start("What are you buying?", font);
                    std::vector<std::string> opts = game.getShopItems();
                    shopMenu.loadOptions(opts, font, {dialogBox->sprite.getPosition().x + 50.f, dialogBox->sprite.getPosition().y + 68.f});
                }
                else if (buttonMenu->buttons[1].bounds.contains(worldPos)) {
                    shopMenu.isSellingMode = true;
                    typewriter.start("What are you selling? (Click your inventory)", font);
                    shopMenu.clear();
                    if (!inventory.isOpen) inventory.toggle(currentState); // Satis icin envanteri ac
                }
                else if (buttonMenu->buttons[2].bounds.contains(worldPos)) {
                    if (inventory.isOpen) inventory.toggle(GameState::EXPLORING); // Envanteri kapat
                    currentState = GameState::DIALOGUE;
                    shopMenu.clear();
                    game.exitShop();
                    std::string startText = game.startDialogue(game.getRoomNPC());
                    typewriter.start(startText, font);
                    buttonMenu->applyStateWithRoom(currentState, game.getCurrentRoom(), game.getRoomNPC(),
                        buttonTex, buttonGreyTex, mapTex, mapGreyTex, invTex, invGreyTex);
                }
                else if (buttonMenu->buttons[3].bounds.contains(worldPos)) {
                    if (inventory.isOpen) inventory.toggle(GameState::EXPLORING); // Envanteri kapat
                    currentState = GameState::EXPLORING;
                    game.exitShop();
                    shopMenu.clear();
                    typewriter.start(game.lookAtRoom(), font);
                    buttonMenu->applyStateWithRoom(currentState, game.getCurrentRoom(), game.getRoomNPC(),
                        buttonTex, buttonGreyTex, mapTex, mapGreyTex, invTex, invGreyTex);
                }
            }

            // Dünya nesneleriyle etkileşim (EXPLORING, envanter kapalı, sol tık)
            if (isLeftClick && currentState == GameState::EXPLORING && !inventory.isOpen) {
                std::string objMsg = worldObjects.handleLeftClick(game, worldPos); // Esya
                if (!objMsg.empty()) {
                    typewriter.start(objMsg, font);
                } else {
                    NPC* clickedNPC = worldObjects.handleLeftClickNPC(game, worldPos); // NPC
                    if (clickedNPC) {
                        currentState = GameState::DIALOGUE;
                        std::string startText = game.startDialogue(clickedNPC);
                        typewriter.start(startText, font);
                        
                        dialogueMenu.clear(); // Yeni konusma basliyor, menuyu temizle

                        // NPC'ye tiklandi, butonlari diyalog moduna al
                        buttonMenu->applyStateWithRoom(currentState, game.getCurrentRoom(), game.getRoomNPC(),
                            buttonTex, buttonGreyTex, mapTex, mapGreyTex, invTex, invGreyTex);
                    }
                }
            }

            // ==========================================
            // DIYALOG SISTEMI 
            // ==========================================
            if (currentState == GameState::DIALOGUE) {
                // Eger yazi bittiyse ve secenekler henuz yuklenmediyse
                if (typewriter.isFinished && dialogueMenu.isEmpty()) {
                    std::vector<std::string> opts = game.getDialogueOptions();
                    dialogueMenu.loadOptions(opts, font, {dialogBox->sprite.getPosition().x + 50.f, dialogBox->sprite.getPosition().y + 68.f});
                }
                
                // Hover guncellemesi
                dialogueMenu.updateHover(worldPos);

                // Seceneklere tiklama
                if (isLeftClick && typewriter.isFinished && !dialogueMenu.isEmpty()) {
                    int clickedIdx = dialogueMenu.getClickedOption(worldPos);
                    if (clickedIdx != -1) {
                        std::string response = game.selectDialogueOption(clickedIdx);
                        
                        if (response.empty()) {
                            // Konusma bitti
                            currentState = GameState::EXPLORING;
                            typewriter.start(game.lookAtRoom(), font);
                            buttonMenu->applyStateWithRoom(currentState, game.getCurrentRoom(), game.getRoomNPC(),
                                buttonTex, buttonGreyTex, mapTex, mapGreyTex, invTex, invGreyTex);
                            worldObjects.syncWithRoom(game); // Yerdeki yeni esyalari yukle
                            dialogueMenu.clear();
                        } 
                        else if (response == "COMBAT_START") {
                            currentState = GameState::COMBAT;
                            typewriter.start("ENEMIES ATTACK!", font);
                            dialogueMenu.clear();
                            buttonMenu->applyStateWithRoom(currentState, game.getCurrentRoom(), game.getRoomNPC(),
                                buttonTex, buttonGreyTex, mapTex, mapGreyTex, invTex, invGreyTex);
                        } 
                        else if (response == "SHOP_OPEN") {
                            currentState = GameState::SHOP;
                            game.enterShop();
                            typewriter.start("Welcome to my shop! Take a look.", font);
                            dialogueMenu.clear();
                            
                            shopMenu.isSellingMode = false;
                            std::vector<std::string> opts = game.getShopItems();
                            shopMenu.loadOptions(opts, font, {dialogBox->sprite.getPosition().x + 50.f, dialogBox->sprite.getPosition().y + 68.f});

                            buttonMenu->applyStateWithRoom(currentState, game.getCurrentRoom(), game.getRoomNPC(),
                                buttonTex, buttonGreyTex, mapTex, mapGreyTex, invTex, invGreyTex);
                        } 
                        else {
                            // NPC konusmaya devam ediyor
                            typewriter.start(response, font);
                            dialogueMenu.clear(); // Yeni secenekler yazi bitince yuklenecek
                        }
                    }
                }
            }

            // ==========================================
            // SHOP SISTEMI (Satin Alma - BUY Modu Eşya Seçimi)
            // ==========================================
            if (currentState == GameState::SHOP && !shopMenu.isSellingMode) {
                shopMenu.updateHover(worldPos);

                if (isLeftClick && typewriter.isFinished && !shopMenu.isEmpty()) {
                    int clickedIdx = shopMenu.getClickedOption(worldPos);
                    if (clickedIdx != -1) {
                        std::string result = game.buyShopItem(clickedIdx);
                        typewriter.start(result, font);
                        statBox->syncWithPlayer(game.getPlayer());
                    }
                }
            }

            // Her frame statları güncel tut
            statBox->syncWithPlayer(game.getPlayer());

            // Envanter slotlarını backend'den güncelle
            inventory.syncSlots(game);

            // Odadaki nesnelerin (esya) hover durumunu güncelle
            worldObjects.update(worldPos);

            // Render
            window.clear(sf::Color::Black);
            window.setView(gameView);
            gamePanel->draw(window);

            // Odadaki nesneleri GamePanel uzerine ciz (envanterin "altinda" kalsin)
            worldObjects.draw(window);

            // Envanter açıksa GamePanel ve nesnelerin üzerine çizilir
            inventory.draw(window, font);
            dialogBox->draw(window);
            statBox->draw(window, font);
            buttonMenu->draw(window, font);
            
            // Diyalog kutusu: hover varsa eşya açıklaması veya düsman bilgisi, yoksa typewriter yazisi
            if (inventory.drawHoverDesc(window, font, dialogBox->sprite.getPosition(), game, isSelling)) {
                // Envanter hoverı çizildi
            }
            else if (currentState == GameState::COMBAT && !inventory.isOpen && combat.drawEnemyHover(window, font, dialogBox->sprite.getPosition(), worldObjects, worldPos)) {
                // Düşman hoverı çizildi
            }
            else {
                typewriter.draw(window, font, dialogBox->sprite.getPosition());
            }

            // Diyalog Secenekleri cizimi (yazi sirasinda cizilmez)
            if (currentState == GameState::DIALOGUE && typewriter.isFinished) {
                dialogueMenu.draw(window);
            }

            // Shop Secenekleri cizimi
            if (currentState == GameState::SHOP && typewriter.isFinished && !shopMenu.isSellingMode) {
                shopMenu.draw(window);
            }
            // --- TEST: Sol üstte mevcut state'i göster (İLERİDE SİLİNECEK) ---
            sf::Text debugText(font);
            debugText.setCharacterSize(14);
            debugText.setFillColor(sf::Color(200, 200, 0));
            debugText.setString(stateToString(currentState));
            debugText.setPosition({5.f, 5.f});
            window.draw(debugText);
            window.display();
        }
    }
};
