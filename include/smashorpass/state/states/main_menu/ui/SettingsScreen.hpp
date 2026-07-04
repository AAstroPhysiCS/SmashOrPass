#pragma once

#include "smashorpass/ui/UIScreen.hpp"

namespace sop {

class SettingsScreen : public UIScreen {
   public:
    explicit SettingsScreen(AppCtx& ctx);
    ~SettingsScreen() override = default;

    void Build(UIBuilder& builder) final override;

   private:
    enum class SettingValue {
        DeathmatchRounds,
        SmashRounds,
        SmashStocks,
        DeathmatchOutOfBoundsDamage,
    };

    void AdjustSetting(AppCtx& ctx, SettingValue setting, int delta);
    void ResetSettings(AppCtx& ctx);
    void UpdateSettingLabels();
    void SaveSettings(AppCtx& ctx) const;
    void SetValueLabel(UIWidgetId labelId, int value);

    UIWidgetId m_DeathmatchRoundsLabel = g_InvalidWidgetId;
    UIWidgetId m_SmashRoundsLabel = g_InvalidWidgetId;
    UIWidgetId m_SmashStocksLabel = g_InvalidWidgetId;
    UIWidgetId m_DeathmatchOutOfBoundsDamageLabel = g_InvalidWidgetId;
};

}  // namespace sop
