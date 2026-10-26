#ifndef KART_TEXT_HPP
#define KART_TEXT_HPP
#include <touchgfx/Drawable.hpp>
#include <touchgfx/Color.hpp>
// Dibuja texto con la fuente 5x7 propia (celda de 12 px de ancho, puntos de 2x2).
// x,y son coordenadas del widget propietario; clip ya esta en coordenadas absolutas.
void kartDrawText(const touchgfx::Drawable& owner,const touchgfx::Rect& clip,int x,int y,const char* text,touchgfx::colortype color);
#endif
