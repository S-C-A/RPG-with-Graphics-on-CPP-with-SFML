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

        // 4. Başlangıç butonlarını oda çıkışlarına göre ayarla
        buttonMenu->applyStateWithRoom(currentState, game.getCurrentRoom(), game.getRoomNPC(),
            buttonTex, buttonGreyTex, mapTex, mapGreyTex, invTex, invGreyTex);

        // 5. Stat panelini gerçek değerlerle doldur
        statBox->syncWithPlayer(game.getPlayer());

        // 6. Başlangıç odasındaki nesneleri yükle
        worldObjects.syncWithRoom(game);
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
                    if (key->code == sf::Keyboard::Key::Y) currentState = GameState::EXPLORING;
                    if (key->code == sf::Keyboard::Key::U) currentState = GameState::COMBAT;
                    if (key->code == sf::Keyboard::Key::I) currentState = GameState::DIALOGUE;
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
                // Gri state'lerde açılamaz mesajı
                if (currentState == GameState::DIALOGUE || currentState == GameState::SHOP) {
                    typewriter.start("Can't open inventory right now.", font);
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
            std::string itemResult = inventory.handleClick(game, isLeftClick, isRightClick);
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
                    // Yeni odanın çıkışlarına göre butonları güncelle
                    buttonMenu->applyStateWithRoom(currentState, game.getCurrentRoom(), game.getRoomNPC(),
                        buttonTex, buttonGreyTex, mapTex, mapGreyTex, invTex, invGreyTex);
                    
                    // Yeni odadaki objeleri (eşya vs.) yükle
                    worldObjects.syncWithRoom(game);
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
                            // Ileride buraya shopMenu eklenecek
                            buttonMenu->applyStateWithRoom(currentState, game.getCurrentRoom(), game.getRoomNPC(),
                                buttonTex, buttonGreyTex, mapTex, mapGreyTex, invTex, invGreyTex);
                        } 
                        else {
                            // NPc konusmaya devam ediyor
                            typewriter.start(response, font);
                            dialogueMenu.clear(); // Yeni secenekler yazi bitince yuklenecek
                        }
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
            if (!inventory.drawHoverDesc(window, font, dialogBox->sprite.getPosition(), game)) {
                typewriter.draw(window, font, dialogBox->sprite.getPosition());
            }

            // Diyalog Secenekleri cizimi (yazi sirasinda cizilmez)
            if (currentState == GameState::DIALOGUE && typewriter.isFinished) {
                dialogueMenu.draw(window);
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
