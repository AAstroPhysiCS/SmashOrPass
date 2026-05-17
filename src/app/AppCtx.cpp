#include "smashorpass/app/AppCtx.hpp"

namespace sop {

AppCtx::AppCtx()
    : m_Window(
          WindowCreateInfo{.Width = 1920, .Height = 1080, .Title = "Smash Or Pass - The Game"}),
      m_Renderer(m_Window),
      m_ParticleSystem(m_Renderer) {}

}  // namespace sop
