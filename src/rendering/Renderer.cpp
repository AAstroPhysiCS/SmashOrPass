#include "smashorpass/rendering/Renderer.hpp"

#include <algorithm>
#include <cstring>

namespace sop {

    Renderer::ScopedClip::ScopedClip(Renderer& renderer, std::optional<SDL_Rect> rect)
        : m_Renderer(&renderer) {
        m_Renderer->PushClipRect(rect);
    }

    Renderer::ScopedClip::~ScopedClip() {
        if (m_Renderer)
            m_Renderer->PopClipRect();
    }

    Renderer::ScopedClip::ScopedClip(ScopedClip&& other) noexcept : m_Renderer(other.m_Renderer) {
        other.m_Renderer = nullptr;
    }

    Renderer::ScopedClip& Renderer::ScopedClip::operator=(ScopedClip&& other) noexcept {
        if (this == &other)
            return *this;

        if (m_Renderer)
            m_Renderer->PopClipRect();

        m_Renderer = other.m_Renderer;
        other.m_Renderer = nullptr;
        return *this;
    }

    Renderer::Renderer(Window& window, const char* driverName) : m_Window(window) {
        // Assuming that index 0 is the main gpu-accelerated driver, but allow overriding it with
        // driverName if provided.
        driverName = driverName ? driverName : SDL_GetRenderDriver(0);
        m_NativeHandle = SDL_CreateRenderer(window.NativeHandle(), driverName);
        if (!m_NativeHandle) {
            throw std::runtime_error(std::string("SDL_CreateRenderer failed: ") + SDL_GetError());
        }

        // Apparently good default for most UI/game rendering.
        SOP_ASSERT(SDL_SetRenderDrawBlendMode(m_NativeHandle, SDL_BLENDMODE_BLEND),
              "SDL_SetRenderDrawBlendMode");
    }

    Renderer::~Renderer() {
        if (m_NativeHandle) {
            SDL_DestroyRenderer(m_NativeHandle);
            m_NativeHandle = nullptr;
        }
    }

    void Renderer::BeginFrame(Color clear) {
        Clear(clear);
    }

    void Renderer::EndFrame() {
        SOP_ASSERT(SDL_RenderPresent(m_NativeHandle), "SDL_RenderPresent");
    }

    void Renderer::Flush() {
        SOP_ASSERT(SDL_FlushRenderer(m_NativeHandle), "SDL_FlushRenderer");
    }

    bool Renderer::SetVSync(bool enabled) {
        return SDL_SetRenderVSync(m_NativeHandle, enabled ? 1 : 0);
    }

    bool Renderer::IsVSync() const {
        int vsync = 0;
        SOP_ASSERT(SDL_GetRenderVSync(m_NativeHandle, &vsync), "SDL_GetRenderVSync");
        return vsync != 0;
    }

    bool Renderer::SetLogicalPresentation(int width, int height, SDL_RendererLogicalPresentation mode) {
        return SDL_SetRenderLogicalPresentation(m_NativeHandle, width, height, mode);
    }

    bool Renderer::GetLogicalPresentation(int& width,
                                          int& height,
                                          SDL_RendererLogicalPresentation& mode) const {
        return SDL_GetRenderLogicalPresentation(m_NativeHandle, &width, &height, &mode);
    }

    SDL_FRect Renderer::GetLogicalPresentationRect() const {
        SDL_FRect rect{};
        SOP_ASSERT(SDL_GetRenderLogicalPresentationRect(m_NativeHandle, &rect),
              "SDL_GetRenderLogicalPresentationRect");
        return rect;
    }

    bool Renderer::SetViewport(std::optional<SDL_Rect> rect) {
        return SDL_SetRenderViewport(m_NativeHandle, rect ? &*rect : nullptr);
    }

    std::optional<SDL_Rect> Renderer::GetViewport() const {
        if (!SDL_RenderViewportSet(m_NativeHandle))
            return std::nullopt;

        SDL_Rect rect{};
        SOP_ASSERT(SDL_GetRenderViewport(m_NativeHandle, &rect), "SDL_GetRenderViewport");
        return rect;
    }

    bool Renderer::SetClipRect(std::optional<SDL_Rect> rect) {
        return SDL_SetRenderClipRect(m_NativeHandle, rect ? &*rect : nullptr);
    }

    void Renderer::PushClipRect(std::optional<SDL_Rect> rect) {
        m_ClipStack.push_back(rect);
        ApplyClipStack();
    }

    void Renderer::PopClipRect() {
        if (m_ClipStack.empty())
            return;

        m_ClipStack.pop_back();
        ApplyClipStack();
    }

    bool Renderer::SetScale(float x, float y) {
        return SDL_SetRenderScale(m_NativeHandle, x, y);
    }

    SDL_FPoint Renderer::GetScale() const {
        SDL_FPoint p{};
        SOP_ASSERT(SDL_GetRenderScale(m_NativeHandle, &p.x, &p.y), "SDL_GetRenderScale");
        return p;
    }

    bool Renderer::SetTarget(SDL_Texture* target) {
        return SDL_SetRenderTarget(m_NativeHandle, target);
    }

    void Renderer::ResetTarget() {
        SOP_ASSERT(SDL_SetRenderTarget(m_NativeHandle, nullptr), "SDL_SetRenderTarget");
    }

    SDL_Texture* Renderer::GetTarget() const {
        return SDL_GetRenderTarget(m_NativeHandle);
    }

    SDL_Point Renderer::GetOutputSize() const {
        SDL_Point p{};
        SOP_ASSERT(SDL_GetRenderOutputSize(m_NativeHandle, &p.x, &p.y), "SDL_GetRenderOutputSize");
        return p;
    }

    SDL_Point Renderer::GetCurrentOutputSize() const {
        SDL_Point p{};
        SOP_ASSERT(SDL_GetCurrentRenderOutputSize(m_NativeHandle, &p.x, &p.y),
              "SDL_GetCurrentRenderOutputSize");
        return p;
    }

    SDL_Rect Renderer::GetSafeArea() const {
        SDL_Rect r{};
        SOP_ASSERT(SDL_GetRenderSafeArea(m_NativeHandle, &r), "SDL_GetRenderSafeArea");
        return r;
    }

    bool Renderer::WindowToRender(float windowX, float windowY, float& renderX, float& renderY) const {
        return SDL_RenderCoordinatesFromWindow(m_NativeHandle, windowX, windowY, &renderX, &renderY);
    }

    bool Renderer::RenderToWindow(float renderX, float renderY, float& windowX, float& windowY) const {
        return SDL_RenderCoordinatesToWindow(m_NativeHandle, renderX, renderY, &windowX, &windowY);
    }

    bool Renderer::ConvertEventToRenderCoordinates(SDL_Event& event) const {
        return SDL_ConvertEventToRenderCoordinates(m_NativeHandle, &event);
    }

    bool Renderer::Clear(Color color) {
        if (!SetDrawColor(color))
            return false;
        return SDL_RenderClear(m_NativeHandle);
    }

    bool Renderer::SetDrawColor(Color color) {
        return SDL_SetRenderDrawColor(m_NativeHandle, color.r, color.g, color.b, color.a);
    }

    Color Renderer::GetDrawColor() const {
        Color c{};
        SOP_ASSERT(SDL_GetRenderDrawColor(m_NativeHandle, &c.r, &c.g, &c.b, &c.a), "SDL_GetRenderDrawColor");
        return c;
    }

    bool Renderer::SetBlendMode(SDL_BlendMode blendMode) {
        return SDL_SetRenderDrawBlendMode(m_NativeHandle, blendMode);
    }

    SDL_BlendMode Renderer::GetBlendMode() const {
        SDL_BlendMode mode{};
        SOP_ASSERT(SDL_GetRenderDrawBlendMode(m_NativeHandle, &mode), "SDL_GetRenderDrawBlendMode");
        return mode;
    }

    bool Renderer::DrawPoint(float x, float y, Color color) {
        if (!SetDrawColor(color))
            return false;
        return SDL_RenderPoint(m_NativeHandle, x, y);
    }

    bool Renderer::DrawLine(float x1, float y1, float x2, float y2, Color color) {
        if (!SetDrawColor(color))
            return false;
        return SDL_RenderLine(m_NativeHandle, x1, y1, x2, y2);
    }

    bool Renderer::DrawLines(std::span<const SDL_FPoint> points, Color color) {
        if (points.empty())
            return true;

        if (!SetDrawColor(color))
            return false;

        return SDL_RenderLines(m_NativeHandle, points.data(), static_cast<int>(points.size()));
    }

    bool Renderer::DrawRect(const SDL_FRect& rect, Color color) {
        if (!SetDrawColor(color))
            return false;
        return SDL_RenderRect(m_NativeHandle, &rect);
    }

    bool Renderer::DrawRects(std::span<const SDL_FRect> rects, Color color) {
        if (rects.empty())
            return true;

        if (!SetDrawColor(color))
            return false;

        return SDL_RenderRects(m_NativeHandle, rects.data(), static_cast<int>(rects.size()));
    }

    bool Renderer::FillRect(const SDL_FRect& rect, Color color) {
        if (!SetDrawColor(color))
            return false;
        return SDL_RenderFillRect(m_NativeHandle, &rect);
    }

    bool Renderer::FillRects(std::span<const SDL_FRect> rects, Color color) {
        if (rects.empty())
            return true;

        if (!SetDrawColor(color))
            return false;

        return SDL_RenderFillRects(m_NativeHandle, rects.data(), static_cast<int>(rects.size()));
    }

    bool Renderer::DrawTexture(SDL_Texture* texture, const SDL_FRect& dst) {
        TextureDrawParams params{};
        params.dst = dst;
        return DrawTexture(texture, params);
    }

    bool Renderer::DrawTexture(SDL_Texture* texture, const TextureDrawParams& params) {
        if (!texture)
            return false;

        const auto backup = BackupTextureState(texture);
        ApplyTextureState(texture, params.tint, params.blendMode);

        const bool ok = (params.rotationDeg == 0.0 && params.flip == SDL_FLIP_NONE)
                            ? SDL_RenderTexture(m_NativeHandle, texture, params.src, &params.dst)
                            : SDL_RenderTextureRotated(m_NativeHandle,
                                                       texture,
                                                       params.src,
                                                       &params.dst,
                                                       params.rotationDeg,
                                                       &params.origin,
                                                       params.flip);

        RestoreTextureState(texture, backup);
        return ok;
    }

    bool Renderer::DrawTextureTiled(SDL_Texture* texture, const TiledTextureDrawParams& params) {
        if (!texture)
            return false;

        const auto backup = BackupTextureState(texture);
        ApplyTextureState(texture, params.tint, params.blendMode);

        const bool ok =
            SDL_RenderTextureTiled(m_NativeHandle, texture, params.src, params.scale, &params.dst);

        RestoreTextureState(texture, backup);
        return ok;
    }

    bool Renderer::DrawTexture9Grid(SDL_Texture* texture, const NineGridDrawParams& params) {
        if (!texture)
            return false;

        const auto backup = BackupTextureState(texture);
        ApplyTextureState(texture, params.tint, params.blendMode);

        const bool ok = SDL_RenderTexture9Grid(m_NativeHandle,
                                               texture,
                                               params.src,
                                               params.leftWidth,
                                               params.rightWidth,
                                               params.topHeight,
                                               params.bottomHeight,
                                               params.scale,
                                               &params.dst);

        RestoreTextureState(texture, backup);
        return ok;
    }

    bool Renderer::DrawGeometry(SDL_Texture* texture,
                                std::span<const SDL_Vertex> vertices,
                                std::span<const int> indices) {
        if (vertices.empty())
            return true;

        return SDL_RenderGeometry(m_NativeHandle,
                                  texture,
                                  vertices.data(),
                                  static_cast<int>(vertices.size()),
                                  indices.empty() ? nullptr : indices.data(),
                                  static_cast<int>(indices.size()));
    }

    bool Renderer::DebugText(float x, float y, std::string_view text, Color color) {
        if (!SetDrawColor(color))
            return false;

        std::string owned(text);
        return SDL_RenderDebugText(m_NativeHandle, x, y, owned.c_str());
    }

    SDL_Surface* Renderer::ReadPixels(const SDL_Rect* rect) const {
        return SDL_RenderReadPixels(m_NativeHandle, rect);
    }

    std::optional<SDL_Rect> Renderer::Intersect(const std::optional<SDL_Rect>& a,
                                                 const std::optional<SDL_Rect>& b) {
        if (!a)
            return b;
        if (!b)
            return a;

        const int x1 = std::max(a->x, b->x);
        const int y1 = std::max(a->y, b->y);
        const int x2 = std::min(a->x + a->w, b->x + b->w);
        const int y2 = std::min(a->y + a->h, b->y + b->h);

        if (x2 <= x1 || y2 <= y1)
            return SDL_Rect{x1, y1, 0, 0};

        return SDL_Rect{x1, y1, x2 - x1, y2 - y1};
    }

    void Renderer::ApplyClipStack() {
        std::optional<SDL_Rect> effective;
        for (const auto& clip : m_ClipStack)
            effective = Intersect(effective, clip);

        SOP_ASSERT(SDL_SetRenderClipRect(m_NativeHandle, effective ? &*effective : nullptr),
              "SDL_SetRenderClipRect");
    }

    Renderer::TextureStateBackup Renderer::BackupTextureState(SDL_Texture* texture) const {
        TextureStateBackup backup{};
        SOP_ASSERT(SDL_GetTextureColorMod(texture, &backup.r, &backup.g, &backup.b),
              "SDL_GetTextureColorMod");
        SOP_ASSERT(SDL_GetTextureAlphaMod(texture, &backup.a), "SDL_GetTextureAlphaMod");
        SOP_ASSERT(SDL_GetTextureBlendMode(texture, &backup.blendMode), "SDL_GetTextureBlendMode");
        return backup;
    }

    void Renderer::RestoreTextureState(SDL_Texture* texture, const TextureStateBackup& backup) const {
        SOP_ASSERT(SDL_SetTextureColorMod(texture, backup.r, backup.g, backup.b), "SDL_SetTextureColorMod");
        SOP_ASSERT(SDL_SetTextureAlphaMod(texture, backup.a), "SDL_SetTextureAlphaMod");
        SOP_ASSERT(SDL_SetTextureBlendMode(texture, backup.blendMode), "SDL_SetTextureBlendMode");
    }

    void Renderer::ApplyTextureState(SDL_Texture* texture, Color tint, SDL_BlendMode blendMode) const {
        SOP_ASSERT(SDL_SetTextureColorMod(texture, tint.r, tint.g, tint.b), "SDL_SetTextureColorMod");
        SOP_ASSERT(SDL_SetTextureAlphaMod(texture, tint.a), "SDL_SetTextureAlphaMod");
        SOP_ASSERT(SDL_SetTextureBlendMode(texture, blendMode), "SDL_SetTextureBlendMode");
    }

}  // namespace sop