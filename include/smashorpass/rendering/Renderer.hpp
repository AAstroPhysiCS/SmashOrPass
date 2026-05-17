#pragma once

#include "SDL3_ttf/SDL_ttf.h"
#include "smashorpass/core/Base.hpp"
#include "smashorpass/core/Window.hpp"
#include "smashorpass/util.hpp"

namespace sop {

using namespace sop_util;

enum class FontId : uint8_t;

struct TextureDrawParams {
    const SDL_FRect* src{nullptr};
    SDL_FRect dst{};

    double rotationDeg{0.0};
    SDL_FPoint origin{0.0f, 0.0f};
    SDL_FlipMode flip{SDL_FLIP_NONE};

    Color tint{Color::White()};
    SDL_BlendMode blendMode{SDL_BLENDMODE_BLEND};
};

struct TiledTextureDrawParams {
    const SDL_FRect* src{nullptr};
    SDL_FRect dst{};
    float scale{1.0f};

    Color tint{Color::White()};
    SDL_BlendMode blendMode{SDL_BLENDMODE_BLEND};
};

struct NineGridDrawParams {
    const SDL_FRect* src{nullptr};
    SDL_FRect dst{};

    float leftWidth{0.0f};
    float rightWidth{0.0f};
    float topHeight{0.0f};
    float bottomHeight{0.0f};
    float scale{1.0f};

    Color tint{Color::White()};
    SDL_BlendMode blendMode{SDL_BLENDMODE_BLEND};
};

class Renderer final {
   public:
    class ScopedClip final {
       public:
        ~ScopedClip();

        ScopedClip(const ScopedClip&) = delete;
        ScopedClip& operator=(const ScopedClip&) = delete;

        ScopedClip(ScopedClip&& other) noexcept;
        ScopedClip& operator=(ScopedClip&& other) noexcept;

       private:
        explicit ScopedClip(Renderer& renderer);

        Renderer* m_Renderer{nullptr};

        friend class Renderer;
    };

   public:
    Renderer() = default;
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    Result<void> Initialize(Window& window, const char* driverName = nullptr);

    Result<void> BeginFrame(Color clear = Color{18, 18, 24, 255});
    Result<void> EndFrame();
    Result<void> Flush();

    Result<void> SetVSync(bool enabled);
    Result<bool> IsVSync() const;

    Result<void> SetLogicalPresentation(int width,
                                        int height,
                                        SDL_RendererLogicalPresentation mode);
    Result<void> GetLogicalPresentation(int& width,
                                        int& height,
                                        SDL_RendererLogicalPresentation& mode) const;
    Result<SDL_FRect> GetLogicalPresentationRect() const;

    Result<void> SetViewport(std::optional<SDL_Rect> rect);
    Result<std::optional<SDL_Rect>> GetViewport() const;

    Result<void> SetClipRect(std::optional<SDL_Rect> rect);
    Result<void> PushClipRect(std::optional<SDL_Rect> rect);
    Result<void> PopClipRect();
    Result<ScopedClip> Clip(std::optional<SDL_Rect> rect) {
        TRY_VOID(PushClipRect(rect));
        return Ok(ScopedClip(*this));
    }

    Result<void> SetScale(float x, float y);
    Result<void> ApplyDisplayScale(float displayScale);
    Result<SDL_FPoint> GetScale() const;

    Result<void> SetTarget(SDL_Texture* target);
    Result<void> ResetTarget();
    [[nodiscard]] SDL_Texture* GetTarget() const;

    Result<SDL_Point> GetOutputSize() const;
    Result<SDL_Point> GetCurrentOutputSize() const;
    Result<SDL_FPoint> GetLogicalOutputSize() const;
    Result<SDL_Rect> GetSafeArea() const;

    Result<void> WindowToRender(float windowX, float windowY, float& renderX, float& renderY) const;
    Result<void> RenderToWindow(float renderX, float renderY, float& windowX, float& windowY) const;
    Result<void> ConvertEventToRenderCoordinates(SDL_Event& event) const;

    Result<void> Clear(Color color);
    Result<void> SetDrawColor(Color color);
    Result<Color> GetDrawColor() const;
    Result<void> SetBlendMode(SDL_BlendMode blendMode);
    Result<SDL_BlendMode> GetBlendMode() const;

    Result<void> DrawPoint(float x, float y, Color color);
    Result<void> DrawLine(float x1, float y1, float x2, float y2, Color color);
    Result<void> DrawLines(std::span<const SDL_FPoint> points, Color color);

    Result<void> DrawRect(const SDL_FRect& rect, Color color);
    Result<void> DrawRects(std::span<const SDL_FRect> rects, Color color);
    Result<void> FillRect(const SDL_FRect& rect, Color color);
    Result<void> FillRects(std::span<const SDL_FRect> rects, Color color);

    Result<void> DrawTexture(SDL_Texture* texture, const SDL_FRect& dst);
    Result<void> DrawTexture(SDL_Texture* texture, const TextureDrawParams& params);
    Result<void> DrawTextureTiled(SDL_Texture* texture, const TiledTextureDrawParams& params);
    Result<void> DrawTexture9Grid(SDL_Texture* texture, const NineGridDrawParams& params);

    Result<void> DrawGeometry(SDL_Texture* texture,
                              std::span<const SDL_Vertex> vertices,
                              std::span<const int> indices = {});

    Result<void> DrawText(
        FontId id, float x, float y, std::string_view text, Color color = Color::White());

    Result<SDL_Surface*> ReadPixels(const SDL_Rect* rect = nullptr) const;

    [[nodiscard]] inline SDL_Renderer* NativeHandle() const {
        return m_NativeHandle;
    }

   private:
    struct TextureStateBackup {
        Uint8 r{255};
        Uint8 g{255};
        Uint8 b{255};
        Uint8 a{255};
        SDL_BlendMode blendMode{SDL_BLENDMODE_BLEND};
    };

   private:
    static std::optional<SDL_Rect> Intersect(const std::optional<SDL_Rect>& a,
                                             const std::optional<SDL_Rect>& b);

    Result<void> ApplyClipStack();

    Result<TextureStateBackup> BackupTextureState(SDL_Texture* texture) const;
    Result<void> RestoreTextureState(SDL_Texture* texture, const TextureStateBackup& backup) const;
    Result<void> ApplyTextureState(SDL_Texture* texture, Color tint, SDL_BlendMode blendMode) const;

    Result<Vec2> MeasureText(FontId id, std::string_view text);
    TTF_Font* GetFontById(FontId id);

   private:
    SDL_Renderer* m_NativeHandle{nullptr};
    std::vector<std::optional<SDL_Rect>> m_ClipStack;

    TTF_Font* m_TitleFont = nullptr;
    TTF_Font* m_BigFont = nullptr;
    TTF_Font* m_MediumFont = nullptr;
    TTF_Font* m_SmallFont = nullptr;

    friend class UIScreen;  // for MeasureText
};
}  // namespace sop
