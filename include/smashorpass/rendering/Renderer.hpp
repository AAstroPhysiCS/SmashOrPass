#pragma once

#include "smashorpass/core/Base.hpp"
#include "smashorpass/core/DisplayMetrics.hpp"
#include "smashorpass/platform/Window.hpp"

#include "SDL3_ttf/SDL_ttf.h"

namespace sop {

enum class FontId : uint8_t;

// TODO: use glm, delete THIS only temporary
struct Vec2 {
    float x;
    float y;
};

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
        ScopedClip(Renderer& renderer, std::optional<SDL_Rect> rect);
        ~ScopedClip();

        ScopedClip(const ScopedClip&) = delete;
        ScopedClip& operator=(const ScopedClip&) = delete;

        ScopedClip(ScopedClip&& other) noexcept;
        ScopedClip& operator=(ScopedClip&& other) noexcept;

       private:
        Renderer* m_Renderer{nullptr};
    };

   public:
    explicit Renderer(Window& window, const char* driverName = nullptr);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    void BeginFrame(Color clear = Color{18, 18, 24, 255});
    void EndFrame();
    void Flush();

    bool SetVSync(bool enabled);
    [[nodiscard]] bool IsVSync() const;

    bool SetLogicalPresentation(int width, int height, SDL_RendererLogicalPresentation mode);
    [[nodiscard]] bool GetLogicalPresentation(int& width,
                                              int& height,
                                              SDL_RendererLogicalPresentation& mode) const;
    [[nodiscard]] SDL_FRect GetLogicalPresentationRect() const;

    bool SetViewport(std::optional<SDL_Rect> rect);
    [[nodiscard]] std::optional<SDL_Rect> GetViewport() const;

    bool SetClipRect(std::optional<SDL_Rect> rect);
    void PushClipRect(std::optional<SDL_Rect> rect);
    void PopClipRect();
    [[nodiscard]] ScopedClip Clip(std::optional<SDL_Rect> rect) {
        return ScopedClip(*this, rect);
    }

    bool SetScale(float x, float y);
    bool ApplyDisplayScale(float displayScale);
    [[nodiscard]] SDL_FPoint GetScale() const;

    bool SetTarget(SDL_Texture* target);
    void ResetTarget();
    [[nodiscard]] SDL_Texture* GetTarget() const;

    [[nodiscard]] SDL_Point GetOutputSize() const;
    [[nodiscard]] SDL_Point GetCurrentOutputSize() const;
    [[nodiscard]] SDL_FPoint GetLogicalOutputSize() const;
    [[nodiscard]] SDL_Rect GetSafeArea() const;

    bool WindowToRender(float windowX, float windowY, float& renderX, float& renderY) const;
    bool RenderToWindow(float renderX, float renderY, float& windowX, float& windowY) const;
    bool ConvertEventToRenderCoordinates(SDL_Event& event) const;

    bool Clear(Color color);
    bool SetDrawColor(Color color);
    [[nodiscard]] Color GetDrawColor() const;
    bool SetBlendMode(SDL_BlendMode blendMode);
    [[nodiscard]] SDL_BlendMode GetBlendMode() const;

    bool DrawPoint(float x, float y, Color color);
    bool DrawLine(float x1, float y1, float x2, float y2, Color color);
    bool DrawLines(std::span<const SDL_FPoint> points, Color color);

    bool DrawRect(const SDL_FRect& rect, Color color);
    bool DrawRects(std::span<const SDL_FRect> rects, Color color);
    bool FillRect(const SDL_FRect& rect, Color color);
    bool FillRects(std::span<const SDL_FRect> rects, Color color);

    bool DrawTexture(SDL_Texture* texture, const SDL_FRect& dst);
    bool DrawTexture(SDL_Texture* texture, const TextureDrawParams& params);
    bool DrawTextureTiled(SDL_Texture* texture, const TiledTextureDrawParams& params);
    bool DrawTexture9Grid(SDL_Texture* texture, const NineGridDrawParams& params);

    bool DrawGeometry(SDL_Texture* texture,
                      std::span<const SDL_Vertex> vertices,
                      std::span<const int> indices = {});

    bool DrawText(FontId id, float x, float y, std::string_view text, Color color = Color::White());

    [[nodiscard]] SDL_Surface* ReadPixels(const SDL_Rect* rect = nullptr) const;

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

    void ApplyClipStack();

    TextureStateBackup BackupTextureState(SDL_Texture* texture) const;
    void RestoreTextureState(SDL_Texture* texture, const TextureStateBackup& backup) const;
    void ApplyTextureState(SDL_Texture* texture, Color tint, SDL_BlendMode blendMode) const;

    Vec2 MeasureText(FontId id, std::string_view text);
    TTF_Font* GetFontById(FontId id);
   private:
    Window& m_Window;
    SDL_Renderer* m_NativeHandle{nullptr};
    std::vector<std::optional<SDL_Rect>> m_ClipStack;

    TTF_Font* m_TitleFont = nullptr;
    TTF_Font* m_BigFont = nullptr;
    TTF_Font* m_MediumFont = nullptr;
    TTF_Font* m_SmallFont = nullptr;

    friend class UIScreen; //for MeasureText
};
}  // namespace sop
