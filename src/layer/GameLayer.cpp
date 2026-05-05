#include "smashorpass/layer/GameLayer.hpp"

#include "smashorpass/core/Base.hpp"
#include "smashorpass/ui/GameScreen.hpp"
#include "smashorpass/ui/UIBuilder.hpp"

namespace sop {

GameLayer::GameLayer(Renderer& renderer, const Window& window, EventDispatcher& eventDispatcher)
    : Layer(renderer, window, eventDispatcher) {
    m_Screens.emplace_back(std::make_unique<GameScreen>(eventDispatcher));

    for (const auto& screen : m_Screens) {
        UIBuilder builder(*screen);
        screen->Build(builder);
    }
}

void GameLayer::OnEvent(const Event& event) {
    for (const auto& component : m_Screens)
        component->OnEvent(event);
    m_Game.OnEvent(event);
}

void GameLayer::OnGameplayTick(ApplicationContext& ctx) {
    if (ctx.CurrentState != ApplicationState::Playing)
        return;

    SOP_ASSERT(ctx.Assets != nullptr, "Application context missing asset manager");
    m_Game.SetDisplayMetrics(ctx.Display);
    m_Game.GameplayTick(ctx.CurrentState, ctx.GameplayStepSeconds, *ctx.Assets);
}

void GameLayer::OnAnimationTick(ApplicationContext& ctx) {
    if (ctx.CurrentState != ApplicationState::Playing)
        return;

    SOP_ASSERT(ctx.Assets != nullptr, "Application context missing asset manager");
    m_Game.AnimationTick(ctx.CurrentState, *ctx.Assets);
}

void GameLayer::OnUpdate(ApplicationContext& ctx) {
    if (ctx.CurrentState != ApplicationState::Playing)
        return;

    for (const auto& component : m_Screens)
        component->OnUpdate();
}

void GameLayer::OnRender(ApplicationContext& ctx) {
    if (ctx.CurrentState != ApplicationState::Playing)
        return;

    SOP_ASSERT(ctx.Assets != nullptr, "Application context missing asset manager");

    auto& renderer = GetRenderer();
    m_Game.SetDisplayMetrics(ctx.Display);
    m_Game.Render(ctx.CurrentState, renderer, *ctx.Assets);

    for (const auto& component : m_Screens)
        component->OnRender(renderer);
}
}  // namespace sop
