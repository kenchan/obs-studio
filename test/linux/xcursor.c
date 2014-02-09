#include <stdlib.h>
#include <stdio.h>
#include <X11/extensions/Xfixes.h>
#include "xcursor.h"

uint32_t *xcursor_pixels(XFixesCursorImage *xc) {
    int size = xc->width * xc->height;
    uint32_t *pixels = bmalloc(size * 4);
    
    // pixel data from XFixes is defined as unsigned long ...
    for (int i = 0; i < size; ++i)
        pixels[i] = (uint32_t) xc->pixels[i];
    
    return pixels;
}

void xcursor_create(xcursor_t *data, XFixesCursorImage *xc) {
    // get cursor data
    data->last_serial = xc->cursor_serial;
    uint32_t *pixels = xcursor_pixels(xc);
    
    // if the pointer has the same size as the last one we can update
    if (data->tex
     && texture_getwidth(data->tex) == xc->width
     && texture_getheight(data->tex) == xc->height) {
        texture_setimage(data->tex, (void **) pixels, xc->width * 4, False);
    }
    else {
        if (data->tex)
            texture_destroy(data->tex);
        
        data->tex = gs_create_texture(
            xc->width, xc->height,
            GS_RGBA, 1,
            (const void **) &pixels,
            GS_DYNAMIC
        );
    }
    bfree(pixels);
}

xcursor_t *xcursor_init(Display *dpy) {
    xcursor_t *data = bmalloc(sizeof(xcursor_t));
    memset(data, 0, sizeof(xcursor_t));
    
    data->dpy = dpy;
    
    return data;
}

void xcursor_destroy(xcursor_t *data) {
    if (data->tex)
        texture_destroy(data->tex);
    bfree(data);
}

void xcursor_render(xcursor_t *data) {
    // get cursor data
    XFixesCursorImage *xc = XFixesGetCursorImage(data->dpy);
    
    if (data->last_serial != xc->cursor_serial)
        xcursor_create(data, xc);
    
    // TODO: why do i need effects ?
    effect_t effect  = gs_geteffect();
    eparam_t diffuse = effect_getparambyname(effect, "diffuse");

    effect_settexture(effect, diffuse, data->tex);
    
    gs_matrix_push();
    gs_matrix_translate3f(
        -1.0 * (xc->x - xc->xhot),
        -1.0 * (xc->y - xc->yhot),
        0
    );
    gs_enable_blending(True);
    gs_blendfunction(GS_BLEND_ONE, GS_BLEND_INVSRCALPHA);
    gs_draw_sprite(data->tex, 0, 0, 0);
    gs_enable_blending(False);
    gs_matrix_pop();
    
    XFree(xc);
}