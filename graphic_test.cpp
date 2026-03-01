#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <sstream> 

// --- ENTEGRASYON: GAME ENGINE ---
#include "Game.h" 
#include "combat.h"

enum class GameState {
    EXPLORING,
    COMBAT,
    DIALOGUE,
    SHOP 
};

const sf::Color BORDEAUX_COLOR = sf::Color(110, 0, 0); 

// --- YARDIMCI FONKSIYON: METIN KAYDIRMA (DUZELTILDI - SATIR KORUMALI) ---
std::string wrapText(sf::String text, int width, const sf::Font& font, int charSize) {
    std::string wrappedText = "";
    std::string currentLine = "";
    
    // 1. Once metni mevcut satir sonlarina (\n) gore parcalara boluyoruz.
    // Boylece oyunun koydugu zorunlu enter'lar kaybolmuyor.
    std::stringstream lineStream(text.toAnsiString());
    std::string segment;
    
    sf::Text tempText(font);
    tempText.setCharacterSize(charSize);

    while(std::getline(lineStream, segment, '\n')) {
        // 2. Her parcayi kelime kelime isleyip sigdiriyoruz
        std::stringstream wordStream(segment);
        std::string word;
        
        while (wordStream >> word) {
            std::string testLine = currentLine + (currentLine.empty() ? "" : " ") + word;
            tempText.setString(testLine);
            
            // Genislik sinirini asti mi?
            if (tempText.getLocalBounds().size.x > width) {
                wrappedText += currentLine + "\n";
                currentLine = word; 
            } else {
                currentLine = testLine;
            }
        }
        // Parca bitti, kalanini ekle ve orijinal satir sonunu (\n) koy
        wrappedText += currentLine + "\n";
        currentLine = "";
    }
    
    return wrappedText;
}

// --- STATLARI EKRANA YAZ ---
void updateStatText(sf::Text& text, Player& player) {
    std::string content;
    content += "   KEYBEARER\n";
    content += "     (Lvl " + std::to_string(player.getLvl()) + ")\n\n";
    content += "HP:   " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + "\n\n";
    content += "ATK:  " + std::to_string(player.getAtk()) + "\n";
    content += "DEF:  " + std::to_string(player.getDef()) + "\n\n";
    content += "GOLD: " + std::to_string(player.getGold()) + "\n"; 
    content += "EXP:  " + std::to_string(player.getExp()) + "\n\n";
    content += "Weapon\n[" + player.getWeaponName() + "]\n\n";
    content += "Armor\n[" + player.getArmorName() + "]";
    text.setString(content);
}

// --- ENVANTER CIZME ---
void drawInventory(sf::RenderWindow& window, sf::Font& font, Player& player, float startX, float startY, sf::Vector2f mousePos) {
    const auto& inv = player.getInventory();
    sf::Text title(font);
    title.setString("--- BACKPACK ---");
    title.setCharacterSize(20);
    title.setFillColor(sf::Color(255, 215, 0)); 
    title.setPosition({startX, startY});
    window.draw(title);

    float currentY = startY + 40.f; 
    for (int i = 0; i < 10; ++i) {
        sf::Text slotText(font);
        slotText.setCharacterSize(18);
        slotText.setPosition({startX, currentY});

        if (i < inv.size()) {
            slotText.setString("[" + std::to_string(i + 1) + "] " + inv[i]->getName());
            if (slotText.getGlobalBounds().contains(mousePos)) {
                slotText.setFillColor(sf::Color(255, 100, 100)); 
            } else {
                slotText.setFillColor(sf::Color::Red); 
            }
        } else {
            slotText.setString("[" + std::to_string(i + 1) + "] - Empty -");
            slotText.setFillColor(sf::Color(100, 100, 100)); 
        }
        window.draw(slotText);
        currentY += 25.f; 
    }
}

// --- DAKTILO EFEKTI ---
struct Typewriter {
    std::string fullText;     
    std::string currentText;  
    sf::Clock charClock;      
    size_t charIndex = 0;     
    float speed = 0.025f;     
    bool isFinished = true;   

    void start(std::string message) {
        fullText = message; 
        currentText = "";                 
        charIndex = 0;                    
        isFinished = false;               
        charClock.restart();
    }

    void update() { 
        if (charIndex < fullText.size()) {
            if (charClock.getElapsedTime().asSeconds() > speed) {
                currentText += fullText[charIndex]; 
                charIndex++;
                charClock.restart(); 
                if (charIndex >= fullText.size()) {
                    isFinished = true; 
                }
            }
        } 
    }
    
    void finishInstant() {
        currentText = fullText;
        charIndex = fullText.size();
        isFinished = true;
    }

    bool isBusy() { return (charIndex < fullText.size()); }
    std::string getCurrentText() { return currentText; }
};

// --- GORSEL YAPILAR ---
struct ItemTarget {
    sf::RectangleShape shape;
    std::string name;
    ItemTarget(std::string _name, float x, float y) {
        name = _name;
        shape.setSize({40.f, 40.f}); 
        shape.setPosition({x, y});
        shape.setFillColor(sf::Color::Yellow); 
        shape.setOutlineThickness(2.f);
        shape.setOutlineColor(sf::Color(200, 200, 0));
        shape.setRotation(sf::degrees(45.f)); 
    }
    void update(sf::Vector2f mousePos) {
        if (shape.getGlobalBounds().contains(mousePos)) {
            shape.setScale({1.1f, 1.1f});
            shape.setFillColor(sf::Color::White); 
        } else {
            shape.setScale({1.0f, 1.0f});
            shape.setFillColor(sf::Color::Yellow);
        }
    }
    bool isClicked(sf::Vector2f mousePos) { return shape.getGlobalBounds().contains(mousePos); }
};

struct NPCTarget {
    sf::RectangleShape shape;
    std::string name;
    NPCTarget(std::string _name, float x, float y) {
        name = _name;
        shape.setSize({50.f, 50.f}); 
        shape.setPosition({x, y});
        shape.setFillColor(sf::Color::Cyan); 
        shape.setOutlineThickness(2.f);
        shape.setOutlineColor(sf::Color::Blue);
        shape.setRotation(sf::degrees(45.f)); 
    }
    void update(sf::Vector2f mousePos) {
        if (shape.getGlobalBounds().contains(mousePos)) {
            shape.setScale({1.1f, 1.1f});
            shape.setFillColor(sf::Color::White); 
        } else {
            shape.setScale({1.0f, 1.0f});
            shape.setFillColor(sf::Color::Cyan);
        }
    }
    bool isClicked(sf::Vector2f mousePos) { return shape.getGlobalBounds().contains(mousePos); }
};

struct EnemyTarget {
    sf::RectangleShape shape;
    std::string id;
    sf::Color normalColor;
    sf::Color hoverColor;
    EnemyTarget(std::string _id, float x, float y) {
        id = _id;
        shape.setSize({100.f, 150.f}); 
        shape.setPosition({x, y});
        normalColor = sf::Color(50, 0, 0);   
        hoverColor = sf::Color(255, 50, 50); 
        shape.setFillColor(normalColor);
        shape.setOutlineThickness(2.f);
        shape.setOutlineColor(sf::Color::Black);
    }
    void update(sf::Vector2f mousePos) {
        if (shape.getGlobalBounds().contains(mousePos)) shape.setFillColor(hoverColor); 
        else shape.setFillColor(normalColor); 
    }
    bool isClicked(sf::Vector2f mousePos) { return shape.getGlobalBounds().contains(mousePos); }
};

void updateEnemiesInView(std::vector<EnemyTarget>& enemyShapes, Room* room, float startX, float startY) {
    enemyShapes.clear(); 
    if (!room) return;
    float offsetX = 0;
    for (int mID : room->monsterID) {
        EnemyTarget target("Monster_" + std::to_string(mID), startX + offsetX, startY);
        enemyShapes.push_back(target);
        offsetX += 120.f; 
    }
}

// --- DIYALOG SECENEKLERI ---
struct VisualOption {
    sf::Text text;
    int index; 
    VisualOption(sf::Font& font, std::string _text, int _index, float x, float y) : index(_index), text(font) {
        text.setString("> " + _text); 
        text.setCharacterSize(18);
        text.setFillColor(sf::Color(200, 200, 200)); 
        text.setPosition({x, y}); 
    }
    bool isHovered(sf::Vector2f mousePos) { return text.getGlobalBounds().contains(mousePos); }
};

// --- BUTONLAR ---
struct Button {
    sf::Sprite sprite; 
    sf::Text label;    
    std::string id;
    sf::Vector2f baseScale; 
    sf::Vector2f m_targetSize; 

    Button(std::string _id, std::string _labelText, const sf::Texture& texture, sf::Font& font, sf::Vector2f position, sf::Vector2f targetSize) 
        : sprite(texture), label(font), m_targetSize(targetSize)
    {
        id = _id;
        sf::Vector2u texSize = texture.getSize();
        sprite.setOrigin({static_cast<float>(texSize.x) / 2.f, static_cast<float>(texSize.y) / 2.f});
        sprite.setPosition({position.x + targetSize.x / 2.f, position.y + targetSize.y / 2.f});
        float scaleX = targetSize.x / static_cast<float>(texSize.x);
        float scaleY = targetSize.y / static_cast<float>(texSize.y);
        baseScale = {scaleX, scaleY}; 
        sprite.setScale(baseScale);
        label.setString(_labelText);
        label.setCharacterSize(14); 
        label.setFillColor(BORDEAUX_COLOR);
        if (!_labelText.empty()) {
            sf::FloatRect textBounds = label.getLocalBounds();
            label.setOrigin({
                textBounds.position.x + textBounds.size.x / 2.0f, 
                textBounds.position.y + textBounds.size.y / 2.0f
            });
            label.setPosition(sprite.getPosition());
        }
    }

    void setStyle(bool active, const sf::Texture& tex) {
        sprite.setTexture(tex); 
        sf::Vector2u texSize = tex.getSize();
        sprite.setOrigin({static_cast<float>(texSize.x) / 2.f, static_cast<float>(texSize.y) / 2.f});
        float scaleX = m_targetSize.x / static_cast<float>(texSize.x);
        float scaleY = m_targetSize.y / static_cast<float>(texSize.y);
        baseScale = {scaleX, scaleY};
        sprite.setScale(baseScale);
        if (active) label.setFillColor(BORDEAUX_COLOR);
        else label.setFillColor(sf::Color(100, 100, 100)); 
    }

    void update(sf::Vector2f mousePos, bool isMousePressed) {
        if (sprite.getGlobalBounds().contains(mousePos)) {
            if (isMousePressed) sprite.setScale({baseScale.x * 0.85f, baseScale.y * 0.85f});
            else sprite.setScale({baseScale.x * 0.95f, baseScale.y * 0.95f});
        } else {
            sprite.setScale(baseScale);
        }
    }
    
    void setLabelText(std::string newText) {
        label.setString(newText);
        sf::FloatRect textBounds = label.getLocalBounds();
        label.setOrigin({
            textBounds.position.x + textBounds.size.x / 2.0f, 
            textBounds.position.y + textBounds.size.y / 2.0f
        });
        label.setPosition(sprite.getPosition());
    }

    bool isClicked(sf::Vector2f mousePos) {
        return sprite.getGlobalBounds().contains(mousePos);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
        if (label.getString().getSize() > 0) window.draw(label);
    }
};

// --- BUTON DURUMLARINI GUNCELLEME ---
void updateButtonStates(std::vector<Button>& buttons, GameState state, Room* room, 
                        const sf::Texture& normalTex, const sf::Texture& greyTex,
                        const sf::Texture& mapTex, const sf::Texture& mapGrayTex,
                        const sf::Texture& invTex, const sf::Texture& invGrayTex) {
    
    if (buttons.size() >= 6) {
        buttons[4].setStyle(true, mapTex); 
        buttons[5].setStyle(true, invTex); 
    }

    if (state == GameState::EXPLORING) {
        buttons[0].setLabelText("NORTH");
        buttons[1].setLabelText("WEST");
        buttons[2].setLabelText("EAST");
        buttons[3].setLabelText("SOUTH");
        if (room) {
            if (room->n == -1) buttons[0].setStyle(false, greyTex); else buttons[0].setStyle(true, normalTex);
            if (room->w == -1) buttons[1].setStyle(false, greyTex); else buttons[1].setStyle(true, normalTex);
            if (room->e == -1) buttons[2].setStyle(false, greyTex); else buttons[2].setStyle(true, normalTex);
            if (room->s == -1) buttons[3].setStyle(false, greyTex); else buttons[3].setStyle(true, normalTex);
        }
    } 
    else if (state == GameState::COMBAT) {
        for(int i=0; i<4; i++) { 
            buttons[i].setStyle(true, normalTex);
        }
        buttons[0].setLabelText("ATK");
        buttons[1].setLabelText("DEF");
        buttons[2].setLabelText("ITEM");
        buttons[3].setLabelText("RUN");
    }
    else if (state == GameState::DIALOGUE) {
        for(int i=0; i<4; i++) {
            buttons[i].setLabelText("");
            buttons[i].setStyle(false, greyTex);
        }
        if (buttons.size() >= 6) {
            buttons[4].setStyle(false, mapGrayTex);
            buttons[5].setStyle(false, invGrayTex);
        }
    }
    else if (state == GameState::SHOP) {
        for(int i=0; i<4; i++) buttons[i].setStyle(true, normalTex);
        buttons[0].setLabelText("BUY"); 
        buttons[1].setLabelText("SELL");
        buttons[2].setLabelText("TALK");
        buttons[3].setLabelText("EXIT");
        
        if (buttons.size() >= 6) {
            buttons[4].setStyle(false, mapGrayTex); 
            buttons[5].setStyle(false, invGrayTex);  
        }
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "RPG - Fixed Formatting & Delay", sf::State::Fullscreen);
    window.setFramerateLimit(60);
    window.setMouseCursorVisible(true);
    sf::View gameView(sf::FloatRect({0.f, 0.f}, {960.f, 540.f}));
    window.setView(gameView);

    GameState currentState = GameState::EXPLORING;
    bool isInventoryOpen = false; 
    bool isSellingMode = false; 

    Game game; 
    Typewriter typer; 

    sf::Font font;
    if (!font.openFromFile("font.ttf")) return -1;

    // --- TEXTURES ---
    sf::Texture dialogTexture; if (!dialogTexture.loadFromFile("textures/Textbox[Final].png")) return -1;
    sf::Texture statTexture; if (!statTexture.loadFromFile("textures/Statbox[Final].png")) return -1;
    sf::Texture buttonTexture; if (!buttonTexture.loadFromFile("textures/Button[Final].png")) return -1;
    
    sf::Texture inventoryTexture; if (!inventoryTexture.loadFromFile("textures/Inventory[Final].png")) return -1;
    sf::Texture inventoryGrayTexture; if (!inventoryGrayTexture.loadFromFile("textures/Inventory[Gray].png")) { inventoryGrayTexture = inventoryTexture; }
    
    sf::Texture mapTexture; if (!mapTexture.loadFromFile("textures/Map[Final].png")) { mapTexture = inventoryTexture; }
    sf::Texture mapGrayTexture; if (!mapGrayTexture.loadFromFile("textures/Map[Gray].png")) { mapGrayTexture = mapTexture; }

    sf::Texture buttonGreyTexture; if (!buttonGreyTexture.loadFromFile("textures/Button[Grey].png")) return -1;

    // --- LAYOUT ---
    float leftWidth = 560.f; 
    float rightWidth = 200.f; 
    float totalGameWidth = leftWidth + rightWidth; 
    float offsetX = (960.f - totalGameWidth) / 2.f; 
    float gameStartX = offsetX;
    float splitY = 380.f;

    sf::RectangleShape redPanel({leftWidth, splitY});
    redPanel.setPosition({gameStartX, 0.f}); 
    redPanel.setFillColor(sf::Color(20, 10, 10)); 

    sf::Sprite statSprite(statTexture);
    statSprite.setPosition({gameStartX + leftWidth, 0.f}); 
    float statScaleX = rightWidth / static_cast<float>(statTexture.getSize().x);
    float statScaleY = splitY / static_cast<float>(statTexture.getSize().y);
    statSprite.setScale({statScaleX, statScaleY});

    sf::Sprite dialogSprite(dialogTexture);
    dialogSprite.setPosition({gameStartX, splitY});
    float diaScaleX = leftWidth / static_cast<float>(dialogTexture.getSize().x);      
    float diaScaleY = (540.f - splitY) / static_cast<float>(dialogTexture.getSize().y); 
    dialogSprite.setScale({diaScaleX, diaScaleY});

    sf::RectangleShape inventoryBg({leftWidth, splitY}); 
    inventoryBg.setPosition({gameStartX, 0.f});
    inventoryBg.setFillColor(sf::Color(0, 0, 0, 210)); 

    std::vector<EnemyTarget> enemies; 
    std::vector<ItemTarget> groundItems; 
    std::vector<NPCTarget> npcs; 
    
    // --- COMBAT STATE VARIABLES ---
    std::vector<Monster*> activeMonsters; 
    bool isPlayerTurn = true;
    bool isTargetSelection = false;

    float centerX = gameStartX + (leftWidth / 2.f); 
    float centerY = splitY / 2.f;

    // --- BASLANGIC GUNCELLEMELERI ---
    updateEnemiesInView(enemies, game.getCurrentRoom(), centerX - 50.f, centerY - 50.f);
    
    groundItems.clear();
    if (game.getCurrentRoom() && game.getCurrentRoom()->itemID != -1) {
        groundItems.emplace_back("Loot", centerX, centerY + 50.f);
    }

    npcs.clear();
    NPC* initialNPC = game.getRoomNPC(); 
    if (initialNPC) {
        npcs.emplace_back(initialNPC->getName(), centerX + 100.f, centerY);
    }
    
    sf::Text statText(font);
    statText.setCharacterSize(16);
    statText.setFillColor(BORDEAUX_COLOR);
    statText.setPosition({statSprite.getPosition().x + 35.f, statSprite.getPosition().y + 40.f});
    updateStatText(statText, game.getPlayer());

    float textPadX = 47.f; 
    float textPadY = 30.f; 

    sf::Text npcText(font);
    npcText.setCharacterSize(16);
    npcText.setFillColor(BORDEAUX_COLOR); 
    npcText.setPosition({dialogSprite.getPosition().x + textPadX, dialogSprite.getPosition().y + textPadY}); 
    
    typer.start(wrapText(game.lookAtRoom(), 480, font, 16));

    // --- DIYALOG VE SHOP LISTELERI ---
    std::vector<VisualOption> dialogueOptions; 
    std::vector<VisualOption> shopOptions; 

    std::vector<Button> buttons;
    float colWidth = rightWidth / 2.f;       
    float areaHeight = 540.f - splitY;       
    float yellowH = areaHeight / 4.f;        
    float actionStartX = gameStartX + leftWidth;
    float actionStartY = splitY;

    for (int i = 0; i < 4; i++) {
        buttons.emplace_back(
            "BTN_" + std::to_string(i), 
            "", 
            buttonTexture,   
            font,            
            sf::Vector2f(actionStartX, actionStartY + (i * yellowH)),
            sf::Vector2f(colWidth, yellowH)
        );
    }
    
    updateButtonStates(buttons, currentState, game.getCurrentRoom(), 
                       buttonTexture, buttonGreyTexture, 
                       mapTexture, mapGrayTexture, 
                       inventoryTexture, inventoryGrayTexture);

    float purpleH = areaHeight / 2.f;        
    buttons.emplace_back("MAP", "", mapTexture, font, sf::Vector2f(actionStartX + colWidth, actionStartY), sf::Vector2f(colWidth, purpleH));
    buttons.emplace_back("INV", "", inventoryTexture, font, sf::Vector2f(actionStartX + colWidth, actionStartY + purpleH), sf::Vector2f(colWidth, purpleH));

    // --- (YENI) HOVER ZAMANLAYICISI ---
    sf::Clock hoverClock;
    int lastHoverIndex = -1; // Hangi itemin uzerindeydik?
    sf::Clock messageTimer;  // Mesajin ne kadar sure ekranda kalacagini takip eder

    while (window.isOpen()) {
        sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
        sf::Vector2f mousePos = window.mapPixelToCoords(pixelPos);
        bool isMousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Escape) window.close();
            }
            
            if (const auto* mousePress = event->getIf<sf::Event::MouseButtonPressed>()) {
                
                sf::Vector2f clickPosF = window.mapPixelToCoords(mousePress->position); 

                // --- 1. DAKTILO HIZLI GECME ---
                if (typer.isBusy() && !typer.isFinished) {
                    typer.finishInstant(); 
                    continue; 
                }

                // --- 2. ENVANTER ETKILESIMI (SATIS DAHIL) ---
                if (isInventoryOpen) {
                    float listStartX = gameStartX + 50.f;
                    float listStartY = 50.f + 40.f;
                    float lineHeight = 25.f;

                    if (clickPosF.x >= listStartX && clickPosF.x <= listStartX + 300.f) {
                        int clickedIndex = (int)((clickPosF.y - listStartY) / lineHeight);
                        if (clickedIndex >= 0 && clickedIndex < 10) {
                            
                            // A) SATIS MODU (SHOP)
                            if (currentState == GameState::SHOP && isSellingMode) {
                                if (clickedIndex < game.getPlayer().getInventory().size()) { 
                                    if (mousePress->button == sf::Mouse::Button::Left) {
                                        std::string result = game.sellShopItem(clickedIndex);
                                        typer.start(wrapText(result, 480, font, 16));
                                        updateStatText(statText, game.getPlayer());
                                        // Mesajin hemen kaybolmamasi icin timer'i resetle
                                        messageTimer.restart();
                                    }
                                }
                            }
                            // B) NORMAL MOD
                            else {
                                if (mousePress->button == sf::Mouse::Button::Left) {
                                    std::string result = game.playerUseItem(clickedIndex);
                                    if (!result.empty()) {
                                        typer.start(wrapText(result, 480, font, 16)); 
                                        updateStatText(statText, game.getPlayer()); 
                                        messageTimer.restart(); // Mesaj icin timer
                                    }
                                }
                                else if (mousePress->button == sf::Mouse::Button::Right) {
                                    std::string result = game.playerDropItem(clickedIndex);
                                    if (!result.empty()) {
                                        typer.start(wrapText(result, 480, font, 16));
                                        messageTimer.restart(); // Mesaj icin timer
                                    }
                                }
                            }
                        }
                    }
                } 
                
                // --- 3. DIYALOG SECENEK TIKLAMA ---
                if (currentState == GameState::DIALOGUE && !typer.isBusy()) {
                    for (const auto& opt : dialogueOptions) {
                        if (opt.text.getGlobalBounds().contains(clickPosF)) {
                            std::string response = game.selectDialogueOption(opt.index);
                            
                            if (response.empty()) {
                                currentState = GameState::EXPLORING;
                                typer.start(wrapText(game.lookAtRoom(), 480, font, 16));
                                updateButtonStates(buttons, currentState, game.getCurrentRoom(), 
                                                   buttonTexture, buttonGreyTexture, 
                                                   mapTexture, mapGrayTexture, 
                                                   inventoryTexture, inventoryGrayTexture);
                                dialogueOptions.clear(); 
                            } 
                            else if (response == "COMBAT_START") {
                                currentState = GameState::COMBAT;
                                typer.start("ENEMIES ATTACK!");
                                dialogueOptions.clear();
                                updateButtonStates(buttons, currentState, game.getCurrentRoom(), 
                                                   buttonTexture, buttonGreyTexture, 
                                                   mapTexture, mapGrayTexture, 
                                                   inventoryTexture, inventoryGrayTexture);
                            }
                            else if (response == "SHOP_OPEN") {
                                currentState = GameState::SHOP;
                                game.enterShop(); 
                                typer.start("Welcome to my shop! Take a look.");
                                dialogueOptions.clear();
                                shopOptions.clear();
                                updateButtonStates(buttons, currentState, game.getCurrentRoom(), 
                                                   buttonTexture, buttonGreyTexture, 
                                                   mapTexture, mapGrayTexture, 
                                                   inventoryTexture, inventoryGrayTexture);
                            }
                            else {
                                typer.start(wrapText(response, 480, font, 16));
                                dialogueOptions.clear(); 
                                updateStatText(statText, game.getPlayer());
                            }
                        }
                    }
                }

                // --- SHOP TIKLAMA (SATIN ALMA) ---
                if (currentState == GameState::SHOP && !typer.isBusy() && !isSellingMode) {
                    for (const auto& opt : shopOptions) {
                        if (opt.text.getGlobalBounds().contains(clickPosF)) {
                            std::string result = game.buyShopItem(opt.index);
                            typer.start(wrapText(result, 480, font, 16));
                            updateStatText(statText, game.getPlayer()); 
                            messageTimer.restart(); // Mesaj icin timer
                        }
                    }
                }

                // --- 4. DIGER ETKILESIMLER ---
                if (mousePress->button == sf::Mouse::Button::Left) {
                    
                    for (auto& btn : buttons) { 
                        if (btn.isClicked(clickPosF)) {
                            // INV
                            if (btn.id == "INV" && currentState == GameState::EXPLORING) {
                                isInventoryOpen = !isInventoryOpen; 
                                if (isInventoryOpen) typer.start("Inventory Opened.");
                                else typer.start(wrapText(game.lookAtRoom(), 480, font, 16));
                            }
                            // MAP
                            else if (btn.id == "MAP" && !isInventoryOpen && currentState != GameState::SHOP) {
                                typer.start("Map system active.");
                            }
                            // ACTIONS
                            else if (!isInventoryOpen || currentState == GameState::SHOP) {
                                if (currentState == GameState::EXPLORING) {
                                    Room* current = game.getCurrentRoom();
                                    std::string moveMsg = "";
                                    
                                    if (btn.id == "BTN_0") moveMsg = game.attemptMove(current->n);
                                    else if (btn.id == "BTN_1") moveMsg = game.attemptMove(current->w);
                                    else if (btn.id == "BTN_2") moveMsg = game.attemptMove(current->e);
                                    else if (btn.id == "BTN_3") moveMsg = game.attemptMove(current->s);

                                    if (!moveMsg.empty()) {
                                        typer.start(wrapText(moveMsg, 480, font, 16)); 
                                        updateEnemiesInView(enemies, game.getCurrentRoom(), centerX - 50.f, centerY - 50.f);
                                        
                                        // --- OTO-SAVAS KONTROLU ---
                                        if (!game.getCurrentRoom()->monsterID.empty()) {
                                            currentState = GameState::COMBAT;
                                            
                                            // 1. Savas degiskenlerini sifirla
                                            isPlayerTurn = true;
                                            isTargetSelection = false; 
                                            typer.start("Enemies ahead! Battle Start. (Your Turn)"); 
                                            
                                            // 2. Canli canavar nesnelerini olustur
                                            for(auto m : activeMonsters) delete m; 
                                            activeMonsters.clear();
                                            for(int mID : game.getCurrentRoom()->monsterID) {
                                                activeMonsters.push_back(game.getMonsterClone(mID));
                                            }

                                            // 3. Savas Modu: Esya ve NPC yok
                                            groundItems.clear();
                                            npcs.clear();
                                            
                                            updateButtonStates(buttons, currentState, game.getCurrentRoom(), 
                                                           buttonTexture, buttonGreyTexture, 
                                                           mapTexture, mapGrayTexture, 
                                                           inventoryTexture, inventoryGrayTexture);
                                        }
                                        else {
                                            // 1. Esyalari Yukle
                                            groundItems.clear();
                                            if (game.getCurrentRoom() && game.getCurrentRoom()->itemID != -1) {
                                                groundItems.emplace_back("Loot", centerX, centerY + 50.f);
                                            }
                                            
                                            // 2. NPC Yukle
                                            npcs.clear();
                                            NPC* movedNPC = game.getRoomNPC(); 
                                            if (movedNPC) {
                                                npcs.emplace_back(movedNPC->getName(), centerX + 100.f, centerY);
                                            }
                                            
                                            NPC* autoNPC = game.checkForAutoDialogue();
                                        if (autoNPC) {
                                            currentState = GameState::DIALOGUE;
                                            std::string startText = game.startDialogue(autoNPC);
                                            typer.start(wrapText(startText, 480, font, 16));
                                        }

                                        updateButtonStates(buttons, currentState, game.getCurrentRoom(), 
                                                           buttonTexture, buttonGreyTexture, 
                                                           mapTexture, mapGrayTexture, 
                                                           inventoryTexture, inventoryGrayTexture);
                                    }
                                }
                                else if (currentState == GameState::COMBAT) {
                                    if (btn.id == "BTN_0") {
                                        // ATK TUSU
                                        if (!isTargetSelection) { 
                                            isTargetSelection = true;
                                            typer.start("Select Target! (Click on Enemy)");
                                        }
                                    }
                                    else if (btn.id == "BTN_1") typer.start("Defend (Not impl)");
                                    else if (btn.id == "BTN_2") typer.start("Item (Not impl)");
                                    else if (btn.id == "BTN_3") typer.start("Run (Not impl)");
                                }
                                // SHOP BUTONLARI
                                else if (currentState == GameState::SHOP) {
                                    // BUY
                                    if (btn.id == "BTN_0") { 
                                        isSellingMode = false;
                                        isInventoryOpen = false; 
                                        typer.start("Buying Mode: Click on an item to buy.");
                                    }
                                    // SELL
                                    else if (btn.id == "BTN_1") { 
                                        isSellingMode = true;
                                        isInventoryOpen = true; 
                                        typer.start("Selling Mode: Click on an inventory item to sell.");
                                    }
                                    // TALK
                                    else if (btn.id == "BTN_2") { 
                                        game.exitShop();
                                        shopOptions.clear();
                                        isSellingMode = false;
                                        isInventoryOpen = false;
                                        
                                        NPC* currentNPC = game.getRoomNPC();
                                        if (currentNPC) {
                                            currentState = GameState::DIALOGUE;
                                            typer.start(wrapText(game.startDialogue(currentNPC), 480, font, 16));
                                            updateButtonStates(buttons, currentState, game.getCurrentRoom(), 
                                                               buttonTexture, buttonGreyTexture, 
                                                               mapTexture, mapGrayTexture, 
                                                               inventoryTexture, inventoryGrayTexture);
                                        }
                                    }
                                    // EXIT
                                    else if (btn.id == "BTN_3") { 
                                        game.exitShop();
                                        shopOptions.clear();
                                        isSellingMode = false;
                                        isInventoryOpen = false;
                                        
                                        currentState = GameState::EXPLORING;
                                        typer.start(wrapText(game.lookAtRoom(), 480, font, 16));
                                        updateButtonStates(buttons, currentState, game.getCurrentRoom(), 
                                                           buttonTexture, buttonGreyTexture, 
                                                           mapTexture, mapGrayTexture, 
                                                           inventoryTexture, inventoryGrayTexture);
                                    }
                                }
                            }
                        } 
                    }

                    // DUNYA ETKILESIMI (Sadece envanter kapaliyken)
                    // DUNYA ETKILESIMI (Sadece envanter kapaliyken)
                    if (!isInventoryOpen) {
                        
                        switch (currentState) {
                            case GameState::EXPLORING:
                            {
                                // KESIF MODU: Esya ve NPC etkilesimi
                                for (auto& item : groundItems) {
                                    if (item.isClicked(clickPosF)) {
                                        std::string msg = game.tryPickupItem();
                                        typer.start(msg);
                                        if (msg.find("Picked up") != std::string::npos) {
                                            groundItems.clear();
                                            updateStatText(statText, game.getPlayer());
                                        }
                                    }
                                }
                                for (auto& npc : npcs) {
                                    if (npc.isClicked(clickPosF)) {
                                        NPC* clickedNPC = game.getRoomNPC();
                                        if (clickedNPC) {
                                            currentState = GameState::DIALOGUE;
                                            std::string startText = game.startDialogue(clickedNPC);
                                            typer.start(wrapText(startText, 480, font, 16));
                                            updateButtonStates(buttons, currentState, game.getCurrentRoom(), 
                                                               buttonTexture, buttonGreyTexture, 
                                                               mapTexture, mapGrayTexture, 
                                                               inventoryTexture, inventoryGrayTexture);
                                        }
                                    }
                                }
                                break;
                            }

                            case GameState::COMBAT:
                            {
                                // SADECE TARGET SELECTION MODUNDAYSA DUSMANA TIKLANABILIR
                                // SADECE TARGET SELECTION MODUNDAYSA DUSMANA TIKLANABILIR
                                if (true) { // isTargetSelection bypass (TEST MODE)
                                    int enemyIndex = -1;
                                    for (size_t i = 0; i < enemies.size(); i++) { 
                                        if (enemies[i].isClicked(clickPosF)) {
                                            enemyIndex = i;
                                            break;
                                        }
                                    }
                                    
                                    if (enemyIndex != -1) {
                                        // SALDIRI ISLEMI
                                        Monster* targetMob = activeMonsters[enemyIndex];
                                        
                                        // TEST MODE: INSTANT KILL
                                        targetMob->takeDamage(9999); 
                                        std::string combatMsg = "INSTANT KILL (Test Mode)!";

                                        // Oldu mu?
                                        if (targetMob->isDead()) {
                                            combatMsg += "\n" + targetMob->getName() + " defeated!";
                                            
                                            // Loot (UI icin)
                                            combatMsg += "\n" + game.getCombatManager()->collectLootUI(&game.getPlayer(), targetMob, game.getItemManager());
                                            
                                            // Temizlik
                                            delete targetMob; 
                                            activeMonsters.erase(activeMonsters.begin() + enemyIndex); 
                                            
                                            // Data ve Gorsel Silme
                                            game.getCurrentRoom()->monsterID.erase(game.getCurrentRoom()->monsterID.begin() + enemyIndex);
                                            enemies.erase(enemies.begin() + enemyIndex); 

                                            updateEnemiesInView(enemies, game.getCurrentRoom(), centerX - 50.f, centerY - 50.f);

                                            if (activeMonsters.empty()) {
                                                combatMsg += "\nVICTORY!";
                                                currentState = GameState::EXPLORING;
                                                groundItems.clear();
                                                if (game.getCurrentRoom() && game.getCurrentRoom()->itemID != -1) {
                                                    groundItems.emplace_back("Loot", centerX, centerY + 50.f);
                                                }
                                                npcs.clear();
                                                NPC* movedNPC = game.getRoomNPC(); 
                                                if (movedNPC) {
                                                    npcs.emplace_back(movedNPC->getName(), centerX + 100.f, centerY);
                                                }
                                            } else {
                                                isTargetSelection = false; 
                                                isPlayerTurn = false; 
                                                combatMsg += "\n(Enemy Turn...)"; 
                                            }
                                        } 
                                        else {
                                            isTargetSelection = false; 
                                            // isPlayerTurn = false; 
                                        }
                                        
                                        typer.start(wrapText(combatMsg, 480, font, 16));
                                        updateStatText(statText, game.getPlayer());
                                        
                                        updateButtonStates(buttons, currentState, game.getCurrentRoom(), 
                                                    buttonTexture, buttonGreyTexture, 
                                                    mapTexture, mapGrayTexture, 
                                                    inventoryTexture, inventoryGrayTexture);
                                    }
                                }
                                break;
                            }
                            
                            default:
                                break;
                        }
                    }
                    }
                }
            }
        }

        // --- UPDATE ---
        for (auto& btn : buttons) btn.update(mousePos, isMousePressed);
        for (auto& item : groundItems) item.update(mousePos); 
        for (auto& npc : npcs) npc.update(mousePos); 
        
        typer.update(); 
        
        // --- DIYALOG SECENEKLERINI GUNCELLEME ---
        if (currentState == GameState::DIALOGUE && !typer.isBusy() && dialogueOptions.empty()) {
            std::vector<std::string> opts = game.getDialogueOptions();
            
            float startX = dialogSprite.getPosition().x + 50.f; 
            float startY = dialogSprite.getPosition().y + 68.f; 
            
            for (size_t i = 0; i < opts.size(); i++) {
                dialogueOptions.emplace_back(font, opts[i], i, startX, startY + (i * 15.f)); 
            }
        }

        // --- SHOP LISTESINI GUNCELLEME ---
        if (currentState == GameState::SHOP && !typer.isBusy() && shopOptions.empty() && !isSellingMode) {
            std::vector<std::string> items = game.getShopItems();
            
            float startX = dialogSprite.getPosition().x + 50.f; 
            float startY = dialogSprite.getPosition().y + 50.f; 
            
            for (size_t i = 0; i < items.size(); i++) {
                shopOptions.emplace_back(font, items[i], i, startX, startY + (i * 20.f)); 
            }
        }

        for (auto& opt : dialogueOptions) {
            if (opt.text.getGlobalBounds().contains(mousePos)) opt.text.setFillColor(sf::Color::Yellow); 
            else opt.text.setFillColor(sf::Color(200, 200, 200)); 
        }
        for (auto& opt : shopOptions) {
            if (opt.text.getGlobalBounds().contains(mousePos)) opt.text.setFillColor(sf::Color::Green); 
            else opt.text.setFillColor(sf::Color(200, 200, 200)); 
        }

        bool isHoveringItem = false;
        bool preventTyperFallback = false;
        
        // --- HOVER MANTIGI (DUZELTILDI) ---
        if (isInventoryOpen) { 
            float listStartX = gameStartX + 50.f;
            float listStartY = 50.f + 40.f;
            float lineHeight = 25.f;

            if (mousePos.x >= listStartX && mousePos.x <= listStartX + 300.f) {
                int hoverIndex = (int)((mousePos.y - listStartY) / lineHeight);
                
                // Hover indeksi degisti mi?
                if (hoverIndex != lastHoverIndex) {
                    // Eger onceden bir seyin uzerinde degilsek (yeni girdi) delay koy.
                    // Ama item'dan item'a geciyorsak (lastHoverIndex != -1) delay koyma, akici olsun.
                    if (lastHoverIndex == -1) {
                         hoverClock.restart(); 
                    }
                    lastHoverIndex = hoverIndex;
                }

                if (hoverIndex >= 0 && hoverIndex < 10) {
                    // Mesaj timer'i 2 saniyeyi gectiyse
                    if (messageTimer.getElapsedTime().asSeconds() > 2.0f) {
                        
                        // Hover suresi doldu mu?
                        if (hoverClock.getElapsedTime().asSeconds() > 0.2f) {
                            std::string desc = game.getItemDesc(hoverIndex);
                            if (!desc.empty()) {
                                if (isSellingMode && currentState == GameState::SHOP) {
                                    int val = game.getItemValue(hoverIndex);
                                    int sellPrice = (val * 3) / 5;
                                    desc += "\n\nSell Price: " + std::to_string(sellPrice) + " G";
                                }
                                
                                npcText.setString("Description:\n" + wrapText(desc, 480, font, 16)); 
                                isHoveringItem = true;
                            }
                        } else {
                            // Mesaj suresi doldu ama hover suresi dolmadi -> Eski mesaji gosterme, bosalt.
                            preventTyperFallback = true;
                        }
                    }
                }
            } else {
                lastHoverIndex = -1; 
            }
        }

        if (!isHoveringItem) {
            if (preventTyperFallback) npcText.setString("");
            else npcText.setString(typer.getCurrentText());
        }

        // --- DRAW ---
        window.clear(sf::Color::Black); 
        window.setView(gameView);

        window.draw(redPanel);
        for (const auto& enemy : enemies) window.draw(enemy.shape);
        if (currentState != GameState::COMBAT) {
            for (const auto& item : groundItems) window.draw(item.shape);
            for (const auto& npc : npcs) window.draw(npc.shape); 
        } 

        if (isInventoryOpen) {
            window.draw(inventoryBg);   
            drawInventory(window, font, game.getPlayer(), gameStartX + 50.f, 50.f, mousePos);
        }

        window.draw(statSprite);
        window.draw(statText); 
        window.draw(dialogSprite); 
        
        for (auto& btn : buttons) btn.draw(window);
        window.draw(npcText);
        
        if (currentState == GameState::DIALOGUE && !typer.isBusy()) {
            for (const auto& opt : dialogueOptions) {
                window.draw(opt.text);
            }
        }
        
        if (currentState == GameState::SHOP && !typer.isBusy() && !isSellingMode) {
            for (const auto& opt : shopOptions) {
                window.draw(opt.text);
            }
        }

        window.display();
    }
    return 0;
}