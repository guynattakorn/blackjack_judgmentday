# 📋 Cheat Sheet — Blackjack: Judgment Day
### สำหรับการนำเสนอโปรเจกต์ FRA143

> **วิธีใช้:** ค้นหาชื่อ function ที่อาจารย์ถาม → อ่านคอลัมน์ที่ 3 ตอบ → อ่านคอลัมน์ที่ 4 อธิบายเทคนิค

---

## Section 1: C++ Game Engine (`game_logic.h` & `game_logic.cpp`)

---

### 📦 Class `Card`

| File & Class / Scope | Function / Variable | What it does (Thai) | Key Logic / Highlight |
|---|---|---|---|
| `game_logic.h` – `Card` | `suit_`, `rank_`, `value_`, `faceDown_` | เก็บข้อมูลของไพ่ 1 ใบ (ดอก, ตัว, แต้ม, หงายหรือคว่ำ) | `private` ทั้งหมด — เข้าถึงได้ผ่าน getter เท่านั้น (Encapsulation) |
| `game_logic.cpp` – `Card` | `Card(suit, rankInfo, faceDown)` | Constructor สร้างไพ่ 1 ใบ | กำหนด suit/rank/value จาก struct `RankInfo`; Ace มี `value_=11` ไว้ก่อน |
| `game_logic.cpp` – `Card` | `flip()` | กลับหน้าไพ่ (หงาย↔คว่ำ) | `faceDown_ = !faceDown_` — toggle boolean |
| `game_logic.cpp` – `Card` | `toJson()` | แปลงข้อมูลไพ่เป็น JSON string | ใช้ `ostringstream` สร้าง JSON ส่งให้ JS ฝั่ง React อ่านต่อ |
| `game_logic.h` – `Card` | `suitSymbols_[4]`, `suitCodes_[4]` | เก็บสัญลักษณ์และโค้ดของดอกไพ่ | `static const` — สร้างครั้งเดียวสำหรับทุก instance |

---

### 📦 Class `Hand`

| File & Class / Scope | Function / Variable | What it does (Thai) | Key Logic / Highlight |
|---|---|---|---|
| `game_logic.h` – `Hand` | `cards_` (vector\<Card\>) | เก็บไพ่ทุกใบบนมือ | `std::vector` — เพิ่ม/ลบ runtime ได้ **[CRIT-4]** |
| `game_logic.cpp` – `Hand` | `addCard(Card&&)` | เพิ่มไพ่เข้ามือ | `push_back(std::move(card))` — ย้าย Card object เข้า vector (move semantics) |
| `game_logic.cpp` – `Hand` | `score()` | คำนวณแต้มรวมของมือ รองรับ Ace 1/11 | `while(total > 21 && aces > 0){ total -= 10; aces--; }` — วน loop ลด Ace จาก 11→1 จนกว่าจะไม่ bust |
| `game_logic.cpp` – `Hand` | `isNaturalBlackjack()` | เช็คว่าได้ 21 จาก 2 ใบแรกพอดีหรือไม่ | `size()==2 && score()==21` — ต้องมีไพ่แค่ 2 ใบเท่านั้น |
| `game_logic.cpp` – `Hand` | `isBust()` | เช็คว่าแต้มเกิน 21 หรือไม่ | `return score() > 21` — เรียก score() ซึ่งจัดการ Ace ไปแล้ว |
| `game_logic.cpp` – `Hand` | `revealAll()` | เปิดไพ่ทุกใบ (ยกเลิกการคว่ำ) | วน `for` loop เช็ค `isFaceDown()` แล้วเรียก `flip()` ทีละใบ |
| `game_logic.cpp` – `Hand` | `toJson()` | แปลงมือไพ่ทั้งหมดเป็น JSON array | วน loop เรียก `card.toJson()` แล้ว concat ด้วย ostringstream |

---

### 📦 Class `Deck`

| File & Class / Scope | Function / Variable | What it does (Thai) | Key Logic / Highlight |
|---|---|---|---|
| `game_logic.h` – `Deck` | `cards_` (vector\<Card\>) | เก็บสำรับไพ่ทั้งหมด 52 ใบ | เพิ่ม/ลบ runtime **[CRIT-4]** |
| `game_logic.h` – `Deck` | `rng_` (mt19937) | ตัวสุ่มตัวเลขแบบ Mersenne Twister | `std::random_device{}()` เป็น seed — สุ่มได้คุณภาพสูง |
| `game_logic.cpp` – `Deck` | `build()` | สร้างสำรับไพ่ใหม่ 52 ใบ + สับ | วน 4 ดอก × 13 ตัว → `emplace_back` แล้วเรียก `shuffle()` |
| `game_logic.cpp` – `Deck` | `shuffle()` | สับไพ่แบบ random | `std::shuffle(begin, end, rng_)` — standard library, ไม่ bias |
| `game_logic.cpp` – `Deck` | `drawTop()` | จั่วไพ่ 1 ใบจากบนสุดของสำรับ | `pop_back()` ลบออก runtime **[CRIT-4]**; ถ้าสำรับหมดจะ `build()` ใหม่อัตโนมัติ |
| `game_logic.cpp` – `Deck` | `rebuild()` | รีเซ็ตสำรับไพ่ใหม่ | เรียก `build()` โดยตรง |

---

### 📦 Class `SkillInventory`

| File & Class / Scope | Function / Variable | What it does (Thai) | Key Logic / Highlight |
|---|---|---|---|
| `game_logic.h` – `SkillInventory` | `skills_` (vector\<Skill\>) | เก็บ Skill ที่ผู้เล่นมีทั้งหมด | Runtime collection **[CRIT-4]** |
| `game_logic.cpp` – `SkillInventory` | `makeSkill(SkillType)` | สร้าง Skill object จากประเภท | `switch(type)` → คืนค่า struct `Skill` พร้อม name/desc |
| `game_logic.cpp` – `SkillInventory` | `addSkill(SkillType)` | เพิ่ม Skill เข้า inventory | `push_back(makeSkill(type))` — เพิ่มเข้า vector runtime **[CRIT-4]** |
| `game_logic.cpp` – `SkillInventory` | `useSkill(index, outSkill)` | ใช้ Skill และลบมันออกจาก inventory | `erase(begin() + index)` — ลบออก runtime **[CRIT-4]**; คืน `false` ถ้า index ผิด |
| `game_logic.cpp` – `SkillInventory` | `toJson()` | แปลง skill list เป็น JSON | วน loop สร้าง JSON array ส่งให้ React แสดง skill ใน left panel |

---

### 📦 Abstract Base Class `Participant` [CRIT-2, CRIT-3]

| File & Class / Scope | Function / Variable | What it does (Thai) | Key Logic / Highlight |
|---|---|---|---|
| `game_logic.h` – `Participant` | `takeTurn()` *pure virtual* | บังคับให้ทุก derived class กำหนดวิธีเล่นของตัวเอง | `= 0` — **[CRIT-2]** Abstract; ไม่มี implementation ใน base class |
| `game_logic.h` – `Participant` | `role()` *pure virtual* | บังคับให้ระบุว่าเป็น "PLAYER" หรือ "JUDGE" | `= 0` — **[CRIT-2]** Abstract |
| `game_logic.h` – `Participant` | `deck_` (protected) | reference ไปยัง Deck ที่แชร์กัน | `protected` — ให้ derived class เข้าถึงได้แต่ภายนอกไม่ได้ |
| `game_logic.cpp` – `Participant` | `receiveCard(Card&&)` | รับไพ่มือไว้ในมือ | เรียก `hand_.addCard(move(card))` — delegate ไป Hand |
| `game_logic.cpp` – `Participant` | `getScore()` | ดูแต้มรวมของผู้เล่น/judge | เรียก `hand_.score()` ที่จัดการ Ace แล้ว |

---

### 📦 Class `Player : Participant` [CRIT-3]

| File & Class / Scope | Function / Variable | What it does (Thai) | Key Logic / Highlight |
|---|---|---|---|
| `game_logic.h` – `Player` | `credits_`, `bet_` | เครดิตและจำนวนเดิมพันปัจจุบัน | `private` — เข้าถึงผ่าน getter/setter |
| `game_logic.h` – `Player` | `shieldActive_` | สถานะว่าโล่ (Override Shield) กำลังใช้งานอยู่หรือไม่ | boolean flag; `true` = bust ครั้งแรกรอดไม่เสียเงิน |
| `game_logic.h` – `Player` | `consecutiveWins_` | จำนวนรอบที่ชนะติดต่อกัน | ใช้คำนวณ streak multiplier และรางวัล skill |
| `game_logic.cpp` – `Player` | `takeTurn()` | วิธีเล่นอัตโนมัติ: จั่วไพ่จนถึง 17 | `while(getScore() < 17) receiveCard(deck_.drawTop())` — Override [CRIT-3] |
| `game_logic.cpp` – `Player` | `placeBet(amount)` | ตั้งค่าเดิมพัน | `bet_ = amount` ตรงๆ; validation อยู่ใน `GameEngine::placeBet()` |
| `game_logic.cpp` – `Player` | `applyWin(amount)` | เพิ่มเครดิตเมื่อชนะ | `credits_ += amount` |
| `game_logic.cpp` – `Player` | `applyLoss(amount)` | ลดเครดิตเมื่อแพ้ | `credits_ -= amount; if(credits_ < 0) credits_ = 0` — ป้องกันติดลบ |
| `game_logic.cpp` – `Player` | `activateShield()` / `deactivateShield()` | เปิด/ปิดโล่ | set `shieldActive_` เป็น true/false |
| `game_logic.cpp` – `Player` | `incrementWinStreak()` / `resetWinStreak()` | เพิ่ม/รีเซ็ตสายชนะ | ใช้ตัดสินว่าได้รับ skill ฟรีและ multiplier หรือไม่ |

---

### 📦 Class `Judge : Participant` [CRIT-3]

| File & Class / Scope | Function / Variable | What it does (Thai) | Key Logic / Highlight |
|---|---|---|---|
| `game_logic.h` – `Judge` | `hp_` | HP ของ Judge (เริ่มต้น 2000) | ถ้า hp_ ≤ 0 → VICTORY; ถ้า hp_ ≤ 600 → Enrage mode |
| `game_logic.h` – `Judge` | `drawCallback_` | callback ที่เรียกทุกครั้งที่ Judge จั่วไพ่ | `std::function<void(const Card&)>` — optional, ใช้สำหรับ WASM bridge |
| `game_logic.cpp` – `Judge` | `takeTurn()` | Judge เปิดไพ่ที่คว่ำ แล้วจั่วจนถึง 17 | `revealAll()` ก่อน → `while(score() < 17) receiveCard(...)` — Override [CRIT-3] |
| `game_logic.cpp` – `Judge` | `applyDamage(dmg)` | ลด HP Judge เมื่อผู้เล่นชนะ | `hp_ -= dmg; if(hp_ < 0) hp_ = 0` — ไม่ติดลบ |
| `game_logic.cpp` – `Judge` | `applyHeal(amount)` | เพิ่ม HP Judge เมื่อผู้เล่นแพ้ (Life Steal) | `hp_ += amount; if(hp_ > JUDGE_START_HP) hp_ = JUDGE_START_HP` — ไม่เกิน cap |
| `game_logic.cpp` – `Judge` | `isEnraged()` | เช็คว่า Judge อยู่ใน Enrage mode หรือไม่ | `return hp_ <= ENRAGE_THRESHOLD && hp_ > 0` — min bet ขึ้นเป็น 50 |
| `game_logic.cpp` – `Judge` | `isDefeated()` | เช็คว่า Judge แพ้แล้วหรือยัง | `return hp_ <= 0` |

---

### 📦 Class `GameEngine`

| File & Class / Scope | Function / Variable | What it does (Thai) | Key Logic / Highlight |
|---|---|---|---|
| `game_logic.h` – `GameEngine` | `deck_`, `player_`, `judge_` | object หลักของเกม — สำรับ, ผู้เล่น, Judge | ทั้งหมดเป็น member ของ GameEngine; deck_ เป็น reference ร่วม |
| `game_logic.h` – `GameEngine` | `phase_` (enum Phase) | สถานะปัจจุบันของเกม | `BETTING → PLAYER_TURN → JUDGE_REVEAL → JUDGE_TURN → ROUND_OVER → ...` |
| `game_logic.h` – `GameEngine` | `shieldAbsorbedBust_` | flag ว่าโล่ช่วยผู้เล่นรอดจาก bust ไปแล้วหรือยัง | ป้องกัน infinite draw; รีเซ็ตทุกรอบใน `startNextRound()` |
| `game_logic.cpp` – `GameEngine` | `reset()` | รีเซ็ตเกมทั้งหมดเป็นค่าเริ่มต้น | สร้าง deck ใหม่, ตั้ง credits=500, HP=2000, ล้าง skill แล้วให้ skill เริ่มต้น 1 ชิ้น |
| `game_logic.cpp` – `GameEngine` | `startNextRound()` | เริ่มรอบใหม่โดยไม่รีเซ็ตเกม | ล้างมือไพ่, รีเซ็ต bet/shield, ตั้ง phase=BETTING; HP และ credits ยังคงเดิม |
| `game_logic.cpp` – `GameEngine` | `placeBet(amount)` | ตรวจสอบและตั้งค่าเดิมพัน | ตรวจ: phase==BETTING, amount ≥ effMin, amount ≤ credits ก่อนจึง `player_.placeBet()` |
| `game_logic.cpp` – `GameEngine` | `deal()` | แจกไพ่เริ่มต้น 4 ใบ (ผู้เล่น 2, Judge 2) | ใบที่ 4 ของ Judge: `hidden.flip()` → คว่ำ; phase เปลี่ยนเป็น PLAYER_TURN |
| `game_logic.cpp` – `GameEngine` | `playerHit()` | ผู้เล่นจั่วไพ่ 1 ใบ | ถ้า bust + shield ยังไม่ถูกใช้: ตั้ง `shieldAbsorbedBust_=true` (โล่ยังอยู่); ถ้า bust อีกครั้ง: ปิดโล่ → phase=JUDGE_REVEAL |
| `game_logic.cpp` – `GameEngine` | `playerStand()` | ผู้เล่นหยุดจั่ว | `phase_ = JUDGE_REVEAL` ทันที |
| `game_logic.cpp` – `GameEngine` | `runJudgeTurn()` | Judge เล่นและประเมินผล | เรียก `judge_.takeTurn()` แล้ว `evaluate()` คืน `EvaluationResult` |
| `game_logic.cpp` – `GameEngine` | `evaluate()` | ตัดสินผลแพ้ชนะของรอบ | `pBust = isBust() && !isShieldActive()` → ถ้า player bust = JUDGE_WIN; ถ้า judge bust = PLAYER_WIN; ไม่งั้นเปรียบแต้ม |
| `game_logic.cpp` – `GameEngine` | `streakMultiplier()` | คืนค่า multiplier ตาม win streak | streak ≥3 → 1.5×, ≥5 → 2×, ≥7 → 3×; ใช้คูณ damage ที่ deal ให้ Judge |
| `game_logic.cpp` – `GameEngine` | `useSkill(skillIndex)` | ใช้ skill ที่เลือก | เรียก `skills_.useSkill()` ลบออก runtime; แล้ว switch ตาม type: SCAN/DOUBLE/SHIELD |
| `game_logic.cpp` – `GameEngine` | `getStateJson()` | ส่งสถานะเกมทั้งหมดเป็น JSON string | รวมทุกอย่าง: phase, money, bet, judgeHp, playerHand, judgeHand, skills, message ฯลฯ |
| `game_logic.cpp` – `GameEngine` | `randomSkillType()` | สุ่มประเภท skill ที่จะให้เป็นรางวัล | `uniform_int_distribution<int>(0,2)` → cast เป็น `SkillType` |

---

### 📦 Struct `EvaluationResult`

| File & Class / Scope | Function / Variable | What it does (Thai) | Key Logic / Highlight |
|---|---|---|---|
| `game_logic.h` – `EvaluationResult` | `result` (RoundResult) | ผลของรอบ: PLAYER_WIN / JUDGE_WIN / DRAW | enum ที่ evaluate() กำหนด |
| `game_logic.h` – `EvaluationResult` | `creditDelta` | จำนวนเครดิตที่เปลี่ยนแปลง (+ชนะ / -แพ้) | ใช้แสดงใน result overlay |
| `game_logic.h` – `EvaluationResult` | `judgeHpDelta` | HP ที่ Judge เสีย (ค่าลบ) หรือฟื้น (ค่าบวก) | ลบ = damage; บวก = life steal กลับมา |
| `game_logic.h` – `EvaluationResult` | `critHit` | เป็น Natural Blackjack หรือเปล่า? | ถ้า true → damage ×1.5, แสดง CRITICAL STRIKE banner |
| `game_logic.h` – `EvaluationResult` | `streakMultiplier` | ค่า multiplier ตาม win streak | คูณกับ damage และ credit ที่ได้ |
| `game_logic.h` – `EvaluationResult` | `newSkillName` | ชื่อ skill ที่ได้รับรางวัลในรอบนี้ | ไม่ว่างถ้าชนะครบทุก 2 รอบ; แสดงใน result overlay |
| `game_logic.cpp` – `EvaluationResult` | `toJson()` | แปลงผลลัพธ์เป็น JSON ส่งให้ JS | ใช้โดย `bj_run_judge_turn()` ใน WASM bridge |

---

### 📦 WASM C-API Bridge (`extern "C"`)

| File & Class / Scope | Function / Variable | What it does (Thai) | Key Logic / Highlight |
|---|---|---|---|
| `game_logic.cpp` – extern "C" | `globalEngine` | instance เดียวของ GameEngine ทั้งเกม | `static GameEngine globalEngine` — singleton |
| `game_logic.cpp` – extern "C" | `bj_new_game()` | เริ่มเกมใหม่ | เรียก `globalEngine.reset()` |
| `game_logic.cpp` – extern "C" | `bj_get_state()` | ส่งสถานะเกมให้ JS | คืน `const char*` จาก `static std::string s` (ป้องกัน dangling pointer) |
| `game_logic.cpp` – extern "C" | `bj_place_bet(int)` | วางเดิมพัน | คืน 1 (สำเร็จ) หรือ 0 (fail) |
| `game_logic.cpp` – extern "C" | `bj_deal()` | แจกไพ่เริ่มรอบ | เรียก `globalEngine.deal()` |
| `game_logic.cpp` – extern "C" | `bj_player_hit()` | จั่วไพ่ 1 ใบ | คืน 1/0 ตามสำเร็จ |
| `game_logic.cpp` – extern "C" | `bj_player_stand()` | หยุดจั่ว | เรียก `globalEngine.playerStand()` |
| `game_logic.cpp` – extern "C" | `bj_use_skill(int)` | ใช้ skill ตาม index | คืน string ข้อความ feedback |
| `game_logic.cpp` – extern "C" | `bj_run_judge_turn()` | Judge เล่น + ตัดสินผล | คืน JSON `EvaluationResult` |
| `game_logic.cpp` – extern "C" | `bj_next_round()` | เริ่มรอบถัดไป | เรียก `globalEngine.startNextRound()` |

---

## Section 2: React UI & WASM Bridge (`blackjack_wasm.html`)

---

### 🌉 Object `GE` — WASM Bridge

| File & Class / Scope | Function / Variable | What it does (Thai) | Key Logic / Highlight |
|---|---|---|---|
| `blackjack_wasm.html` – GE | `GE.newGame()` | เรียก C++ reset เกม | `Module.ccall("bj_new_game", null, [], [])` |
| `blackjack_wasm.html` – GE | `GE.getState()` | ดึง state ทั้งหมดจาก C++ เป็น JS object | `JSON.parse(Module.ccall("bj_get_state", "string", [], []))` |
| `blackjack_wasm.html` – GE | `GE.placeBet(amount)` | ส่งค่าเดิมพันไป C++ | `ccall` พร้อม `["number"]` type |
| `blackjack_wasm.html` – GE | `GE.playerHit()` | บอก C++ ให้จั่วไพ่ | คืน number (0/1) |
| `blackjack_wasm.html` – GE | `GE.useSkill(i)` | บอก C++ ใช้ skill index i | คืน string ข้อความ |
| `blackjack_wasm.html` – GE | `GE.runJudgeTurn()` | สั่ง Judge เล่น + ตัดสินผล | คืน JSON object ที่ `JSON.parse` แล้ว |

---

### ⚛️ React State ที่สำคัญ

| File & Class / Scope | Function / Variable | What it does (Thai) | Key Logic / Highlight |
|---|---|---|---|
| `App()` | `gs` | สถานะเกมทั้งหมดที่ได้จาก C++ | อัปเดตด้วย `pull()` ทุกครั้งที่มีการเปลี่ยนแปลง |
| `App()` | `busy` (useRef) | ป้องกันผู้เล่นกดซ้ำระหว่าง animation | `busy.current = true` ตอนเริ่ม action; `= false` ตอนจบ |
| `App()` | `playerScore` | แต้มผู้เล่นที่แสดงบนหน้าจอ (live) | อัปเดตหลังทุก HIT; ถูก reset เป็น 0 ตอน stand/bust |
| `App()` | `judgeRevealScore` | แต้ม Judge ที่แสดงระหว่าง animation | `null` = ใช้ค่าจาก gs ปกติ; มีค่า = กำลัง animate ทีละใบ |
| `App()` | `showResult` | แสดง/ซ่อน result overlay หลังจบรอบ | `true` หลัง `runJudge()` เสร็จ |
| `App()` | `tension` | ความตึงเครียด (0-100) แสดงเป็น tension bar | เพิ่มขึ้นเรื่อยๆ ระหว่าง judge sequence |
| `App()` | `betAmount` | จำนวนเดิมพันที่ผู้เล่นตั้งไว้ (ใน JS) | ไม่ใช่ค่าเดียวกับ `gs.bet` จาก C++; เป็นแค่ UI state |
| `App()` | `timers` (useRef) | เก็บ timer ID ทุกอัน | ใช้ `forEach(clearTimeout)` ตอน restart เพื่อหยุด animation ทั้งหมด |
| `App()` | `showIntro` | แสดง/ซ่อนหน้า intro/main menu | `true` ตอนเปิดเกมและหลัง game over/victory |
| `App()` | `anims` | เก็บ animation class ของไพ่แต่ละใบ | key เป็น `"p-0"`, `"j-1"` ฯลฯ; `addAnim()` ตั้งแล้วลบหลัง 700ms |

---

### ⚙️ Key Functions — Game Flow

| File & Class / Scope | Function / Variable | What it does (Thai) | Key Logic / Highlight |
|---|---|---|---|
| `App()` | `pull()` | ดึงสถานะล่าสุดจาก C++ มาอัปเดต React | `const s = GE.getState(); setGs(s); return s;` — เรียกหลังทุก action |
| `App()` | `calcScore(hand)` | คำนวณแต้มจาก hand array รองรับ Ace 1/11 | `while(total > 21 && aces > 0){ total -= 10; }` — เหมือนฝั่ง C++ แต่ใน JS |
| `App()` | `confirmBet()` | ยืนยันเดิมพันและเริ่มแจกไพ่ | `async`; เรียก `GE.placeBet()` → `GE.deal()` → `pull()` → animate ทีละใบด้วย `sleep()` |
| `App()` | `playerHit()` | จั่วไพ่ + animate + เช็ค bust | `async`; guard `busy.current`; ถ้า `s.phase==="JUDGE_REVEAL"` → bust → เรียก `runJudge()` |
| `App()` | `playerStand()` | หยุดจั่วและส่งต่อ Judge | `async`; เรียก `GE.playerStand()` → `pull()` → `runJudge()` |
| `App()` | `runJudge(prevS)` | sequence เปิดไพ่ Judge ทีละใบพร้อม animation | `async`; 3 ขั้น: (1) เปิดไพ่คว่ำ (2) จั่วไพ่ animate ทีละใบ (3) แสดงผล; ใช้ `sleep()` สร้าง delay |
| `App()` | `useSkill(i)` | ใช้ skill index i | guard `busy`; เรียก `GE.useSkill(i)` → `pull()`; มี busy guard ป้องกันคลิกซ้ำ |
| `App()` | `nextRound()` | เริ่มรอบใหม่ | เรียก `GE.nextRound()` → reset state ใน React → clamp `betAmount` ตาม enrage |
| `App()` | `restart()` | รีเซ็ตเกมทั้งหมด + กลับ main menu | ล้าง timer ทั้งหมด, `GE.newGame()`, `setShowIntro(true)`, `setIntroPage(0)` |
| `App()` | `adjBet(delta)` | ปรับจำนวน bet ขึ้น/ลง | clamp ด้วย `Math.max(effMin, Math.min(gs.money, betAmount+delta))` |
| `App()` | `sleep(ms)` | delay แบบ async | `new Promise(r => setTimeout(r, ms))` — เก็บ timer ใน `timers.current` |

---

### ⚙️ Key Functions — Sound & Visual Effects

| File & Class / Scope | Function / Variable | What it does (Thai) | Key Logic / Highlight |
|---|---|---|---|
| `blackjack_wasm.html` – SFX | `tone(f, d, t, v, dl)` | สร้างเสียงจาก Web Audio API | สร้าง OscillatorNode + GainNode; `exponentialRampToValueAtTime` ทำให้เสียงค่อยๆ หาย |
| `blackjack_wasm.html` – SFX | `SFX.card()`, `SFX.bust()`, `SFX.win()` ฯลฯ | เสียง effect แต่ละแบบ | เรียก `tone()` ด้วย frequency/duration ต่างกัน |
| `App()` | `trigFlash(type)` | กะพริบหน้าจอสีต่างๆ | `setFlash(type)` → CSS animation → `setTimeout` clear หลัง 600ms |
| `App()` | `trigShake(big)` | สั่นหน้าจอ | `setShake("shake"/"shakeSm")` → CSS animation |
| `App()` | `addAnim(key, cls)` | กำหนด animation class ให้ไพ่ | `setAnims(prev => ({...prev, [key]: cls}))` → clear หลัง 700ms |
| `blackjack_wasm.html` – Particles | `Particles({ enrage })` | แสดง particle animation พื้นหลัง | `useRef` + `requestAnimationFrame` loop; enrage เปลี่ยนสีและความถี่ particle |

---

### 🃏 UI Sub-Components

| File & Class / Scope | Function / Variable | What it does (Thai) | Key Logic / Highlight |
|---|---|---|---|
| `CardView` | `card.faceDown` | แสดงไพ่คว่ำหรือหงาย | ถ้า `faceDown=true` → render `cback` pattern; ไม่งั้น render หน้าไพ่ปกติ |
| `HpBar` | `money / STARTING_CREDITS` | แสดง HP bar ของผู้เล่น (credit %) | `pct > 50 → "full"`, `> 20 → "med"`, ไม่งั้น `"low"` (สีแดงกะพริบ) |
| `JudgeHpBar` | `judgeHp / JUDGE_MAX_HP` | แสดง HP bar ของ Judge พร้อม milestone | มี marker ที่ ENRAGE (600) และ HALF (1000); `pct < 30 → "low"` (สีแดงเข้ม) |
| `StreakPips` | `wins % 2` | แสดง 2 pip สำหรับ win streak | ทุก 2 consecutive wins → pip เต็ม → ได้ skill; class `"bonus"` ถ้าพอดี milestone |
| `RiskMeter` | `bet / money` | แสดงความเสี่ยงของเดิมพัน | `pct < 30 → LOW`, `< 60 → MED`, ไม่งั้น `HIGH`; แสดงเป็น % ของ credits |

---

### 🔑 OOP Criteria Mapping (สรุปสำหรับตอบอาจารย์)

| Criteria | ตัวอย่างในโค้ด | ไฟล์ |
|---|---|---|
| **[CRIT-1] Encapsulation** | ทุก member ใน Card, Hand, Player, Judge เป็น `private`; เข้าถึงผ่าน getter/setter เท่านั้น | `game_logic.h` |
| **[CRIT-2] Abstract Base Class** | `Participant` มี `takeTurn()` และ `role()` เป็น pure virtual (`= 0`) | `game_logic.h §6` |
| **[CRIT-3] Inheritance ×2** | `Player : Participant` override `takeTurn()` (จั่วถึง 17); `Judge : Participant` override `takeTurn()` (เปิดไพ่ + จั่วถึง 17) | `game_logic.h §7,8` |
| **[CRIT-4] Runtime State Modify** | `Hand::addCard` (push_back), `Deck::drawTop` (pop_back), `SkillInventory::useSkill` (erase) | `game_logic.cpp` |
