#include "smashorpass/state/overlays/DebugState.hpp"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <string>

#include "SDL3/SDL.h"
#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/core/Base.hpp"
#include "smashorpass/state/states/in_game/InGameState.hpp"

namespace sop {

DebugState::DebugState(AppCtx&) {}

Result<void> DebugState::Initialize(AppCtx& ctx) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    const bool sdl3Initialized =
        ImGui_ImplSDL3_InitForSDLRenderer(ctx.window.NativeHandle(), ctx.renderer.NativeHandle());
    if (!sdl3Initialized) {
        ImGui::DestroyContext();
        return Err(std::string("Failed to initialize ImGui SDL3 backend"));
    }

    const bool rendererInitialized = ImGui_ImplSDLRenderer3_Init(ctx.renderer.NativeHandle());
    if (!rendererInitialized) {
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        return Err(std::string("Failed to initialize ImGui SDLRenderer3 backend"));
    }

    m_Initialized = true;
    return Ok();
}

DebugState::~DebugState() {
    if (m_Initialized) {
        ImGui_ImplSDLRenderer3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        m_Initialized = false;
    }
}

void DebugState::BeginFrame() {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void DebugState::Draw(AppCtx& ctx) {
    const ImGuiIO& io = ImGui::GetIO();
    const SDL_FPoint logicalSize = ctx.displayMetrics.LogicalSize();
    const double frameMilliseconds = static_cast<double>(io.DeltaTime) * 1000.0;
    const double framesPerSecond = static_cast<double>(io.Framerate);
    const State* topState = ctx.stateManager.TopState();
    const std::string_view stateName =
        topState != nullptr ? topState->DebugName() : std::string_view{"None"};

    ImGui::SetNextWindowPos(ImVec2{16.0f, 16.0f}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2{360.0f, 0.0f}, ImGuiCond_FirstUseEver);

    ImGui::Begin("Smash Or Pass Debug");
    ImGui::Text("State: %.*s", static_cast<int>(stateName.size()), stateName.data());
    ImGui::Text("Frame: %.3f ms", frameMilliseconds);
    ImGui::Text("FPS: %.1f", framesPerSecond);

    ImGui::Separator();
    ImGui::Text(
        "Window: %d x %d", ctx.displayMetrics.WindowSize.x, ctx.displayMetrics.WindowSize.y);
    ImGui::Text("Pixels: %d x %d", ctx.displayMetrics.PixelSize.x, ctx.displayMetrics.PixelSize.y);
    ImGui::Text("Logical: %.1f x %.1f",
                static_cast<double>(logicalSize.x),
                static_cast<double>(logicalSize.y));
    ImGui::Text("Display scale: %.2f", static_cast<double>(ctx.displayMetrics.DisplayScale));
    ImGui::Text("Pixel density: %.2f", static_cast<double>(ctx.displayMetrics.PixelDensity));

    ImGui::Separator();
    ImGui::Text("Debug Rendering");

    ImGui::Checkbox("Render Arena collision boxes", &ctx.debugRender.renderArenaCollisionBoxes);
    ImGui::Checkbox("Render Player boxes", &ctx.debugRender.renderPlayerBoxes);

    if (InGameState* inGame = ctx.stateManager.TopStateAs<InGameState>()) {
        ImGui::Separator();
        ImGui::Text("Players");

        for (std::size_t i = 0; i < inGame->GetPlayerCount(); ++i) {
            PlayerDebugRenderOptions& options = inGame->GetPlayerDebugOptions(i);
            const std::string label = "Player " + std::to_string(i + 1);

            if (ImGui::CollapsingHeader(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::PushID(static_cast<int>(i));

                ImGui::Checkbox("Enabled", &options.enabled);

                ImGui::BeginDisabled(!options.enabled);
                ImGui::Checkbox("Collision box", &options.collisionBox);
                ImGui::Checkbox("Hitboxes", &options.hitBoxes);
                ImGui::Checkbox("Hurtboxes", &options.hurtBoxes);
                ImGui::Checkbox("Combat Hitboxes", &options.combatHitBoxes);
                ImGui::Checkbox("Combat Hurtboxes", &options.combatHurtBoxes);
                ImGui::EndDisabled();

                ImGui::PopID();
            }
        }
    }

    ImGui::End();
}

void DebugState::EndFrame(AppCtx& ctx) {
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), ctx.renderer.NativeHandle());
}

Result<EventFlow> DebugState::OnEvent(AppCtx&, const Event& event) {
    if (event.RawEvent == nullptr) {
        return Ok(EventFlow::Passed);
    }

    ImGui_ImplSDL3_ProcessEvent(event.RawEvent);
    const ImGuiIO& io = ImGui::GetIO();
    if (IsPointerEventType(event.RawEvent->type) && io.WantCaptureMouse) {
        return Ok(EventFlow::Consumed);
    }

    return Ok(EventFlow::Passed);
}

Result<void> DebugState::OnRender(AppCtx& ctx) {
    BeginFrame();
    Draw(ctx);
    EndFrame(ctx);
    return Ok();
}

}  // namespace sop
