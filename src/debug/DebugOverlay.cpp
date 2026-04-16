#include "smashorpass/debug/DebugOverlay.hpp"

#include "smashorpass/core/Base.hpp"
#include "smashorpass/platform/Window.hpp"
#include "smashorpass/rendering/Renderer.hpp"

#include "SDL3/SDL.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

namespace sop {

    DebugOverlay::DebugOverlay(Window& window, Renderer& renderer)
        : m_Window(window), m_Renderer(renderer) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        ImGui::StyleColorsDark();

        const bool sdl3Initialized = ImGui_ImplSDL3_InitForSDLRenderer(m_Window.nativeHandle(), m_Renderer.nativeHandle());
        SOP_VERIFY(sdl3Initialized, "Failed to initialize ImGui SDL3 backend");

        const bool rendererInitialized = ImGui_ImplSDLRenderer3_Init(m_Renderer.nativeHandle());
        SOP_VERIFY(rendererInitialized, "Failed to initialize ImGui SDLRenderer3 backend");
    }

    DebugOverlay::~DebugOverlay() {
        ImGui_ImplSDLRenderer3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }

    void DebugOverlay::OnEvent(const SDL_Event& event) {
        ImGui_ImplSDL3_ProcessEvent(&event);
    }

    void DebugOverlay::BeginFrame() {
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }

    void DebugOverlay::Draw() {
#ifdef SOP_ENABLE_IMGUI_DEMO
        ImGui::ShowDemoWindow();
#endif
    }

    void DebugOverlay::EndFrame() {
        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_Renderer.nativeHandle());
    }
}