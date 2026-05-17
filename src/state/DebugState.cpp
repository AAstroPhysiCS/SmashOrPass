#include "smashorpass/state/DebugState.hpp"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include "SDL3/SDL.h"
#include "smashorpass/app/AppCtx.hpp"
#include "smashorpass/core/Base.hpp"

namespace {
const char* ApplicationStateName(sop::ApplicationState state) {
    switch (state) {
        case sop::ApplicationState::None:
            return "None";
        case sop::ApplicationState::MainMenu:
            return "MainMenu";
        case sop::ApplicationState::Playing:
            return "Playing";
        case sop::ApplicationState::CharacterSelect:
            return "CharacterSelect";
        case sop::ApplicationState::Paused:
            return "Paused";
        case sop::ApplicationState::GameOver:
            return "GameOver";
    }

    return "Unknown";
}
}  // namespace

namespace sop {

DebugState::DebugState(AppCtx& ctx) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    const bool sdl3Initialized =
        ImGui_ImplSDL3_InitForSDLRenderer(ctx.m_Window.NativeHandle(), ctx.m_Renderer.NativeHandle());
    SOP_VERIFY(sdl3Initialized, "Failed to initialize ImGui SDL3 backend");

    const bool rendererInitialized = ImGui_ImplSDLRenderer3_Init(ctx.m_Renderer.NativeHandle());
    SOP_VERIFY(rendererInitialized, "Failed to initialize ImGui SDLRenderer3 backend");
}

DebugState::~DebugState() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void DebugState::BeginFrame() {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void DebugState::Draw(AppCtx& ctx) {
    const ImGuiIO& io = ImGui::GetIO();
    const SDL_FPoint logicalSize = ctx.Display.LogicalSize();
    const double frameMilliseconds = static_cast<double>(io.DeltaTime) * 1000.0;
    const double framesPerSecond = static_cast<double>(io.Framerate);

    ImGui::SetNextWindowPos(ImVec2{16.0f, 16.0f}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2{360.0f, 0.0f}, ImGuiCond_FirstUseEver);

    ImGui::Begin("Smash Or Pass Debug");
    ImGui::Text("State: %s", ApplicationStateName(ctx.CurrentState));
    ImGui::Text("Frame: %.3f ms", frameMilliseconds);
    ImGui::Text("FPS: %.1f", framesPerSecond);

    ImGui::Separator();
    ImGui::Text("Window: %d x %d", ctx.Display.WindowSize.x, ctx.Display.WindowSize.y);
    ImGui::Text("Pixels: %d x %d", ctx.Display.PixelSize.x, ctx.Display.PixelSize.y);
    ImGui::Text("Logical: %.1f x %.1f",
                static_cast<double>(logicalSize.x),
                static_cast<double>(logicalSize.y));
    ImGui::Text("Display scale: %.2f", static_cast<double>(ctx.Display.DisplayScale));
    ImGui::Text("Pixel density: %.2f", static_cast<double>(ctx.Display.PixelDensity));

    ImGui::Separator();
    ImGui::Text("Gameplay ticks: %llu", static_cast<unsigned long long>(ctx.GameplayTickCount));
    ImGui::Text("Animation ticks: %llu", static_cast<unsigned long long>(ctx.AnimationTickCount));

    ImGui::Separator();
    ImGui::Checkbox("Render collision boxes", &ctx.RenderCollisionBoxes);
    ImGui::End();
}

void DebugState::EndFrame(AppCtx& ctx) {
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), ctx.m_Renderer.NativeHandle());
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

    switch (event.RawEvent->type) {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
        case SDL_EVENT_TEXT_INPUT:
            return Ok(io.WantCaptureKeyboard ? EventFlow::Consumed : EventFlow::Passed);
        default:
            return Ok(EventFlow::Passed);
    }
}

Result<void> DebugState::OnRender(AppCtx& ctx) {
    BeginFrame();
    Draw(ctx);
    EndFrame(ctx);
    return Ok();
}

}  // namespace sop
