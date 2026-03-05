#pragma once
#include <string>

// === OYUN STATE'LERİ ===
// Bu enum hangi ekranın aktif olduğunu belirler.
// Her state butonların yazısını, rengini ve diyalog kutusu davranışını değiştirecek.
enum class GameState {
    EXPLORING,  // Normal oda keşfi (NORTH/WEST/EAST/SOUTH butonları)
    COMBAT,     // Savaş modu      (ATK/DEF/ITEM/RUN butonları)
    DIALOGUE,   // NPC konuşması   (diyalog seçenekleri)
    SHOP        // Alışveriş       (BUY/SELL/TALK/EXIT butonları)
};

// --- TEST YARDIMCISI ---
// Sol üst köşede hangi state'te olduğumuzu göstermek için
// GameState'i yazıya çeviren küçük yardımcı fonksiyon.
// İleride bu fonksiyon ve debug gösterimi SİLİNECEK.
inline std::string stateToString(GameState s) {
    switch (s) {
        case GameState::EXPLORING: return "STATE: EXPLORING";
        case GameState::COMBAT:    return "STATE: COMBAT";
        case GameState::DIALOGUE:  return "STATE: DIALOGUE";
        case GameState::SHOP:      return "STATE: SHOP";
        default:                   return "STATE: UNKNOWN";
    }
}
