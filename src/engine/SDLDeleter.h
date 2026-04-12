#pragma once

#include <SDL3/SDL.h>
#include <memory>

struct SDLDeleter {
    void operator()(SDL_Window* window) const {
        if (window) {
            SDL_DestroyWindow(window);
        }
    }
    
    void operator()(SDL_Renderer* renderer) const {
        if (renderer) {
            SDL_DestroyRenderer(renderer);
        }
    }
    
    void operator()(SDL_Texture* texture) const {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
    
    void operator()(SDL_Surface* surface) const {
        if (surface) {
            SDL_DestroySurface(surface);
        }
    }
};

template<typename T>
using SDLPtr = std::unique_ptr<T, SDLDeleter>;

using WindowPtr = SDLPtr<SDL_Window>;
using RendererPtr = SDLPtr<SDL_Renderer>;
using TexturePtr = SDLPtr<SDL_Texture>;
using SurfacePtr = SDLPtr<SDL_Surface>;