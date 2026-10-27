#ifndef WOW_SOURCE_WORLDCLIENT_MODERNRENDER_H
#define WOW_SOURCE_WORLDCLIENT_MODERNRENDER_H

class CRect;

namespace ModernRender {
void Begin();
void AfterOpaque();
void End(const CRect& viewportNDC);
}

#endif
