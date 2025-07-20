#include "plib/gnw/svga.h"

#include "plib/gnw/gnw.h"
#include "plib/gnw/grbuf.h"
#include "plib/gnw/mouse.h"
#include "plib/gnw/winmain.h"
#include <hal/video.h>
namespace fallout {


// screen rect
Rect scr_size;

// 0x6ACA18
ScreenBlitFunc* scr_blit = GNW95_ShowRect;

SDL_Window* gSdlWindow = NULL;
SDL_Surface* gSdlSurface = NULL;
SDL_Renderer* gSdlRenderer = NULL;
SDL_Texture* gSdlTexture = NULL;
//SDL_Surface* gSdlTextureSurface = NULL;

// TODO: Remove once migration to update-render cycle is completed.
FpsLimiter sharedFpsLimiter;

void GNW95_SetPaletteEntries(unsigned char* palette, int start, int count)
{
    if (gSdlSurface && gSdlSurface->format->palette) {
        SDL_Color colors[256];
        for (int i = 0; i < count; i++) {
            colors[i].r = palette[i * 3 + 0] << 2;
            colors[i].g = palette[i * 3 + 1] << 2;
            colors[i].b = palette[i * 3 + 2] << 2;
            colors[i].a = 255;
        }
        SDL_SetPaletteColors(gSdlSurface->format->palette, colors, start, count);
    }
}

void GNW95_SetPalette(unsigned char* palette)
{
    if (gSdlSurface && gSdlSurface->format->palette) {
        SDL_Color colors[256];
        for (int i = 0; i < 256; i++) {
            colors[i].r = palette[i * 3 + 0] << 2;
            colors[i].g = palette[i * 3 + 1] << 2;
            colors[i].b = palette[i * 3 + 2] << 2;
            colors[i].a = 255;
        }
        SDL_SetPaletteColors(gSdlSurface->format->palette, colors, 0, 256);
    }
}


// 0x4CB850
void GNW95_ShowRect(unsigned char* src, unsigned int srcPitch, unsigned int a3, unsigned int srcX, unsigned int srcY, unsigned int srcWidth, unsigned int srcHeight, unsigned int destX, unsigned int destY)
{
    buf_to_buf(src + srcPitch * srcY + srcX, srcWidth, srcHeight, srcPitch, (unsigned char*)gSdlSurface->pixels + gSdlSurface->pitch * destY + destX, gSdlSurface->pitch);

    SDL_Rect srcRect;
    srcRect.x = destX;
    srcRect.y = destY;
    srcRect.w = srcWidth;
    srcRect.h = srcHeight;

    SDL_Rect destRect;
    destRect.x = destX;
    destRect.y = destY;
    //SDL_BlitSurface(gSdlSurface, &srcRect, gSdlTextureSurface, &destRect);
}
bool svga_init(VideoOptions* video_options)
{
    int width = video_options->width;
    int height = video_options->height;
    int scaled_width = width * video_options->scale;
    int scaled_height = height * video_options->scale;

    // Step 0: Set Xbox framebuffer video mode
    XVideoSetMode(scaled_width, scaled_height, 32, REFRESH_DEFAULT);
    // DbgPrint("svga_init: XVideoSetMode done (%dx%d)\n", scaled_width, scaled_height);

    // [SDL_Init is now done in game_init()]
    // Do NOT call SDL_Init here.

    // Step 1: Create window
    Uint32 windowFlags = SDL_WINDOW_SHOWN;
    if (video_options->fullscreen) {
        windowFlags |= SDL_WINDOW_FULLSCREEN;
    }

    gSdlWindow = SDL_CreateWindow("Fallout 1 CE",
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  scaled_width,
                                  scaled_height,
                                  windowFlags);
    if (!gSdlWindow) {
        // DbgPrint("svga_init: SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

    // Step 2: Create software renderer
    gSdlRenderer = SDL_CreateRenderer(gSdlWindow, -1, SDL_RENDERER_SOFTWARE);
    if (!gSdlRenderer) {
        // DbgPrint("svga_init: SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(gSdlWindow);
        gSdlWindow = nullptr;
        return false;
    }

    // Step 3: Create 8-bit palettized surface for Fallout's framebuffer
    gSdlSurface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 8, SDL_PIXELFORMAT_INDEX8);
    if (!gSdlSurface) {
        // DbgPrint("svga_init: SDL_CreateRGBSurface failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(gSdlRenderer);
        SDL_DestroyWindow(gSdlWindow);
        gSdlRenderer = nullptr;
        gSdlWindow = nullptr;
        return false;
    }

    // Step 4: Create streaming texture (ARGB8888 format for final blit)
    gSdlTexture = SDL_CreateTexture(gSdlRenderer,
                                    SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_STREAMING,
                                    width,
                                    height);
    if (!gSdlTexture) {
        // DbgPrint("svga_init: SDL_CreateTexture failed: %s\n", SDL_GetError());
        SDL_FreeSurface(gSdlSurface);
        SDL_DestroyRenderer(gSdlRenderer);
        SDL_DestroyWindow(gSdlWindow);
        gSdlSurface = nullptr;
        gSdlRenderer = nullptr;
        gSdlWindow = nullptr;
        return false;
    }

    // Step 5: Set screen dimensions
    scr_size.ulx = 0;
    scr_size.uly = 0;
    scr_size.lrx = width - 1;
    scr_size.lry = height - 1;

    // Step 6: Assign blit functions
    mouse_blit_trans = nullptr;
    scr_blit = GNW95_ShowRect;
    mouse_blit = GNW95_ShowRect;

    // DbgPrint("svga_init: completed successfully\n");
    return true;
}


void svga_exit()
{
    if (gSdlTexture) {
        SDL_DestroyTexture(gSdlTexture);
        gSdlTexture = NULL;
    }

    if (gSdlRenderer) {
        SDL_DestroyRenderer(gSdlRenderer);
        gSdlRenderer = NULL;
    }

    if (gSdlSurface) {
        SDL_FreeSurface(gSdlSurface);
        gSdlSurface = NULL;
    }

    if (gSdlWindow) {
        SDL_DestroyWindow(gSdlWindow);
        gSdlWindow = NULL;
    }

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


int screenGetWidth()
{
    // TODO: Make it on par with _xres;
    return rectGetWidth(&scr_size);
}

int screenGetHeight()
{
    // TODO: Make it on par with _yres.
    return rectGetHeight(&scr_size);
}

void handleWindowSizeChanged()
{
    //destroyRenderer();
    //createRenderer(screenGetWidth(), screenGetHeight());
}
void renderPresent() {
    if (!gSdlSurface || !gSdlTexture || !gSdlRenderer) return;

    SDL_Surface* converted = SDL_ConvertSurfaceFormat(gSdlSurface, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_UpdateTexture(gSdlTexture, NULL, converted->pixels, converted->pitch);
    SDL_FreeSurface(converted);

    SDL_RenderClear(gSdlRenderer);
    SDL_RenderCopy(gSdlRenderer, gSdlTexture, NULL, NULL);
    SDL_RenderPresent(gSdlRenderer);
}




} // namespace fallout
