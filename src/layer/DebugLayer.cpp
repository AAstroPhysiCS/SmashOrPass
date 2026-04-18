#include "smashorpass/layer/DebugLayer.hpp"

#include "smashorpass/core/Base.hpp"

#include "smashorpass/platform/Window.hpp"
#include "smashorpass/rendering/Renderer.hpp"

#include "SDL3/SDL.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

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

    void DebugLayer::Draw() {
#ifdef SOP_ENABLE_IMGUI_DEMO
        ImGui::ShowDemoWindow();
#endif
    }

    void DebugLayer::EndFrame() {
        const auto& renderer = GetRenderer();

        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer.NativeHandle());
    }
    
    void DebugLayer::OnEvent(const Event& event) 
    {
        if (event.RawEvent == nullptr)
            return;
        ImGui_ImplSDL3_ProcessEvent(event.RawEvent);
    }
    
    void DebugLayer::OnUpdate(ApplicationContext& ctx) 
    {
        
    }

    void DebugLayer::OnRender(ApplicationContext& ctx) 
    {
        return;
        BeginFrame();
        Draw();
        EndFrame();
    }
}