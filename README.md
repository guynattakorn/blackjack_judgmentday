# BLACKJACK: JUDGMENT DAY

**FRA143 Software Development for Robotics and Automation Engineering — FIBO KMUTT**
ก้องภพ 68340500003 · จิรายุ 68340500013 · ณฐกร 68340500062 · ภูริต 68340500086 


---

## Demo Video อัพเดตครับ
https://youtu.be/o4GBMYtoOW8

---

## การใช้ Generative AI

โปรเจกต์นี้ใช้ **Claude (Anthropic)** และ **Gemini** เป็น AI assistant ในกระบวนการพัฒนา ตามนโยบายที่อนุญาตของรายวิชา

---

## ภาพรวมโปรเจกต์

Blackjack สไตล์ Boss-Fight ที่ผู้เล่นต้องเอาชนะ **THE JUDGE** ด้วยการสะสม Win Streak, ใช้ Skills พิเศษ และจัดการเงินเดิมพันให้ชาญฉลาด Logic ทั้งหมดเขียนด้วย C++ และ compile เป็น WebAssembly (WASM) ผ่าน Emscripten — หน้า HTML เป็นเพียง UI ที่เรียกใช้ C-API ของ C++ โดยตรง ไม่มี game logic ใดอยู่ใน JavaScript เลย

---

## โครงสร้างไฟล์

```
.
├── game_logic.h          ← C++ Header — OOP class declarations ทั้งหมด
├── game_logic.cpp        ← C++ Implementation + WASM C-API Bridge (extern "C")
├── blackjack_wasm.html   ← Frontend (React/JSX via Babel) — UI เท่านั้น ไม่มี game logic
├── requirements.md       ← Functional & Non-Functional Requirements
└── README.md             ← ไฟล์นี้
```

> หลังจาก build แล้วจะได้ไฟล์เพิ่ม: `blackjack.js` และ `blackjack.wasm`

---

## OOP Design

```
Participant   (Abstract Base Class)                    [CRIT-2]
│  # name_, hand_, deck_       protected / private
│  + takeTurn() = 0            pure virtual
│  + role() const = 0          pure virtual
│
├── Player  : Participant      [CRIT-3 — Derived Class 1]
│    + takeTurn()  → auto-stand ที่ score 17
│    + role()      → "PLAYER"
│    - credits_, bet_, shieldActive_, consecutiveWins_, skills_
│
└── Judge   : Participant      [CRIT-3 — Derived Class 2]
     + takeTurn()  → เปิดไพ่คว่ำ แล้ว draw จนถึง score ≥ 17
     + role()      → "JUDGE"
     - hp_, drawCallback_
```

### Supporting Classes

| Class             | หน้าที่ |
|------------------|---------|
| `Card`            | ข้อมูลไพ่ (suit, rank, value, faceDown) — immutable หลัง construct |
| `Hand`            | Collection ของ `Card` — `addCard` (push) / `clear` |
| `Deck`            | สำรับ 52 ใบ — `drawTop()` ดึงและลบออกจาก vector |
| `SkillInventory`  | กระเป๋า Skill ของผู้เล่น — `addSkill` (push) / `useSkill` (erase) |
| `GameEngine`      | Orchestrator — จัดการ flow ทั้งหมด, expose C-API ให้ WASM |

### Runtime State Modification (CRIT-4)

| ตำแหน่ง | Operation | Collection |
|--------|-----------|------------|
| `Deck::drawTop()` | `pop_back` — ลบไพ่ออกจากสำรับ | `std::vector<Card>` |
| `Hand::addCard()` | `push_back` — เพิ่มไพ่เข้ามือ | `std::vector<Card>` |
| `SkillInventory::addSkill()` | `push_back` — รับ Skill หลังชนะ 2 ครั้ง | `std::vector<Skill>` |
| `SkillInventory::useSkill()` | `erase` — ลบ Skill ที่ใช้แล้วออก | `std::vector<Skill>` |

---

## วิธี Build WASM

ต้องติดตั้ง [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) ก่อน จากนั้นรัน:

```bash
emcc game_logic.cpp -o blackjack.js \
  -s EXPORTED_FUNCTIONS="['_bj_new_game','_bj_get_state','_bj_place_bet',\
'_bj_cancel_bet','_bj_deal','_bj_player_hit','_bj_player_stand',\
'_bj_use_skill','_bj_run_judge_turn']" \
  -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap']" \
  -std=c++17 -O3
```

ผลลัพธ์: `blackjack.js` + `blackjack.wasm` ในโฟลเดอร์เดียวกัน

### วิธีเปิดเกม

```bash
# วิธีที่ 1 — Python HTTP server (แนะนำ เพราะ browser บล็อก WASM ที่โหลดจาก file://)
python3 -m http.server 8080
# แล้วเปิด http://localhost:8080/blackjack_wasm.html

# วิธีที่ 2 — VS Code Live Server extension ก็ได้เหมือนกัน
```

---

## กฎและ Mechanics

### ค่าเริ่มต้น

| รายการ | ค่า |
|--------|-----|
| Starting Credits | ¢500 |
| Judge HP | ¢2,000 |
| Min Bet | ¢10 |
| Enrage Threshold | Judge HP ≤ ¢600 (30%) |
| Min Bet ระหว่าง Enrage | ¢50 |

### Win Streak Multiplier

| Streak | Multiplier |
|--------|-----------|
| 1–2 wins | ×1.0 |
| 3–4 wins | ×1.5 |
| 5–6 wins | ×2.0 |
| 7+ wins  | ×3.0 |

### Skills

| Skill | Effect |
|-------|--------|
| **SCAN PROTOCOL** | เปิดไพ่คว่ำของ Judge ทันที |
| **DOUBLE RISK** | เพิ่มเดิมพันปัจจุบัน ×2 รอบนี้ |
| **OVERRIDE SHIELD** | ป้องกัน Bust ได้ 1 ครั้งในรอบนี้ |

ผู้เล่นได้รับ Skill แบบสุ่ม 1 ชิ้นตอนเริ่มเกม และได้รับเพิ่มทุก 2 ชัยชนะติดต่อกัน Skill ที่ใช้แล้วจะถูกลบออกจาก Inventory ทันที

### เงื่อนไขชนะ/แพ้

- **VICTORY** — Judge HP ลดเหลือ 0
- **GAME OVER** — Credits ของผู้เล่นลดเหลือ 0
- ผู้เล่นสามารถ restart ได้จากทั้งสองหน้า

---

## Keyboard Controls

| ปุ่ม | Action |
|------|--------|
| `H` | Hit (รับไพ่เพิ่ม) |
| `S` | Stand (หยุด) |
| `ENTER` | วางเดิมพัน / ยืนยัน / ต่อรอบถัดไป |
| `ESC` | ยกเลิกการยืนยันเดิมพัน |
| `↑` / `↓` | ปรับจำนวนเดิมพัน +10 / −10 |
| `R` | Restart (บนหน้า Game Over / Victory) |

---

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                  blackjack_wasm.html                │
│  React UI  ──────────────────────────────────────  │
│  (rendering only — zero game logic)                 │
│       │  Module.ccall(...)  ▲  JSON state           │
└───────┼─────────────────────┼───────────────────────┘
        │  WASM C-API Bridge  │
        ▼                     │
┌─────────────────────────────────────────────────────┐
│              game_logic.cpp / .wasm                 │
│  GameEngine  →  Player / Judge / Deck / Hand        │
│  (all logic: betting, dealing, evaluation, skills)  │
└─────────────────────────────────────────────────────┘
```

HTML เรียก C-function ผ่าน `Module.ccall()` แล้วรับ state กลับมาเป็น JSON string จาก `bj_get_state()` — React render จาก JSON นั้นโดยตรง ไม่มี state ซ้ำซ้อนระหว่าง JS และ C++

### C-API Functions

| Function | คืนค่า | หน้าที่ |
|----------|--------|---------|
| `bj_new_game()` | void | Reset เกมทั้งหมด |
| `bj_get_state()` | `const char*` (JSON) | ดึง state ปัจจุบันทั้งหมด |
| `bj_place_bet(int)` | int (0/1) | วางเดิมพัน, คืน 1 ถ้าสำเร็จ |
| `bj_cancel_bet()` | void | ยกเลิกเดิมพัน |
| `bj_deal()` | void | แจกไพ่เริ่มรอบ |
| `bj_player_hit()` | int (0/1) | ผู้เล่นรับไพ่ |
| `bj_player_stand()` | void | ผู้เล่นหยุด |
| `bj_use_skill(int)` | `const char*` | ใช้ Skill ตาม index |
| `bj_run_judge_turn()` | `const char*` (JSON) | Judge เล่น + evaluate รอบ |

---

## OOP Criteria Checklist

| เกณฑ์ | รายละเอียด | ไฟล์ |
|-------|-----------|------|
| ✅ Class + Encapsulation | ทุก class มี `private` members และ `public` accessors เท่านั้น | `game_logic.h` |
| ✅ Abstract Base Class | `Participant` มี `takeTurn()` และ `role()` เป็น pure virtual | `game_logic.h §6` |
| ✅ Inheritance ×2 | `Player : Participant` และ `Judge : Participant` | `game_logic.h §7, §8` |
| ✅ Runtime State Modify | `Deck::drawTop` (pop), `Hand::addCard` (push), `SkillInventory::useSkill` (erase) | `game_logic.cpp` |

---
