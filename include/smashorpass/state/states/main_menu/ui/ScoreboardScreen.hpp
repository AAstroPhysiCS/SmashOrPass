#pragma once

#include <string>

#include "smashorpass/state/states/in_game/OverallStats.hpp"
#include "smashorpass/ui/UIScreen.hpp"

namespace sop {

class ScoreboardScreen : public UIScreen {
   public:
    explicit ScoreboardScreen(AppCtx& ctx);
    ~ScoreboardScreen() override = default;

    void Build(UIBuilder& builder) final override;

   private:
    [[nodiscard]] std::string MakeWinRateText(const OverallMatchupStats& stats) const;
    [[nodiscard]] std::string MakeRoundedText(float value) const;
};

}  // namespace sop
