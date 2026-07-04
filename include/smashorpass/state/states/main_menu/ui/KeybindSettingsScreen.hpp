#pragma once

#include <SDL3/SDL_keycode.h>

#include <cstddef>
#include <optional>

#include "smashorpass/core/Settings.hpp"
#include "smashorpass/ui/UIScreen.hpp"

namespace sop {

class KeybindSettingsScreen : public UIScreen {
   public:
    explicit KeybindSettingsScreen(AppCtx& ctx);
    ~KeybindSettingsScreen() override = default;

    void Build(UIBuilder& builder) final override;
    EventFlow OnEvent(AppCtx& ctx, const Event& event) final override;

   private:
    enum class KeybindAction {
        MoveLeft,
        MoveRight,
        Jump,
        Dash,
        Attack,
    };

    struct PendingBinding {
        std::size_t PlayerIndex = 0;
        KeybindAction Action = KeybindAction::MoveLeft;
    };

    void StartBinding(std::size_t playerIndex, KeybindAction action);
    void AssignPendingBinding(AppCtx& ctx, SDL_Keycode key);
    void ResetKeybinds(AppCtx& ctx);
    [[nodiscard]] bool CanAssignKey(const AppCtx& ctx,
                                    std::size_t playerIndex,
                                    KeybindAction action,
                                    SDL_Keycode key) const;
    void SaveSettings(AppCtx& ctx) const;
    static void SetKeyForAction(PlayerKeyBindings& bindings, KeybindAction action, SDL_Keycode key);
    [[nodiscard]] static SDL_Keycode GetKeyForAction(const PlayerKeyBindings& bindings,
                                                     KeybindAction action);

    std::optional<PendingBinding> m_PendingBinding;
};

}  // namespace sop
