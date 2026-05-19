#include "smashorpass/rendering/Renderer.hpp"

#include <algorithm>

#include "smashorpass/ui/UIWidget.hpp"
#include "smashorpass/util.hpp"
#include "spdlog/spdlog.h"

namespace sop {

Renderer::ScopedClip::ScopedClip(Renderer& renderer) : m_Renderer(&renderer) {}

Renderer::ScopedClip::~ScopedClip() {
    if (m_Renderer) {
        auto result = m_Renderer->PopClipRect();
        if (!result) {
            spdlog::error("Failed to pop renderer clip rect: {}", result.error());
        }
    }
}

Renderer::ScopedClip::ScopedClip(ScopedClip&& other) noexcept : m_Renderer(other.m_Renderer) {
    other.m_Renderer = nullptr;
}

Renderer::ScopedClip& Renderer::ScopedClip::operator=(ScopedClip&& other) noexcept {
    if (this == &other)
        return *this;

    if (m_Renderer) {
        auto result = m_Renderer->PopClipRect();
        if (!result) {
            spdlog::error("Failed to pop renderer clip rect: {}", result.error());
        }
    }

    m_Renderer = other.m_Renderer;
    other.m_Renderer = nullptr;
    return *this;
}

Result<void> Renderer::Initialize(Window& window, const char* driverName) {
    // Assuming that index 0 is the main gpu-accelerated driver, but allow overriding it with
    // driverName if provided.
    driverName = driverName ? driverName : SDL_GetRenderDriver(0);
    if (driverName == nullptr) {
        return Err(SdlError("SDL_GetRenderDriver"));
    }

    m_NativeHandle = SDL_CreateRenderer(window.NativeHandle(), driverName);
    if (m_NativeHandle == nullptr) {
        return Err(SdlError("SDL_CreateRenderer"));
    }

    // Apparently good default for most UI/game rendering.
    if (!SDL_SetRenderDrawBlendMode(m_NativeHandle, SDL_BLENDMODE_BLEND)) {
        const std::string error = SdlError("SDL_SetRenderDrawBlendMode");
        SDL_DestroyRenderer(m_NativeHandle);
        m_NativeHandle = nullptr;
        return Err(error);
    }

    m_TitleFont = TTF_OpenFont(
        (std::string(SOP_ASSET_ROOT_DIR) + "/fonts/Oxanium/static/Oxanium-ExtraBold.ttf").c_str(),
        64.0f);
    m_BigFont = TTF_OpenFont(
        (std::string(SOP_ASSET_ROOT_DIR) + "/fonts/Oxanium/static/Oxanium-SemiBold.ttf").c_str(),
        32.0f);
    m_MediumFont = TTF_OpenFont(
        (std::string(SOP_ASSET_ROOT_DIR) + "/fonts/Oxanium/static/Oxanium-Bold.ttf").c_str(),
        32.0f);
    m_SmallFont = TTF_OpenFont(
        (std::string(SOP_ASSET_ROOT_DIR) + "/fonts/Oxanium/static/Oxanium-Regular.ttf").c_str(),
        24.0f);

    if (m_TitleFont == nullptr || m_BigFont == nullptr || m_MediumFont == nullptr ||
        m_SmallFont == nullptr) {
        const std::string error = SdlError("TTF_OpenFont");
        if (m_TitleFont != nullptr) {
            TTF_CloseFont(m_TitleFont);
        }
        if (m_BigFont != nullptr) {
            TTF_CloseFont(m_BigFont);
        }
        if (m_MediumFont != nullptr) {
            TTF_CloseFont(m_MediumFont);
        }
        if (m_SmallFont != nullptr) {
            TTF_CloseFont(m_SmallFont);
        }
        m_TitleFont = nullptr;
        m_BigFont = nullptr;
        m_MediumFont = nullptr;
        m_SmallFont = nullptr;
        SDL_DestroyRenderer(m_NativeHandle);
        m_NativeHandle = nullptr;
        return Err(error);
    }

    return Ok();
}

Renderer::~Renderer() {
    if (m_TitleFont != nullptr) {
        TTF_CloseFont(m_TitleFont);
    }
    if (m_BigFont != nullptr) {
        TTF_CloseFont(m_BigFont);
    }
    if (m_MediumFont != nullptr) {
        TTF_CloseFont(m_MediumFont);
    }
    if (m_SmallFont != nullptr) {
        TTF_CloseFont(m_SmallFont);
    }
    m_TitleFont = nullptr;
    m_BigFont = nullptr;
    m_MediumFont = nullptr;
    m_SmallFont = nullptr;

    if (m_NativeHandle) {
        SDL_DestroyRenderer(m_NativeHandle);
        m_NativeHandle = nullptr;
    }
}

Result<void> Renderer::BeginFrame(Color clear) {
    return Clear(clear);
}

Result<void> Renderer::EndFrame() {
    return SdlResult(SDL_RenderPresent(m_NativeHandle), "SDL_RenderPresent");
}

Result<void> Renderer::Flush() {
    return SdlResult(SDL_FlushRenderer(m_NativeHandle), "SDL_FlushRenderer");
}

Result<void> Renderer::SetVSync(bool enabled) {
    return SdlResult(SDL_SetRenderVSync(m_NativeHandle, enabled ? 1 : 0), "SDL_SetRenderVSync");
}

Result<bool> Renderer::IsVSync() const {
    int vsync = 0;
    TRY_VOID(SdlResult(SDL_GetRenderVSync(m_NativeHandle, &vsync), "SDL_GetRenderVSync"));
    return Ok(vsync != 0);
}

Result<void> Renderer::SetLogicalPresentation(int width,
                                              int height,
                                              SDL_RendererLogicalPresentation mode) {
    return SdlResult(SDL_SetRenderLogicalPresentation(m_NativeHandle, width, height, mode),
                     "SDL_SetRenderLogicalPresentation");
}

Result<void> Renderer::GetLogicalPresentation(int& width,
                                              int& height,
                                              SDL_RendererLogicalPresentation& mode) const {
    return SdlResult(SDL_GetRenderLogicalPresentation(m_NativeHandle, &width, &height, &mode),
                     "SDL_GetRenderLogicalPresentation");
}

Result<SDL_FRect> Renderer::GetLogicalPresentationRect() const {
    SDL_FRect rect{};
    TRY_VOID(SdlResult(SDL_GetRenderLogicalPresentationRect(m_NativeHandle, &rect),
                       "SDL_GetRenderLogicalPresentationRect"));
    return Ok(rect);
}

Result<void> Renderer::SetViewport(std::optional<SDL_Rect> rect) {
    return SdlResult(SDL_SetRenderViewport(m_NativeHandle, rect ? &*rect : nullptr),
                     "SDL_SetRenderViewport");
}

Result<std::optional<SDL_Rect>> Renderer::GetViewport() const {
    if (!SDL_RenderViewportSet(m_NativeHandle))
        return Ok(std::optional<SDL_Rect>{});

    SDL_Rect rect{};
    TRY_VOID(SdlResult(SDL_GetRenderViewport(m_NativeHandle, &rect), "SDL_GetRenderViewport"));
    return Ok(std::optional<SDL_Rect>{rect});
}

Result<void> Renderer::SetClipRect(std::optional<SDL_Rect> rect) {
    return SdlResult(SDL_SetRenderClipRect(m_NativeHandle, rect ? &*rect : nullptr),
                     "SDL_SetRenderClipRect");
}

Result<void> Renderer::PushClipRect(std::optional<SDL_Rect> rect) {
    m_ClipStack.push_back(rect);
    auto result = ApplyClipStack();
    if (!result) {
        m_ClipStack.pop_back();
        return result;
    }
    return Ok();
}

Result<void> Renderer::PopClipRect() {
    if (m_ClipStack.empty())
        return Ok();

    m_ClipStack.pop_back();
    return ApplyClipStack();
}

Result<void> Renderer::SetScale(float x, float y) {
    return SdlResult(SDL_SetRenderScale(m_NativeHandle, x, y), "SDL_SetRenderScale");
}

Result<void> Renderer::ApplyDisplayScale(float displayScale) {
    const float scale = NormalizeDisplayScale(displayScale);
    return SetScale(scale, scale);
}

Result<SDL_FPoint> Renderer::GetScale() const {
    SDL_FPoint p{};
    TRY_VOID(SdlResult(SDL_GetRenderScale(m_NativeHandle, &p.x, &p.y), "SDL_GetRenderScale"));
    return Ok(p);
}

Result<void> Renderer::SetTarget(SDL_Texture* target) {
    return SdlResult(SDL_SetRenderTarget(m_NativeHandle, target), "SDL_SetRenderTarget");
}

Result<void> Renderer::ResetTarget() {
    return SdlResult(SDL_SetRenderTarget(m_NativeHandle, nullptr), "SDL_SetRenderTarget");
}

SDL_Texture* Renderer::GetTarget() const {
    return SDL_GetRenderTarget(m_NativeHandle);
}

Result<SDL_Point> Renderer::GetOutputSize() const {
    SDL_Point p{};
    TRY_VOID(
        SdlResult(SDL_GetRenderOutputSize(m_NativeHandle, &p.x, &p.y), "SDL_GetRenderOutputSize"));
    return Ok(p);
}

Result<SDL_Point> Renderer::GetCurrentOutputSize() const {
    SDL_Point p{};
    TRY_VOID(SdlResult(SDL_GetCurrentRenderOutputSize(m_NativeHandle, &p.x, &p.y),
                       "SDL_GetCurrentRenderOutputSize"));
    return Ok(p);
}

Result<SDL_FPoint> Renderer::GetLogicalOutputSize() const {
    TRY(outputSize, GetCurrentOutputSize());
    TRY(scale, GetScale());
    return Ok(SDL_FPoint{static_cast<float>(outputSize.x) / NormalizeDisplayScale(scale.x),
                         static_cast<float>(outputSize.y) / NormalizeDisplayScale(scale.y)});
}

Result<SDL_Rect> Renderer::GetSafeArea() const {
    SDL_Rect r{};
    TRY_VOID(SdlResult(SDL_GetRenderSafeArea(m_NativeHandle, &r), "SDL_GetRenderSafeArea"));
    return Ok(r);
}

Result<void> Renderer::WindowToRender(float windowX,
                                      float windowY,
                                      float& renderX,
                                      float& renderY) const {
    return SdlResult(
        SDL_RenderCoordinatesFromWindow(m_NativeHandle, windowX, windowY, &renderX, &renderY),
        "SDL_RenderCoordinatesFromWindow");
}

Result<void> Renderer::RenderToWindow(float renderX,
                                      float renderY,
                                      float& windowX,
                                      float& windowY) const {
    return SdlResult(
        SDL_RenderCoordinatesToWindow(m_NativeHandle, renderX, renderY, &windowX, &windowY),
        "SDL_RenderCoordinatesToWindow");
}

Result<void> Renderer::ConvertEventToRenderCoordinates(SDL_Event& event) const {
    return SdlResult(SDL_ConvertEventToRenderCoordinates(m_NativeHandle, &event),
                     "SDL_ConvertEventToRenderCoordinates");
}

Result<void> Renderer::Clear(Color color) {
    TRY_VOID(SetDrawColor(color));
    return SdlResult(SDL_RenderClear(m_NativeHandle), "SDL_RenderClear");
}

Result<void> Renderer::SetDrawColor(Color color) {
    return SdlResult(SDL_SetRenderDrawColor(m_NativeHandle, color.r, color.g, color.b, color.a),
                     "SDL_SetRenderDrawColor");
}

Result<Color> Renderer::GetDrawColor() const {
    Color c{};
    TRY_VOID(SdlResult(SDL_GetRenderDrawColor(m_NativeHandle, &c.r, &c.g, &c.b, &c.a),
                       "SDL_GetRenderDrawColor"));
    return Ok(c);
}

Result<void> Renderer::SetBlendMode(SDL_BlendMode blendMode) {
    return SdlResult(SDL_SetRenderDrawBlendMode(m_NativeHandle, blendMode),
                     "SDL_SetRenderDrawBlendMode");
}

Result<SDL_BlendMode> Renderer::GetBlendMode() const {
    SDL_BlendMode mode{};
    TRY_VOID(
        SdlResult(SDL_GetRenderDrawBlendMode(m_NativeHandle, &mode), "SDL_GetRenderDrawBlendMode"));
    return Ok(mode);
}

Result<void> Renderer::DrawPoint(float x, float y, Color color) {
    TRY_VOID(SetDrawColor(color));
    return SdlResult(SDL_RenderPoint(m_NativeHandle, x, y), "SDL_RenderPoint");
}

Result<void> Renderer::DrawLine(float x1, float y1, float x2, float y2, Color color) {
    TRY_VOID(SetDrawColor(color));
    return SdlResult(SDL_RenderLine(m_NativeHandle, x1, y1, x2, y2), "SDL_RenderLine");
}

Result<void> Renderer::DrawLines(std::span<const SDL_FPoint> points, Color color) {
    if (points.empty())
        return Ok();

    TRY_VOID(SetDrawColor(color));

    return SdlResult(
        SDL_RenderLines(m_NativeHandle, points.data(), static_cast<int>(points.size())),
        "SDL_RenderLines");
}

Result<void> Renderer::DrawRect(const SDL_FRect& rect, Color color) {
    TRY_VOID(SetDrawColor(color));
    return SdlResult(SDL_RenderRect(m_NativeHandle, &rect), "SDL_RenderRect");
}

Result<void> Renderer::DrawRects(std::span<const SDL_FRect> rects, Color color) {
    if (rects.empty())
        return Ok();

    TRY_VOID(SetDrawColor(color));

    return SdlResult(SDL_RenderRects(m_NativeHandle, rects.data(), static_cast<int>(rects.size())),
                     "SDL_RenderRects");
}

Result<void> Renderer::FillRect(const SDL_FRect& rect, Color color) {
    TRY_VOID(SetDrawColor(color));
    return SdlResult(SDL_RenderFillRect(m_NativeHandle, &rect), "SDL_RenderFillRect");
}

Result<void> Renderer::FillRects(std::span<const SDL_FRect> rects, Color color) {
    if (rects.empty())
        return Ok();

    TRY_VOID(SetDrawColor(color));

    return SdlResult(
        SDL_RenderFillRects(m_NativeHandle, rects.data(), static_cast<int>(rects.size())),
        "SDL_RenderFillRects");
}

Result<void> Renderer::DrawTexture(SDL_Texture* texture, const SDL_FRect& dst) {
    TextureDrawParams params{};
    params.dst = dst;
    return DrawTexture(texture, params);
}

Result<void> Renderer::DrawTexture(SDL_Texture* texture, const TextureDrawParams& params) {
    if (!texture)
        return Err(std::string("Renderer::DrawTexture failed: texture is null"));

    TRY(backup, BackupTextureState(texture));
    auto stateResult = ApplyTextureState(texture, params.tint, params.blendMode);
    if (!stateResult) {
        TRY_VOID(RestoreTextureState(texture, backup));
        return Err(std::move(stateResult).error());
    }

    const bool usesSimpleDraw = params.rotationDeg == 0.0 && params.flip == SDL_FLIP_NONE;
    const char* operation = usesSimpleDraw ? "SDL_RenderTexture" : "SDL_RenderTextureRotated";
    const bool ok = usesSimpleDraw
                        ? SDL_RenderTexture(m_NativeHandle, texture, params.src, &params.dst)
                        : SDL_RenderTextureRotated(m_NativeHandle,
                                                   texture,
                                                   params.src,
                                                   &params.dst,
                                                   params.rotationDeg,
                                                   &params.origin,
                                                   params.flip);

    if (!ok) {
        std::string renderError = SdlError(operation);
        TRY_VOID(RestoreTextureState(texture, backup));
        return Err(std::move(renderError));
    }

    TRY_VOID(RestoreTextureState(texture, backup));
    return Ok();
}

Result<void> Renderer::DrawTextureTiled(SDL_Texture* texture,
                                        const TiledTextureDrawParams& params) {
    if (!texture)
        return Err(std::string("Renderer::DrawTextureTiled failed: texture is null"));

    TRY(backup, BackupTextureState(texture));
    auto stateResult = ApplyTextureState(texture, params.tint, params.blendMode);
    if (!stateResult) {
        TRY_VOID(RestoreTextureState(texture, backup));
        return Err(std::move(stateResult).error());
    }

    const bool ok =
        SDL_RenderTextureTiled(m_NativeHandle, texture, params.src, params.scale, &params.dst);

    if (!ok) {
        std::string renderError = SdlError("SDL_RenderTextureTiled");
        TRY_VOID(RestoreTextureState(texture, backup));
        return Err(std::move(renderError));
    }

    TRY_VOID(RestoreTextureState(texture, backup));
    return Ok();
}

Result<void> Renderer::DrawTexture9Grid(SDL_Texture* texture, const NineGridDrawParams& params) {
    if (!texture)
        return Err(std::string("Renderer::DrawTexture9Grid failed: texture is null"));

    TRY(backup, BackupTextureState(texture));
    auto stateResult = ApplyTextureState(texture, params.tint, params.blendMode);
    if (!stateResult) {
        TRY_VOID(RestoreTextureState(texture, backup));
        return Err(std::move(stateResult).error());
    }

    const bool ok = SDL_RenderTexture9Grid(m_NativeHandle,
                                           texture,
                                           params.src,
                                           params.leftWidth,
                                           params.rightWidth,
                                           params.topHeight,
                                           params.bottomHeight,
                                           params.scale,
                                           &params.dst);

    if (!ok) {
        std::string renderError = SdlError("SDL_RenderTexture9Grid");
        TRY_VOID(RestoreTextureState(texture, backup));
        return Err(std::move(renderError));
    }

    TRY_VOID(RestoreTextureState(texture, backup));
    return Ok();
}

Result<void> Renderer::DrawGeometry(SDL_Texture* texture,
                                    std::span<const SDL_Vertex> vertices,
                                    std::span<const int> indices) {
    if (vertices.empty())
        return Ok();

    return SdlResult(SDL_RenderGeometry(m_NativeHandle,
                                        texture,
                                        vertices.data(),
                                        static_cast<int>(vertices.size()),
                                        indices.empty() ? nullptr : indices.data(),
                                        static_cast<int>(indices.size())),
                     "SDL_RenderGeometry");
}

Result<void> Renderer::DrawText(FontId id, float x, float y, std::string_view text, Color color) {
    std::string owned(text);

    TTF_Font* font = GetFontById(id);
    if (!font)
        return Err(std::string("Renderer::DrawText failed: font is null"));

    SDL_Surface* surface = TTF_RenderText_Blended(font, text.data(), 0, color);
    if (!surface)
        return Err(SdlError("TTF_RenderText_Blended"));

    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_NativeHandle, surface);
    if (!texture) {
        SDL_DestroySurface(surface);
        return Err(SdlError("SDL_CreateTextureFromSurface"));
    }

    SDL_FRect dst{x, y, static_cast<float>(surface->w), static_cast<float>(surface->h)};
    bool ok = SDL_RenderTexture(m_NativeHandle, texture, nullptr, &dst);

    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);

    return SdlResult(ok, "SDL_RenderTexture");
}

Result<SDL_Surface*> Renderer::ReadPixels(const SDL_Rect* rect) const {
    SDL_Surface* surface = SDL_RenderReadPixels(m_NativeHandle, rect);
    if (!surface)
        return Err(SdlError("SDL_RenderReadPixels"));
    return Ok(surface);
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

Result<void> Renderer::ApplyClipStack() {
    std::optional<SDL_Rect> effective;
    for (const auto& clip : m_ClipStack)
        effective = Intersect(effective, clip);

    return SdlResult(SDL_SetRenderClipRect(m_NativeHandle, effective ? &*effective : nullptr),
                     "SDL_SetRenderClipRect");
}

Result<Renderer::TextureStateBackup> Renderer::BackupTextureState(SDL_Texture* texture) const {
    TextureStateBackup backup{};
    TRY_VOID(SdlResult(SDL_GetTextureColorMod(texture, &backup.r, &backup.g, &backup.b),
                       "SDL_GetTextureColorMod"));
    TRY_VOID(SdlResult(SDL_GetTextureAlphaMod(texture, &backup.a), "SDL_GetTextureAlphaMod"));
    TRY_VOID(
        SdlResult(SDL_GetTextureBlendMode(texture, &backup.blendMode), "SDL_GetTextureBlendMode"));
    return Ok(backup);
}

Result<void> Renderer::RestoreTextureState(SDL_Texture* texture,
                                           const TextureStateBackup& backup) const {
    TRY_VOID(SdlResult(SDL_SetTextureColorMod(texture, backup.r, backup.g, backup.b),
                       "SDL_SetTextureColorMod"));
    TRY_VOID(SdlResult(SDL_SetTextureAlphaMod(texture, backup.a), "SDL_SetTextureAlphaMod"));
    TRY_VOID(
        SdlResult(SDL_SetTextureBlendMode(texture, backup.blendMode), "SDL_SetTextureBlendMode"));
    return Ok();
}

Result<void> Renderer::ApplyTextureState(SDL_Texture* texture,
                                         Color tint,
                                         SDL_BlendMode blendMode) const {
    TRY_VOID(SdlResult(SDL_SetTextureColorMod(texture, tint.r, tint.g, tint.b),
                       "SDL_SetTextureColorMod"));
    TRY_VOID(SdlResult(SDL_SetTextureAlphaMod(texture, tint.a), "SDL_SetTextureAlphaMod"));
    TRY_VOID(SdlResult(SDL_SetTextureBlendMode(texture, blendMode), "SDL_SetTextureBlendMode"));
    return Ok();
}

Result<Vec2> Renderer::MeasureText(FontId id, std::string_view text) {
    TTF_Font* font = GetFontById(id);

    if (!font)
        return Err(std::string("Renderer::MeasureText failed: font is null"));

    if (text.empty())
        return Ok(Vec2{0.0f, 0.0f});

    int32_t w = 0;
    int32_t h = 0;

    if (!TTF_GetStringSize(font, text.data(), text.size(), &w, &h)) {
        spdlog::warn("TTF_GetStringSize failed: {}", SDL_GetError());
        return Err(SdlError("TTF_GetStringSize"));
    }

    return Ok(Vec2{static_cast<float>(w), static_cast<float>(h)});
}

TTF_Font* Renderer::GetFontById(FontId id) {
    switch (id) {
        case FontId::Title:
            return m_TitleFont;
        case FontId::Body:
            return m_BigFont;
        case FontId::Medium:
            return m_MediumFont;
        case FontId::Small:
            return m_SmallFont;
    }
    return nullptr;
};
}  // namespace sop
