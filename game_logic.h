/**
 * ============================================================================
 * game_logic.h — FRA143 Blackjack: Judgment Day
 * OOP Design: Abstract Base Class + Derived Classes (Inheritance / Encapsulation)
 * ============================================================================
 *
 * Class Hierarchy:
 *
 *   Participant  (Abstract Base Class — §6)
 *   ├── Player   (Derived Class 1    — §7)
 *   └── Judge    (Derived Class 2    — §8)
 *
 * Supporting Classes:
 *   Card, Hand, Deck, SkillInventory, EvaluationResult, GameEngine
 *
 * OOP Criteria Mapping:
 *   [CRIT-1] Class / Encapsulation  : All member variables are private/protected;
 *                                     access only through public methods.
 *   [CRIT-2] Abstract Base Class    : Participant has pure-virtual takeTurn() & role().
 *   [CRIT-3] Inheritance ×2         : Player : Participant, Judge : Participant.
 *   [CRIT-4] Runtime state modify   : Hand::addCard (push_back), SkillInventory::useSkill
 *                                     (erase), Deck::drawTop (pop_back) — objects are
 *                                     added/removed from collections at runtime.
 * ============================================================================
 */

#pragma once
#include <string>
#include <vector>
#include <functional>
#include <random>
#include <sstream>
#include <algorithm>
#include <stdexcept>

// ── Game constants ──────────────────────────────────────────────────────────
static constexpr int STARTING_CREDITS  = 500;
static constexpr int JUDGE_START_HP    = 2000;
static constexpr int ENRAGE_THRESHOLD  = 600;   // 30 % of 2000
static constexpr int MIN_BET           = 10;
static constexpr int ENRAGE_MIN_BET    = 50;

// ============================================================================
// §1  Enumerations
// ============================================================================
enum class Suit { SPADE = 0, HEART, DIAMOND, CLUB };

enum class SkillType { SCAN_PROTOCOL = 0, DOUBLE_RISK, OVERRIDE_SHIELD };

enum class RoundResult { NONE, PLAYER_WIN, JUDGE_WIN, DRAW };

enum class Phase {
    BETTING, DEALING, PLAYER_TURN,
    JUDGE_REVEAL, JUDGE_TURN, ROUND_OVER,
    GAME_OVER, VICTORY
};

// ============================================================================
// §2  Card  [CRIT-1: Encapsulation]
// ============================================================================
struct RankInfo { std::string label; int baseValue; };

class Card {
public:
    Card(Suit suit, const RankInfo& rankInfo, bool faceDown = false);

    // Accessors (Encapsulation — no public member variables)
    Suit        getSuit()       const;
    std::string getSuitSymbol() const;
    std::string getSuitCode()   const;
    std::string getRank()       const;
    int         getValue()      const;
    bool        isFaceDown()    const;
    void        flip();

    std::string toJson() const;

private:
    Suit        suit_;
    std::string rank_;
    int         value_;
    bool        faceDown_;

    static const std::string suitSymbols_[4];
    static const std::string suitCodes_[4];
};

// ============================================================================
// §3  Hand  — runtime collection of Card objects  [CRIT-4]
// ============================================================================
class Hand {
public:
    // [CRIT-4] addCard: push Card into runtime std::vector collection
    void addCard(Card&& card);
    void clear();

    int         score()              const;
    bool        isNaturalBlackjack() const;
    bool        isBust()             const;
    int         size()               const;
    const Card& cardAt(int index)    const;
    void        revealAll();

    std::string toJson() const;

private:
    std::vector<Card> cards_;   // [CRIT-4] runtime-modified collection
};

// ============================================================================
// §4  Deck  [CRIT-1: Encapsulation]
// ============================================================================
class Deck {
public:
    Deck();

    // [CRIT-4] drawTop: physically removes Card from vector at runtime
    Card drawTop();
    int  remaining() const;
    bool isEmpty()   const;
    void rebuild();

private:
    std::vector<Card>          cards_;
    std::mt19937               rng_;

    void build();
    void shuffle();
};

// ============================================================================
// §5  Skill & SkillInventory  [CRIT-4: runtime add/remove]
// ============================================================================
struct Skill {
    SkillType   type;
    std::string name;
    std::string desc;
};

class SkillInventory {
public:
    // [CRIT-4] addSkill: push Skill into runtime collection
    void addSkill(SkillType type);

    // [CRIT-4] useSkill: erase Skill from runtime collection
    bool useSkill(int index, Skill& outSkill);

    const Skill& getSkill(int index) const;
    int          count()             const;
    std::string  toJson()            const;

    // Static factory — also used by GameEngine for reward names
    static Skill makeSkill(SkillType t);

private:
    std::vector<Skill> skills_;   // [CRIT-4] runtime-modified collection
};

// ============================================================================
// §6  Participant — Abstract Base Class  [CRIT-2, CRIT-3]
// ============================================================================
class Participant {
public:
    explicit Participant(const std::string& name, Deck& deck);
    virtual ~Participant() = default;

    // ── Pure virtual interface (must be overridden by derived classes) ──────
    virtual void        takeTurn() = 0;   // [CRIT-2] pure virtual
    virtual std::string role()     const = 0;  // [CRIT-2] pure virtual

    // ── Non-virtual shared behaviour (Encapsulation) ─────────────────────
    Hand&       getHand();
    const Hand& getHand()    const;
    void        receiveCard(Card&& card);
    void        clearHand();
    int         getScore()   const;
    bool        isBust()     const;
    std::string getName()    const;

protected:
    // Protected so derived classes can access deck for their turn logic
    Deck&       deck_;    // reference — shared single deck

private:
    std::string name_;
    Hand        hand_;
};

// ============================================================================
// §7  Player : Participant  (Derived Class 1)  [CRIT-3]
// ============================================================================
class Player : public Participant {
public:
    explicit Player(Deck& deck);

    // ── Override pure virtual ───────────────────────────────────────────────
    void        takeTurn() override;          // basic auto-stand at 17
    std::string role()     const override;    // returns "PLAYER"

    // ── Player-specific public interface ───────────────────────────────────
    int  getCredits()        const;
    void setCredits(int c);
    bool isEliminated()      const;

    int  getCurrentBet()     const;
    void placeBet(int amount);
    void resetBet();

    void applyWin(int amount);
    void applyLoss(int amount);

    bool isShieldActive()    const;
    void activateShield();
    void deactivateShield();

    int  getConsecutiveWins()  const;
    void incrementWinStreak();
    void resetWinStreak();

    SkillInventory&       getSkills();
    const SkillInventory& getSkillsConst() const;

    std::string toJson() const;

private:
    int            credits_;
    int            bet_;
    bool           shieldActive_;
    int            consecutiveWins_;
    SkillInventory skills_;    // [CRIT-4] SkillInventory is itself a runtime collection
};

// ============================================================================
// §8  Judge : Participant  (Derived Class 2)  [CRIT-3]
// ============================================================================
class Judge : public Participant {
public:
    explicit Judge(Deck& deck);

    // ── Override pure virtual ───────────────────────────────────────────────
    void        takeTurn() override;          // draws until score ≥ 17, invokes callback
    std::string role()     const override;    // returns "JUDGE"

    // ── Judge-specific public interface ────────────────────────────────────
    int  getHp()           const;
    void applyDamage(int dmg);
    void applyHeal(int amount);
    void resetHp();             // resets HP to JUDGE_START_HP
    bool isDefeated()      const;
    bool isEnraged()       const;   // HP ≤ ENRAGE_THRESHOLD

    // Optional callback invoked each time Judge draws a card (for JS bridge)
    void setDrawCallback(std::function<void(const Card&)> cb);

    std::string toJson() const;

private:
    int                              hp_;
    std::function<void(const Card&)> drawCallback_;
};

// ============================================================================
// §9  EvaluationResult — value object returned after each round
// ============================================================================
struct EvaluationResult {
    RoundResult result           = RoundResult::NONE;
    int         creditDelta      = 0;    // positive = player gained
    int         judgeHpDelta     = 0;    // negative = damage dealt to judge
    bool        critHit          = false;
    bool        newEnrageTriggered = false;
    float       streakMultiplier = 1.0f;
    std::string newSkillName;            // non-empty if a skill was awarded

    std::string toJson() const;
};

// ============================================================================
// §10  GameEngine — orchestrates all objects
// ============================================================================
class GameEngine {
public:
    GameEngine();

    void reset();
    void startNextRound();

    // ── Betting phase ────────────────────────────────────────────────────
    bool placeBet(int amount);
    void cancelBet();

    // ── Round flow ───────────────────────────────────────────────────────
    void              deal();
    bool              playerHit();
    void              playerStand();
    EvaluationResult  runJudgeTurn();   // reveals + draws + evaluates
    std::string       useSkill(int skillIndex);

    // ── Accessors for WASM C-bridge ──────────────────────────────────────
    const Player& getPlayer()  const { return player_; }
          Player& getPlayer()        { return player_; }
    const Judge&  getJudge()   const { return judge_;  }
          Judge&  getJudge()         { return judge_;  }
    int           getRound()   const { return round_;  }

    std::string getPhaseString() const;
    std::string getStateJson()   const;

    int getEffectiveMinBet() const;

private:
    Deck          deck_;
    Player        player_;
    Judge         judge_;
    Phase         phase_;
    int           round_;
    RoundResult   lastResult_;
    int           lastWinAmount_;
    float         lastMultiplier_;
    bool          critHit_;
    int           judgeHealAmount_;
    std::mt19937  rng_;

    EvaluationResult evaluate();
    float            streakMultiplier() const;
    SkillType        randomSkillType();
};
