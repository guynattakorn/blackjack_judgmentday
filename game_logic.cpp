/**
 * ============================================================================
 * game_logic.cpp — FRA143 Blackjack: Judgment Day
 * ============================================================================
 */

#include "game_logic.h"

// ============================================================================
// §2 Card Implementation
// ============================================================================
const std::string Card::suitSymbols_[4] = {"♠", "♥", "♦", "♣"};
const std::string Card::suitCodes_[4]   = {"S", "H", "D", "C"};

Card::Card(Suit suit, const RankInfo& rankInfo, bool faceDown)
    : suit_(suit), rank_(rankInfo.label), value_(rankInfo.baseValue), faceDown_(faceDown) {}

Suit        Card::getSuit()       const { return suit_; }
std::string Card::getSuitSymbol() const { return suitSymbols_[static_cast<int>(suit_)]; }
std::string Card::getSuitCode()   const { return suitCodes_[static_cast<int>(suit_)]; }
std::string Card::getRank()       const { return rank_; }
int         Card::getValue()      const { return value_; }
bool        Card::isFaceDown()    const { return faceDown_; }
void        Card::flip()                { faceDown_ = !faceDown_; }

std::string Card::toJson() const {
    std::ostringstream ss;
    ss << "{\"suit\":\"" << getSuitCode() << "\",\"rank\":\"" << rank_
       << "\",\"value\":" << value_ << ",\"faceDown\":" << (faceDown_ ? "true" : "false") << "}";
    return ss.str();
}

// ============================================================================
// §3 Hand Implementation
// ============================================================================

// [CRIT-4] addCard — push Card object into runtime vector collection
void Hand::addCard(Card&& card) {
    cards_.push_back(std::move(card));
}

void Hand::clear() { cards_.clear(); }

int Hand::score() const {
    int total = 0, aces = 0;
    for (const auto& c : cards_) {
        if (c.isFaceDown()) continue;
        total += c.getValue();
        if (c.getRank() == "A") aces++;
    }
    while (total > 21 && aces > 0) { total -= 10; aces--; }
    return total;
}

bool Hand::isNaturalBlackjack() const { return cards_.size() == 2 && score() == 21; }
bool Hand::isBust()             const { return score() > 21; }
int  Hand::size()               const { return static_cast<int>(cards_.size()); }
const Card& Hand::cardAt(int index) const { return cards_.at(index); }

void Hand::revealAll() {
    for (auto& c : cards_) if (c.isFaceDown()) c.flip();
}

std::string Hand::toJson() const {
    std::ostringstream ss;
    ss << "[";
    for (size_t i = 0; i < cards_.size(); ++i) {
        ss << cards_[i].toJson();
        if (i < cards_.size() - 1) ss << ",";
    }
    ss << "]";
    return ss.str();
}

// ============================================================================
// §4 Deck Implementation
// ============================================================================
Deck::Deck() : rng_(std::random_device{}()) { build(); }

void Deck::build() {
    cards_.clear();
    const std::vector<RankInfo> ranks = {
        {"2",2},{"3",3},{"4",4},{"5",5},{"6",6},{"7",7},{"8",8},
        {"9",9},{"10",10},{"J",10},{"Q",10},{"K",10},{"A",11}
    };
    for (int s = 0; s < 4; ++s)
        for (const auto& r : ranks)
            cards_.emplace_back(static_cast<Suit>(s), r);
    shuffle();
}

void Deck::shuffle() { std::shuffle(cards_.begin(), cards_.end(), rng_); }

// [CRIT-4] drawTop — Card is physically REMOVED from the runtime vector
Card Deck::drawTop() {
    if (cards_.empty()) build();
    Card top = std::move(cards_.back());
    cards_.pop_back();
    return top;
}

int  Deck::remaining() const { return static_cast<int>(cards_.size()); }
bool Deck::isEmpty()   const { return cards_.empty(); }
void Deck::rebuild()         { build(); }

// ============================================================================
// §5 SkillInventory Implementation
// ============================================================================
Skill SkillInventory::makeSkill(SkillType t) {
    switch (t) {
        case SkillType::SCAN_PROTOCOL:   return {t, "SCAN PROTOCOL",   "Reveal 1 hidden Judge card"};
        case SkillType::DOUBLE_RISK:     return {t, "DOUBLE RISK",     "Double your bet this round"};
        case SkillType::OVERRIDE_SHIELD: return {t, "OVERRIDE SHIELD", "Prevent bust once this round"};
        default:                         return {t, "UNKNOWN", ""};
    }
}

// [CRIT-4] addSkill — Skill object is ADDED to runtime vector collection
void SkillInventory::addSkill(SkillType type) {
    skills_.push_back(makeSkill(type));
}

// [CRIT-4] useSkill — Skill object is ERASED from runtime vector collection
bool SkillInventory::useSkill(int index, Skill& outSkill) {
    if (index < 0 || index >= static_cast<int>(skills_.size())) return false;
    outSkill = skills_[index];
    skills_.erase(skills_.begin() + index);   // runtime removal
    return true;
}

const Skill& SkillInventory::getSkill(int index) const { return skills_.at(index); }
int          SkillInventory::count()             const { return static_cast<int>(skills_.size()); }

std::string SkillInventory::toJson() const {
    std::ostringstream ss;
    ss << "[";
    for (size_t i = 0; i < skills_.size(); ++i) {
        ss << "{\"type\":" << static_cast<int>(skills_[i].type)
           << ",\"name\":\"" << skills_[i].name
           << "\",\"desc\":\"" << skills_[i].desc << "\"}";
        if (i < skills_.size() - 1) ss << ",";
    }
    ss << "]";
    return ss.str();
}

// ============================================================================
// §6 Participant — Abstract Base Class  [CRIT-2]
// ============================================================================
Participant::Participant(const std::string& name, Deck& deck)
    : deck_(deck), name_(name) {}

Hand&       Participant::getHand()              { return hand_; }
const Hand& Participant::getHand()        const { return hand_; }
void        Participant::receiveCard(Card&& c)  { hand_.addCard(std::move(c)); }
void        Participant::clearHand()            { hand_.clear(); }
int         Participant::getScore()       const { return hand_.score(); }
bool        Participant::isBust()         const { return hand_.isBust(); }
std::string Participant::getName()        const { return name_; }

// ============================================================================
// §7 Player : Participant  (Derived Class 1)  [CRIT-3]
// ============================================================================
Player::Player(Deck& deck)
    : Participant("CITIZEN UNIT-001", deck),
      credits_(STARTING_CREDITS), bet_(0),
      shieldActive_(false), consecutiveWins_(0) {}

// Override pure virtual — basic auto-stand at 17
void Player::takeTurn() {
    while (getScore() < 17)
        receiveCard(deck_.drawTop());
}
std::string Player::role() const { return "PLAYER"; }

int  Player::getCredits()       const { return credits_; }
void Player::setCredits(int c)        { credits_ = c; }
bool Player::isEliminated()     const { return credits_ <= 0; }

int  Player::getCurrentBet()    const { return bet_; }
void Player::placeBet(int a)          { bet_ = a; }
void Player::resetBet()               { bet_ = 0; }

void Player::applyWin(int a)  { credits_ += a; }
void Player::applyLoss(int a) { credits_ -= a; if (credits_ < 0) credits_ = 0; }

bool Player::isShieldActive()   const { return shieldActive_; }
void Player::activateShield()         { shieldActive_ = true; }
void Player::deactivateShield()       { shieldActive_ = false; }

int  Player::getConsecutiveWins()  const { return consecutiveWins_; }
void Player::incrementWinStreak()        { consecutiveWins_++; }
void Player::resetWinStreak()           { consecutiveWins_ = 0; }

SkillInventory&       Player::getSkills()          { return skills_; }
const SkillInventory& Player::getSkillsConst() const { return skills_; }

std::string Player::toJson() const { return getHand().toJson(); }

// ============================================================================
// §8 Judge : Participant  (Derived Class 2)  [CRIT-3]
// ============================================================================
Judge::Judge(Deck& deck)
    : Participant("THE JUDGE", deck), hp_(JUDGE_START_HP) {}

// Override pure virtual — reveal hidden card, draw until score ≥ 17
void Judge::takeTurn() {
    getHand().revealAll();
    while (getScore() < 17) {
        Card c = deck_.drawTop();
        if (drawCallback_) drawCallback_(c);
        receiveCard(std::move(c));
    }
}
std::string Judge::role() const { return "JUDGE"; }

int  Judge::getHp()         const { return hp_; }
void Judge::applyDamage(int d)    { hp_ -= d; if (hp_ < 0) hp_ = 0; }
void Judge::applyHeal(int a)      { hp_ += a; if (hp_ > JUDGE_START_HP) hp_ = JUDGE_START_HP; }
void Judge::resetHp()           { hp_ = JUDGE_START_HP; }
bool Judge::isDefeated()    const { return hp_ <= 0; }
bool Judge::isEnraged()     const { return hp_ <= ENRAGE_THRESHOLD && hp_ > 0; }

void Judge::setDrawCallback(std::function<void(const Card&)> cb) { drawCallback_ = cb; }

std::string Judge::toJson() const { return getHand().toJson(); }

// ============================================================================
// §9 EvaluationResult
// ============================================================================
std::string EvaluationResult::toJson() const {
    auto rstr = [&](){
        switch (result) {
            case RoundResult::PLAYER_WIN: return "PLAYER_WIN";
            case RoundResult::JUDGE_WIN:  return "JUDGE_WIN";
            case RoundResult::DRAW:       return "DRAW";
            default:                      return "NONE";
        }
    };
    std::ostringstream ss;
    ss << "{\"result\":\"" << rstr()
       << "\",\"creditDelta\":" << creditDelta
       << ",\"judgeHpDelta\":" << judgeHpDelta
       << ",\"critHit\":"     << (critHit          ? "true" : "false")
       << ",\"newEnrageTriggered\":" << (newEnrageTriggered ? "true" : "false")
       << ",\"streakMultiplier\":" << streakMultiplier
       << ",\"newSkillName\":\"" << newSkillName << "\"}";
    return ss.str();
}

// ============================================================================
// §10 GameEngine Implementation
// ============================================================================
GameEngine::GameEngine()
    : deck_(), player_(deck_), judge_(deck_), rng_(std::random_device{}()) {
    reset();
}

void GameEngine::reset() {
    deck_.rebuild();
    // Re-initialise player and judge state manually (cannot reassign reference members)
    player_.clearHand();
    player_.setCredits(STARTING_CREDITS);
    player_.resetBet();
    player_.deactivateShield();
    player_.resetWinStreak();
    // Clear existing skills and add a fresh starting skill
    while (player_.getSkills().count() > 0) {
        Skill tmp; player_.getSkills().useSkill(0, tmp);
    }
    player_.getSkills().addSkill(randomSkillType());

    judge_.clearHand();
    judge_.resetHp();
    phase_          = Phase::BETTING;
    round_          = 0;
    lastWinAmount_  = 0;
    lastMultiplier_ = 1.0f;
    critHit_        = false;
    judgeHealAmount_= 0;
    lastResult_     = RoundResult::NONE;
    shieldAbsorbedBust_ = false;
}

void GameEngine::startNextRound() {
    player_.clearHand();
    judge_.clearHand();
    player_.resetBet();
    player_.deactivateShield();
    lastResult_         = RoundResult::NONE;
    critHit_            = false;
    judgeHealAmount_    = 0;
    shieldAbsorbedBust_ = false;
    phase_              = Phase::BETTING;
}

std::string GameEngine::getPhaseString() const {
    switch (phase_) {
        case Phase::BETTING:      return "BETTING";
        case Phase::DEALING:      return "DEALING";
        case Phase::PLAYER_TURN:  return "PLAYER_TURN";
        case Phase::JUDGE_REVEAL: return "JUDGE_REVEAL";
        case Phase::JUDGE_TURN:   return "JUDGE_TURN";
        case Phase::ROUND_OVER:   return "ROUND_OVER";
        case Phase::GAME_OVER:    return "GAME_OVER";
        case Phase::VICTORY:      return "VICTORY";
        default:                  return "UNKNOWN";
    }
}

SkillType GameEngine::randomSkillType() {
    std::uniform_int_distribution<int> d(0, 2);
    return static_cast<SkillType>(d(rng_));
}

float GameEngine::streakMultiplier() const {
    int w = player_.getConsecutiveWins();
    if (w >= 7) return 3.0f;
    if (w >= 5) return 2.0f;
    if (w >= 3) return 1.5f;
    return 1.0f;
}

bool GameEngine::placeBet(int amount) {
    if (phase_ != Phase::BETTING) return false;
    if (amount < getEffectiveMinBet() || amount > player_.getCredits()) return false;
    player_.placeBet(amount);
    return true;
}

void GameEngine::cancelBet() { player_.resetBet(); }

void GameEngine::deal() {
    if (phase_ != Phase::BETTING) return;
    player_.clearHand();
    judge_.clearHand();
    round_++;

    // [CRIT-4] Cards are moved from Deck (pop_back) into Hand (push_back) at runtime
    player_.receiveCard(deck_.drawTop());
    player_.receiveCard(deck_.drawTop());

    judge_.receiveCard(deck_.drawTop());
    Card hidden = deck_.drawTop();
    hidden.flip();                        // face-down
    judge_.receiveCard(std::move(hidden));

    player_.deactivateShield();
    phase_ = Phase::PLAYER_TURN;
}

bool GameEngine::playerHit() {
    if (phase_ != Phase::PLAYER_TURN) return false;
    player_.receiveCard(deck_.drawTop());
    if (player_.isBust()) {
        if (player_.isShieldActive() && !shieldAbsorbedBust_) {
            // First bust while shield is active:
            // Mark it as absorbed but KEEP the shield active so that evaluate()
            // will see isShieldActive()==true and correctly skip the bust penalty.
            // The player survives this bust and may continue to HIT or STAND.
            shieldAbsorbedBust_ = true;
        } else {
            // Either no shield, or the shield already saved a bust this round.
            // Deactivate shield (so evaluate() counts this as a real bust)
            // and advance the phase to end the player's turn.
            player_.deactivateShield();
            phase_ = Phase::JUDGE_REVEAL;
        }
    }
    return true;
}

void GameEngine::playerStand() {
    if (phase_ == Phase::PLAYER_TURN) phase_ = Phase::JUDGE_REVEAL;
}

EvaluationResult GameEngine::evaluate() {
    EvaluationResult res;

    // Shield stays active throughout the player's turn when it has absorbed a bust.
    // So if the player busted but the shield is STILL active here, we treat it as
    // NOT a bust (the shield protection holds for the rest of the round).
    const bool pBust  = player_.isBust() && !player_.isShieldActive();
    const bool jBust  = judge_.isBust();
    const int  pScore = player_.getScore();
    const int  jScore = judge_.getScore();

    // Standard rule: player bust = judge wins regardless of judge bust status.
    if      (pBust)           res.result = RoundResult::JUDGE_WIN;
    else if (jBust)           res.result = RoundResult::PLAYER_WIN;
    else if (pScore > jScore) res.result = RoundResult::PLAYER_WIN;
    else if (jScore > pScore) res.result = RoundResult::JUDGE_WIN;
    else                      res.result = RoundResult::DRAW;

    lastResult_ = res.result;
    const bool wasEnraged = judge_.isEnraged();

    if (res.result == RoundResult::PLAYER_WIN) {
        player_.incrementWinStreak();
        res.streakMultiplier = streakMultiplier();
        int dmg = static_cast<int>(player_.getCurrentBet() * res.streakMultiplier);

        if (player_.getHand().isNaturalBlackjack()) {
            dmg = static_cast<int>(dmg * 1.5f);
            res.critHit = true;
        }
        player_.applyWin(dmg);
        judge_.applyDamage(dmg);
        res.creditDelta  = dmg;
        res.judgeHpDelta = -dmg;

        // Award skill every 2 consecutive wins
        if (player_.getConsecutiveWins() % 2 == 0) {
            SkillType st = randomSkillType();
            // [CRIT-4] Skill object added to SkillInventory runtime collection
            player_.getSkills().addSkill(st);
            res.newSkillName = SkillInventory::makeSkill(st).name;
        }
    } else if (res.result == RoundResult::JUDGE_WIN) {
        int dmg = player_.getCurrentBet();
        if (judge_.getHand().isNaturalBlackjack()) {
            dmg = static_cast<int>(dmg * 1.5f);
            res.critHit = true;
        }
        player_.applyLoss(dmg);
        judge_.applyHeal(dmg);
        res.creditDelta  = -dmg;
        res.judgeHpDelta =  dmg;
        player_.resetWinStreak();
    } else {
        player_.resetWinStreak();
    }

    if (!wasEnraged && judge_.isEnraged()) res.newEnrageTriggered = true;

    if      (judge_.isDefeated())    phase_ = Phase::VICTORY;
    else if (player_.isEliminated()) phase_ = Phase::GAME_OVER;
    else                             phase_ = Phase::ROUND_OVER;

    lastWinAmount_   = res.creditDelta;
    lastMultiplier_  = res.streakMultiplier;
    critHit_         = res.critHit;
    judgeHealAmount_ = (res.judgeHpDelta > 0) ? res.judgeHpDelta : 0;

    return res;
}

EvaluationResult GameEngine::runJudgeTurn() {
    phase_ = Phase::JUDGE_TURN;
    judge_.takeTurn();
    return evaluate();
}

std::string GameEngine::useSkill(int skillIndex) {
    if (phase_ != Phase::PLAYER_TURN) return "";
    Skill sk;
    // [CRIT-4] useSkill erases Skill from runtime collection
    if (!player_.getSkills().useSkill(skillIndex, sk)) return "";

    if      (sk.type == SkillType::SCAN_PROTOCOL)   { judge_.getHand().revealAll(); return "Scan Protocol Activated."; }
    else if (sk.type == SkillType::DOUBLE_RISK)      { player_.placeBet(player_.getCurrentBet() * 2); return "Double Risk Activated."; }
    else if (sk.type == SkillType::OVERRIDE_SHIELD)  { player_.activateShield(); return "Override Shield Activated."; }
    return "";
}

int GameEngine::getEffectiveMinBet() const {
    return judge_.isEnraged() ? ENRAGE_MIN_BET : MIN_BET;
}

std::string GameEngine::getStateJson() const {
    auto rstr = [&](){
        switch (lastResult_) {
            case RoundResult::PLAYER_WIN: return "PLAYER_WIN";
            case RoundResult::JUDGE_WIN:  return "JUDGE_WIN";
            case RoundResult::DRAW:       return "DRAW";
            default:                      return "NONE";
        }
    };

    // Generate phase-appropriate message for the UI
    auto makeMessage = [&]() -> std::string {
        switch (phase_) {
            case Phase::BETTING:
                if (judge_.isEnraged())
                    return "PHASE 2 — The Judge is enraged. Min bet raised to \\u00a250.";
                return "Place your wager to begin the round.";
            case Phase::PLAYER_TURN:
                if (player_.isShieldActive())
                    return "Override Shield active. You cannot bust this draw. HIT or STAND?";
                return "Your move, Citizen. HIT to draw a card. STAND to hold.";
            case Phase::JUDGE_REVEAL:
                return "The Judge reveals the hidden card...";
            case Phase::JUDGE_TURN:
                return "The Judge draws...";
            case Phase::ROUND_OVER:
                switch (lastResult_) {
                    case RoundResult::PLAYER_WIN:
                        return critHit_
                            ? "CRITICAL STRIKE! Bonus damage dealt to the Judge."
                            : "Round won. Damage dealt to the Judge.";
                    case RoundResult::JUDGE_WIN:
                        return critHit_
                            ? "Counter Crit! The Judge strikes back with full force."
                            : "Round lost. The Judge reclaims power via life steal.";
                    case RoundResult::DRAW:
                        return "Stalemate. Neither side gains ground.";
                    default: return "Round complete.";
                }
            case Phase::GAME_OVER:
                return "Judgment has been passed. You have been eliminated from the system.";
            case Phase::VICTORY:
                return "THE JUDGE HAS FALLEN. The system is broken. You are free.";
            default:
                return "";
        }
    };

    // Escape the message string for safe JSON embedding
    auto jsonEscape = [](const std::string& s) -> std::string {
        std::ostringstream out;
        for (char c : s) {
            switch (c) {
                case '"':  out << "\\\""; break;
                case '\\': out << "\\\\"; break;
                case '\n': out << "\\n";  break;
                case '\r': out << "\\r";  break;
                case '\t': out << "\\t";  break;
                default:   out << c;      break;
            }
        }
        return out.str();
    };

    std::ostringstream ss;
    ss << "{"
       << "\"phase\":\""          << getPhaseString()                          << "\","
       << "\"result\":\""         << rstr()                                    << "\","
       << "\"round\":"            << round_                                    << ","
       << "\"money\":"            << player_.getCredits()                      << ","
       << "\"bet\":"              << player_.getCurrentBet()                   << ","
       << "\"judgeHp\":"          << judge_.getHp()                            << ","
       << "\"consecutiveWins\":"  << player_.getConsecutiveWins()              << ","
       << "\"shieldActive\":"     << (player_.isShieldActive()?"true":"false") << ","
       << "\"critHit\":"          << (critHit_?"true":"false")                 << ","
       << "\"lastWinAmount\":"    << lastWinAmount_                            << ","
       << "\"lastMultiplier\":"   << lastMultiplier_                           << ","
       << "\"judgeHealAmount\":"  << judgeHealAmount_                          << ","
       << "\"playerHand\":"       << player_.toJson()                          << ","
       << "\"judgeHand\":"        << judge_.toJson()                           << ","
       << "\"skills\":"           << player_.getSkillsConst().toJson()         << ","
       << "\"deckRemaining\":"    << deck_.remaining()                         << ","
       << "\"message\":\""        << jsonEscape(makeMessage())                 << "\""
       << "}";
    return ss.str();
}

// ============================================================================
// §11 WASM C-API Bridge  [Emscripten Exports]
// ============================================================================
static GameEngine globalEngine;

extern "C" {
    void        bj_new_game()           { globalEngine.reset(); }
    const char* bj_get_state()          { static std::string s; s = globalEngine.getStateJson(); return s.c_str(); }
    int         bj_place_bet(int a)     { return globalEngine.placeBet(a) ? 1 : 0; }
    void        bj_cancel_bet()         { globalEngine.cancelBet(); }
    void        bj_deal()               { globalEngine.deal(); }
    int         bj_player_hit()         { return globalEngine.playerHit() ? 1 : 0; }
    void        bj_player_stand()       { globalEngine.playerStand(); }
    const char* bj_use_skill(int i)     { static std::string s; s = globalEngine.useSkill(i); return s.c_str(); }
    const char* bj_run_judge_turn()     { static std::string s; s = globalEngine.runJudgeTurn().toJson(); return s.c_str(); }
    int         bj_get_round()          { return globalEngine.getRound(); }
    int         bj_get_player_credits() { return globalEngine.getPlayer().getCredits(); }
    int         bj_get_judge_hp()       { return globalEngine.getJudge().getHp(); }
    int         bj_is_enraged()         { return globalEngine.getJudge().isEnraged() ? 1 : 0; }
    void bj_next_round()                { globalEngine.startNextRound(); }

}
