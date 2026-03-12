#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include "ui_elements.h"
#include "typewriter.h"
#include "gamestate.h"
#include "inventory.h"
#include "world_objects.h"
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

    // --- COMBAT STATE ---
    bool isPlayerTurn = true;
    int enemyIndex = 0;
    std::vector<Monster*> activeEnemies;

    // Odanin dusmanlarini instantiate et
    void setupCombat() {
        // Eski listenin temizligi (Pointer'lar oldugu icin silinmeli)
        for (auto m : activeEnemies) delete m;
        activeEnemies.clear();

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
    }

    void cleanupCombat() {
        for (auto m : activeEnemies) delete m;
        activeEnemies.clear();
    }

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

        // 2. UI Elemanlarını oluştur, resimleri referans olarak ver
        gamePanel.emplace();
        dialogBox.emplace(dialogTex);
        statBox.emplace(statTex);
        buttonMenu.emplace(buttonTex, mapTex, invTex);

        // 3. Font yükle ve test mesajı başlat
        if (!font.openFromFile("font.ttf")) return;
        typewriter.start(game.lookAtRoom(), font);

        // 4. Başlangıç odasındaki nesneleri yükle
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
            setupCombat();
            typewriter.start("Enemies appeared! Prepare for battle.", font);
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
                    
                    // --- SAVAS: SIRA ATLAMA (L TUSU) ---
                    if (key->code == sf::Keyboard::Key::L && currentState == GameState::COMBAT && isPlayerTurn) {
                        isPlayerTurn = false;
                        enemyIndex = 0; // Dusman sirasini baslat
                        typewriter.start("Your turn skipped. Enemies are moving...", font);
                    }

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
                    else
                        typewriter.start(game.lookAtRoom(), font);
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
            if (!itemResult.empty())
                typewriter.start(itemResult, font);

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
                        setupCombat();
                        typewriter.start("Enemies blocked your path! Prepare for battle.", font);
                    }

                    // Yeni odanın çıkışlarına göre butonları güncelle
                    buttonMenu->applyStateWithRoom(currentState, game.getCurrentRoom(), game.getRoomNPC(),
                        buttonTex, buttonGreyTex, mapTex, mapGreyTex, invTex, invGreyTex);
                }
            }

            // ==========================================
            // COMBAT SISTEMI (Dusman Tiklama - Gecici Oldurme Testi)
            // ==========================================
            if (isLeftClick && currentState == GameState::COMBAT && !inventory.isOpen) {
                int clickedEnemyIdx = worldObjects.handleLeftClickEnemy(worldPos);
                if (clickedEnemyIdx != -1) {
                    Room* r = game.getCurrentRoom();
                    if (r && clickedEnemyIdx >= 0 && clickedEnemyIdx < r->monsterID.size()) {
                        // Tiklanan dusmani odadan silme, yerini korumasi icin -1 yap
                        r->monsterID[clickedEnemyIdx] = -1;
                        
                        // Odadaki nesneleri yeniden senkronize et
                        worldObjects.syncWithRoom(game);

                        bool allDead = true;
                        int remaining = 0;
                        for (int id : r->monsterID) {
                            if (id != -1) {
                                allDead = false;
                                remaining++;
                            }
                        }

                        if (allDead) {
                            currentState = GameState::EXPLORING;
                            cleanupCombat();
                            typewriter.start("Enemies defeated! " + game.lookAtRoom(), font);
                            buttonMenu->applyStateWithRoom(currentState, game.getCurrentRoom(), game.getRoomNPC(),
                                buttonTex, buttonGreyTex, mapTex, mapGreyTex, invTex, invGreyTex);
                        } else {
                            typewriter.start("Enemy killed! " + std::to_string(remaining) + " remaining.", font);
                        }
                    }
                }
            }

            // ==========================================
            // ENEMY TURN LOGIC (Sırayla hamle yapma)
            // ==========================================
            if (currentState == GameState::COMBAT && !isPlayerTurn && !typewriter.isBusy()) {
                if (enemyIndex < activeEnemies.size()) {
                    // Bir sonraki dusman hamlesi
                    Monster* m = activeEnemies[enemyIndex];
                    if (m && !m->isDead()) {
                        string moveMsg = m->makeMove(&game.getPlayer());
                        typewriter.start(moveMsg, font);
                        statBox->syncWithPlayer(game.getPlayer()); // Hasar aldiysak can guncellensin
                    }
                    enemyIndex++;
                } else {
                    // Tum dusmanlar oynadi, sira oyuncuya gecer
                    isPlayerTurn = true;
                    typewriter.start("Your turn! (Use buttons or 'L' to skip)", font);
                }
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
            
            // Diyalog kutusu: hover varsa eşya açıklaması, yoksa typewriter yazisi
            if (!inventory.drawHoverDesc(window, font, dialogBox->sprite.getPosition(), game, isSelling)) {
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
