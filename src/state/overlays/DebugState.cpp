#include "smashorpass/state/overlays/DebugState.hpp"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <string>

#include "SDL3/SDL.h"
#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/core/Base.hpp"

namespace sop {

using namespace sop_util;

DebugState::DebugState(AppCtx&) {}

Result<void> DebugState::Initialize(AppCtx& ctx) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    const bool sdl3Initialized = ImGui_ImplSDL3_InitForSDLRenderer(ctx.m_Window.NativeHandle(),
                                                                   ctx.m_Renderer.NativeHandle());
    if (!sdl3Initialized) {
        ImGui::DestroyContext();
        return Err(std::string("Failed to initialize ImGui SDL3 backend"));
    }

    const bool rendererInitialized = ImGui_ImplSDLRenderer3_Init(ctx.m_Renderer.NativeHandle());
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
    const SDL_FPoint logicalSize = ctx.m_DisplayMetrics.LogicalSize();
    const double frameMilliseconds = static_cast<double>(io.DeltaTime) * 1000.0;
    const double framesPerSecond = static_cast<double>(io.Framerate);
    const State* topState = ctx.m_StateManager.TopState();
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
        "Window: %d x %d", ctx.m_DisplayMetrics.WindowSize.x, ctx.m_DisplayMetrics.WindowSize.y);
    ImGui::Text(
        "Pixels: %d x %d", ctx.m_DisplayMetrics.PixelSize.x, ctx.m_DisplayMetrics.PixelSize.y);
    ImGui::Text("Logical: %.1f x %.1f",
                static_cast<double>(logicalSize.x),
                static_cast<double>(logicalSize.y));
    ImGui::Text("Display scale: %.2f", static_cast<double>(ctx.m_DisplayMetrics.DisplayScale));
    ImGui::Text("Pixel density: %.2f", static_cast<double>(ctx.m_DisplayMetrics.PixelDensity));

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
