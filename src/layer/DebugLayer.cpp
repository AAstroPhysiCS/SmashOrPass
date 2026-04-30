#include "smashorpass/layer/DebugLayer.hpp"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include "SDL3/SDL.h"
#include "smashorpass/core/Base.hpp"
#include "smashorpass/platform/Window.hpp"
#include "smashorpass/rendering/Renderer.hpp"

namespace {
const char* ApplicationStateName(sop::ApplicationState state) {
    switch (state) {
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

DebugLayer::DebugLayer(Renderer& renderer, const Window& window, EventDispatcher& eventDispatcher)
    : Layer(renderer, window, eventDispatcher) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    const bool sdl3Initialized =
        ImGui_ImplSDL3_InitForSDLRenderer(window.NativeHandle(), renderer.NativeHandle());
    SOP_VERIFY(sdl3Initialized, "Failed to initialize ImGui SDL3 backend");

    const bool rendererInitialized = ImGui_ImplSDLRenderer3_Init(renderer.NativeHandle());
    SOP_VERIFY(rendererInitialized, "Failed to initialize ImGui SDLRenderer3 backend");
}

DebugLayer::~DebugLayer() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void DebugLayer::BeginFrame() {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void DebugLayer::Draw(const ApplicationContext& ctx) {
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
    ImGui::End();
}

void DebugLayer::EndFrame() {
    const auto& renderer = GetRenderer();

    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer.NativeHandle());
}

void DebugLayer::OnEvent(const Event& event) {
    if (event.RawEvent == nullptr)
        return;
    ImGui_ImplSDL3_ProcessEvent(event.RawEvent);
}

void DebugLayer::OnUpdate(ApplicationContext&) {}

void DebugLayer::OnRender(ApplicationContext& ctx) {
    BeginFrame();
    Draw(ctx);
    EndFrame();
}
}  // namespace sop
