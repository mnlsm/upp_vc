#ifndef DRAW_H
#define DRAW_H

#define SYSTEMDRAW 1

#include <Core/Core.h>



NAMESPACE_UPP

class Drawing;
class Draw;
class Painting;
class SystemDraw;
class ImageDraw;

#ifdef _MULTITHREADED
void EnterGMutex();
void EnterGMutex(int n);
void LeaveGMutex();
int  LeaveGMutexAll();

struct DrawLock {
    DrawLock() {
        EnterGMutex();
    }
    ~DrawLock() {
        LeaveGMutex();
    }
};
#else
inline void EnterGMutex() {}
inline void EnterGMutex(int n) {}
inline void LeaveGMutex() {}
inline int  LeaveGMutexAll() {
    return 0;
}

struct DrawLock {
    DrawLock() {}
    ~DrawLock() {}
};
#endif

#include "Image.h"

const int FONT_V = 40;

#include "FontInt.h"

class FontInfo;

class Font : AssignValueTypeNo<Font, FONT_V, Moveable<Font> > {
    union {
        int64 data;
        struct {
            word  face;
            word  flags;
            int16 height;
            int16 width;
        } v;
    };

    static Font AStdFont;
    static Size StdFontSize;

    static const Vector<FaceInfo>& List();
    static void SyncStdFont();
    static void InitStdFont();

    const CommonFontInfo& Fi() const;

    friend void sInitFonts();

public:
    enum {
        FIXEDPITCH  = 0x0001,
        SCALEABLE   = 0x0002,
        SPECIAL     = 0x0010,
    };

    static int    GetFaceCount();
    static String GetFaceName(int index);
    static int    FindFaceNameIndex(const String& name);
    static dword  GetFaceInfo(int index);

    static void   SetStdFont(Font font);
    static Font   GetStdFont();
    static Size   GetStdFontSize();

    enum {
        STDFONT,
        SERIF,
        SANSSERIF,
        MONOSPACE,
        SYMBOL,
        WINGDINGS,
        TAHOMA,
        OTHER,

        // Backward compatibility:
        ROMAN = SERIF,
        ARIAL = SANSSERIF,
        COURIER = MONOSPACE,
        SCREEN_SERIF = SERIF,
        SCREEN_SANS = SANSSERIF,
        SCREEN_FIXED = MONOSPACE,
    };

    int    GetFace() const {
        return v.face;
    }
    int    GetHeight() const;
    int    GetWidth() const {
        return v.width;
    }
    bool   IsBold() const {
        return v.flags & 0x8000;
    }
    bool   IsItalic() const {
        return v.flags & 0x4000;
    }
    bool   IsUnderline() const {
        return v.flags & 0x2000;
    }
    bool   IsStrikeout() const {
        return v.flags & 0x1000;
    }
    bool   IsNonAntiAliased() const {
        return v.flags & 0x800;
    }
    bool   IsTrueTypeOnly() const {
        return v.flags & 0x400;
    }
    String GetFaceName() const;
    dword  GetFaceInfo() const;
    int64  AsInt64() const {
        return data;
    }

    Font& Face(int n) {
        v.face = n;
        return *this;
    }
    Font& Height(int n) {
        v.height = n;
        return *this;
    }
    Font& Width(int n) {
        v.width = n;
        return *this;
    }
    Font& Bold() {
        v.flags |= 0x8000;
        return *this;
    }
    Font& NoBold() {
        v.flags &= ~0x8000;
        return *this;
    }
    Font& Bold(bool b) {
        return b ? Bold() : NoBold();
    }
    Font& Italic() {
        v.flags |= 0x4000;
        return *this;
    }
    Font& NoItalic() {
        v.flags &= ~0x4000;
        return *this;
    }
    Font& Italic(bool b) {
        return b ? Italic() : NoItalic();
    }
    Font& Underline() {
        v.flags |= 0x2000;
        return *this;
    }
    Font& NoUnderline() {
        v.flags &= ~0x2000;
        return *this;
    }
    Font& Underline(bool b) {
        return b ? Underline() : NoUnderline();
    }
    Font& Strikeout() {
        v.flags |= 0x1000;
        return *this;
    }
    Font& NoStrikeout() {
        v.flags &= ~0x1000;
        return *this;
    }
    Font& Strikeout(bool b) {
        return b ? Strikeout() : NoStrikeout();
    }
    Font& NonAntiAliased() {
        v.flags |= 0x800;
        return *this;
    }
    Font& NoNonAntiAliased() {
        v.flags &= ~0x800;
        return *this;
    }
    Font& NonAntiAliased(bool b) {
        return b ? NonAntiAliased() : NoNonAntiAliased();
    }
    Font& TrueTypeOnly() {
        v.flags |= 0x400;
        return *this;
    }
    Font& NoTrueTypeOnly() {
        v.flags &= ~0x400;
        return *this;
    }
    Font& TrueTypeOnly(bool b) {
        return b ? TrueTypeOnly() : NoTrueTypeOnly();
    }
    Font& FaceName(const String& name);

    Font  operator()() const {
        return *this;
    }
    Font  operator()(int n) const {
        return Font(*this).Height(n);
    }


    int   GetAscent() const {
        return Fi().ascent;
    }
    int   GetDescent() const {
        return Fi().descent;
    }
    int   GetCy() const {
        return Fi().height;
    }
    int   GetExternal() const {
        return Fi().external;
    }
    int   GetInternal() const {
        return Fi().internal;
    }
    int   GetLineHeight() const {
        return Fi().lineheight;
    }
    int   GetOverhang() const {
        return Fi().overhang;
    }
    int   GetAveWidth() const {
        return Fi().avewidth;
    }
    int   GetMaxWidth() const {
        return Fi().maxwidth;
    }
    bool  IsNormal(int ch) const;
    bool  IsComposed(int ch) const;
    bool  IsReplaced(int ch) const;
    bool  IsMissing(int ch) const;
    int   HasChar(int ch) const;
    int   GetWidth(int c) const;
    int   operator[](int c) const {
        return GetWidth(c);
    }
    int   GetLeftSpace(int c) const;
    int   GetRightSpace(int c) const;
    bool  IsFixedPitch() const {
        return Fi().fixedpitch;
    }
    bool  IsScaleable() const {
        return Fi().scaleable;
    }
    bool  IsSpecial() const {
        return !(GetFaceInfo() & SPECIAL);
    }

    void  Serialize(Stream& s);

    bool  operator==(Font f) const {
        return v.face == f.v.face && v.flags == f.v.flags &&
               v.width == f.v.width && v.height == f.v.height;
    }
    bool  operator!=(Font f) const {
        return !operator==(f);
    }

    dword GetHashValue() const {
        return CombineHash(v.width, v.flags, v.height, v.face);
    }
    bool  IsNull() const {
        return v.face == 0xffff;
    }

    Font() {
        v.height = v.width = 0;
        v.face = v.flags = 0;
    }
    Font(int face, int height) {
        v.face = face;
        v.height = height;
        v.flags = 0;
        v.width = 0;
    }
    Font(const Nuller&) {
        v.face = 0xffff;
        v.height = v.width = 0;
        v.flags = 0;
    }

    operator Value() const {
        return RichValue<Font>(*this);
    }
    Font(const Value& q) {
        *this = RichValue<Font>::Extract(q);
    }

// BW compatibility
    FontInfo Info() const;
};

//BW compatibility
class FontInfo {
    Font font;
    friend class Font;
public:
    int    GetAscent() const {
        return font.GetAscent();
    }
    int    GetDescent() const {
        return font.GetDescent();
    }
    int    GetExternal() const {
        return font.GetExternal();
    }
    int    GetInternal() const {
        return font.GetInternal();
    }
    int    GetHeight() const {
        return font.GetCy();
    }
    int    GetLineHeight() const {
        return font.GetLineHeight();
    }
    int    GetOverhang() const {
        return font.GetOverhang();
    }
    int    GetAveWidth() const {
        return font.GetAveWidth();
    }
    int    GetMaxWidth() const {
        return font.GetMaxWidth();
    }
    int    HasChar(int c) const {
        return font.HasChar(c);
    }
    int    GetWidth(int c) const {
        return font.GetWidth(c);
    }
    int    operator[](int c) const {
        return GetWidth(c);
    }
    int    GetLeftSpace(int c) const {
        return font.GetLeftSpace(c);
    }
    int    GetRightSpace(int c) const {
        return font.GetRightSpace(c);
    }
    bool   IsFixedPitch() const {
        return font.IsFixedPitch();
    }
    bool   IsScaleable() const {
        return font.IsScaleable();
    }
    int    GetFontHeight() const {
        return font.GetHeight();
    }
    Font   GetFont() const {
        return font;
    }

};

struct ComposedGlyph {
    wchar  basic_char;
    Point  mark_pos;
    wchar  mark_char;
    Font   mark_font;
};

bool Compose(Font fnt, int chr, ComposedGlyph& cs);

template<>
inline bool IsNull(const Font& f) {
    return f.IsNull();
}

template<>
inline unsigned GetHashValue(const Font& f) {
    return f.GetHashValue();
}

template<>
String AsString(const Font& f);

inline void SetStdFont(Font font) {
    Font::SetStdFont(font);
}
inline Font GetStdFont() {
    return Font::GetStdFont();
}
inline Size GetStdFontSize() {
    return Font::GetStdFontSize();
}
inline int  GetStdFontCy() {
    return GetStdFontSize().cy;
}

Font StdFont();

inline Font StdFont(int h) {
    return StdFont().Height(h);
}


/*
struct ScreenSans : public Font  { ScreenSans(int n = 0) : Font(SCREEN_SANS, n) {} };
struct ScreenSerif : public Font { ScreenSerif(int n = 0) : Font(SCREEN_SERIF, n) {} };
struct ScreenFixed : public Font { ScreenFixed(int n = 0) : Font(SCREEN_FIXED, n) {} };

struct Roman     : public Font { Roman(int n) : Font(ROMAN, n) {} };
struct Arial     : public Font { Arial(int n) : Font(ARIAL, n) {} };
struct Courier   : public Font { Courier(int n) : Font(COURIER, n) {} };

struct Symbol    : public Font { Symbol(int n) : Font(SYMBOL, n) {} };
struct WingDings : public Font { WingDings(int n) : Font(WINGDINGS, n) {} };
struct Tahoma    : public Font { Tahoma(int n) : Font(TAHOMA, n) {} };
*/

inline Font Serif(int n = 0) {
    return Font(Font::SCREEN_SERIF, n);
}
inline Font SansSerif(int n = 0) {
    return Font(Font::SCREEN_SANS, n);
}
inline Font Monospace(int n = 0) {
    return Font(Font::SCREEN_FIXED, n);
}

inline Font Roman(int n = 0) {
    return Font(Font::SCREEN_SERIF, n);
}
inline Font Arial(int n = 0) {
    return Font(Font::SCREEN_SANS, n);
}
inline Font Courier(int n = 0) {
    return Font(Font::SCREEN_FIXED, n);
}

inline Font ScreenSerif(int n = 0) {
    return Font(Font::SCREEN_SERIF, n);
}
inline Font ScreenSans(int n = 0) {
    return Font(Font::SCREEN_SANS, n);
}
inline Font ScreenFixed(int n = 0) {
    return Font(Font::SCREEN_FIXED, n);
}

inline Font Tahoma(int n = 0) {
    return Font(Font::TAHOMA, n);
}

Size GetTextSize(const wchar *text, Font font, int n = -1);
Size GetTextSize(const WString& text, Font font);
Size GetTextSize(const char *text, byte charset, Font font, int n = -1);
Size GetTextSize(const char *text, Font font, int n = -1);
Size GetTextSize(const String& text, Font font);

enum {
    PEN_NULL = -1,
    PEN_SOLID = -2,
    PEN_DASH = -3,
    PEN_DOT = -4,
    PEN_DASHDOT = -5,
    PEN_DASHDOTDOT = -6,
};

class Image;

//DEPRECATED: TODO
Color SBlack();
Color SGray();
Color SLtGray();
Color SWhiteGray();
Color SWhite();
Color SRed();
Color SGreen();
Color SBrown();
Color SBlue();
Color SMagenta();
Color SCyan();
Color SYellow();
Color SLtRed();
Color SLtGreen();
Color SLtYellow();
Color SLtBlue();
Color SLtMagenta();
Color SLtCyan();
//END OF DEPRECATED

Color SColorPaper();
Color SColorText();
Color SColorHighlight();
Color SColorHighlightText();//
Color SColorMenu();
Color SColorMenuText();
Color SColorInfo();
Color SColorInfoText();//
Color SColorMark();
Color SColorMenuMark();
Color SColorDisabled();
Color SColorLight();
Color SColorFace();
Color SColorLabel();
Color SColorShadow();

Color SColorLtFace();
Color SColorDkShadow();


void SBlack_Write(Color c);
void SGray_Write(Color c);
void SLtGray_Write(Color c);
void SWhiteGray_Write(Color c);
void SWhite_Write(Color c);
void SRed_Write(Color c);
void SGreen_Write(Color c);
void SBrown_Write(Color c);
void SBlue_Write(Color c);
void SMagenta_Write(Color c);
void SCyan_Write(Color c);
void SYellow_Write(Color c);
void SLtRed_Write(Color c);
void SLtGreen_Write(Color c);
void SLtYellow_Write(Color c);
void SLtBlue_Write(Color c);
void SLtMagenta_Write(Color c);
void SLtCyan_Write(Color c);

void SColorPaper_Write(Color c);
void SColorText_Write(Color c);
void SColorHighlight_Write(Color c);
void SColorHighlightText_Write(Color c);   //
void SColorMenu_Write(Color c);
void SColorMenuText_Write(Color c);
void SColorInfo_Write(Color c);
void SColorInfoText_Write(Color c);   //
void SColorMark_Write(Color c);
void SColorMenuMark_Write(Color c);
void SColorDisabled_Write(Color c);
void SColorLight_Write(Color c);
void SColorFace_Write(Color c);
void SColorLabel_Write(Color c);
void SColorShadow_Write(Color c);

void SColorLtFace_Write(Color c);
void SColorDkShadow_Write(Color c);


inline Color InvertColor() {
    return Color(255, 0);
}
inline Color DefaultInk() {
    return Black();    //TODO!
}

class Painting : AssignValueTypeNo<Painting, 48, Moveable<Painting> > {
    String     cmd;
    ValueArray data;
    Sizef      size;

    friend class PaintingPainter;
    friend class Painter;

public:
    Sizef   GetSize() const {
        return size;
    }

    void    Clear() {
        size = Null;
        data.Clear();
        cmd.Clear();
    }
    void    Serialize(Stream& s) {
        s % cmd % data % size;
    }
    bool    IsNullInstance() const {
        return cmd.IsEmpty();
    }
    bool    operator==(const Painting& b) const {
        return cmd == b.cmd && data == b.data && size == b.size;
    }
    unsigned GetHashValue() const {
        return CombineHash(cmd, data);
    }
    String  ToString() const {
        return "painting " + AsString(size);
    }

    operator Value() const {
        return RichValue<Painting>(*this);
    }
    Painting(const Value& q) {
        *this = RichValue<Painting>::Extract(q);
    }

    Painting() {
        size = Null;
    }
    Painting(const Nuller&) {
        size = Null;
    }
};

enum {
    MODE_ANTIALIASED = 0,
    MODE_NOAA        = 1,
    MODE_SUBPIXEL    = 2,
};

bool HasPainter();
void PaintImageBuffer(ImageBuffer& ib, const Painting& p, Size sz, Point pos, int mode = MODE_ANTIALIASED);
void PaintImageBuffer(ImageBuffer& ib, const Painting& p, int mode = MODE_ANTIALIASED);
void PaintImageBuffer(ImageBuffer& ib, const Drawing& p, int mode = MODE_ANTIALIASED);

class Draw : NoCopy {
private:
    struct DrawingPos;

public:
    enum {
        DOTS = 0x001,
        GUI = 0x002,
        PRINTER = 0x004,
        NATIVE = 0x008,
        DATABANDS = 0x010,
    };

    virtual dword GetInfo() const = 0;

    virtual Size GetPageSize() const;
    virtual void StartPage();
    virtual void EndPage();

    virtual void BeginOp() = 0;
    virtual void EndOp() = 0;
    virtual void OffsetOp(Point p) = 0;
    virtual bool ClipOp(const Rect& r) = 0;
    virtual bool ClipoffOp(const Rect& r) = 0;
    virtual bool ExcludeClipOp(const Rect& r) = 0;
    virtual bool IntersectClipOp(const Rect& r) = 0;
    virtual bool IsPaintingOp(const Rect& r) const = 0;
    virtual Rect GetPaintRect() const;

    virtual void DrawRectOp(int x, int y, int cx, int cy, Color color) = 0;
    virtual void DrawImageOp(int x, int y, int cx, int cy, const Image& img, const Rect& src, Color color) = 0;
    virtual void DrawDataOp(int x, int y, int cx, int cy, const String& data, const char *id);
    virtual void DrawLineOp(int x1, int y1, int x2, int y2, int width, Color color) = 0;

    virtual void DrawPolyPolylineOp(const Point *vertices, int vertex_count,
                                    const int *counts, int count_count,
                                    int width, Color color, Color doxor) = 0;
    virtual void DrawPolyPolyPolygonOp(const Point *vertices, int vertex_count,
                                       const int *subpolygon_counts, int scc,
                                       const int *disjunct_polygon_counts, int dpcc,
                                       Color color, int width, Color outline,
                                       uint64 pattern, Color doxor) = 0;
    virtual void DrawArcOp(const Rect& rc, Point start, Point end, int width, Color color) = 0;

    virtual void DrawEllipseOp(const Rect& r, Color color, int pen, Color pencolor) = 0;
    virtual void DrawTextOp(int x, int y, int angle, const wchar *text, Font font,
                            Color ink, int n, const int *dx) = 0;
    virtual void DrawDrawingOp(const Rect& target, const Drawing& w);
    virtual void DrawPaintingOp(const Rect& target, const Painting& w);

    virtual Size GetNativeDpi() const;
    virtual void BeginNative();
    virtual void EndNative();

    virtual int  GetCloffLevel() const;

    virtual ~Draw();

// --------------
    Size  GetPixelsPerInch() const;
    Size  GetPageMMs() const;

    bool  Dots() const {
        return GetInfo() & DOTS;
    }
    bool  Pixels() const {
        return !Dots();
    }
    bool  IsGui() const {
        return GetInfo() & GUI;
    }
    bool  IsPrinter() const {
        return GetInfo() & PRINTER;
    }
    bool  IsNative() const {
        return GetInfo() & NATIVE;
    }

    int  GetNativeX(int x) const;
    int  GetNativeY(int y) const;
    void Native(int& x, int& y) const;
    void Native(Point& p) const;
    void Native(Size& sz) const;
    void Native(Rect& r) const;

    void  Begin() {
        BeginOp();
    }
    void  End() {
        EndOp();
    }
    void  Offset(Point p) {
        OffsetOp(p);
    }
    void  Offset(int x, int y);
    bool  Clip(const Rect& r) {
        return ClipOp(r);
    }
    bool  Clip(int x, int y, int cx, int cy);
    bool  Clipoff(const Rect& r) {
        return ClipoffOp(r);
    }
    bool  Clipoff(int x, int y, int cx, int cy);
    bool  ExcludeClip(const Rect& r) {
        return ExcludeClipOp(r);
    }
    bool  ExcludeClip(int x, int y, int cx, int cy);
    bool  IntersectClip(const Rect& r) {
        return IntersectClipOp(r);
    }
    bool  IntersectClip(int x, int y, int cx, int cy);
    bool  IsPainting(const Rect& r) const {
        return IsPaintingOp(r);
    }
    bool  IsPainting(int x, int y, int cx, int cy) const;

    void DrawRect(int x, int y, int cx, int cy, Color color);
    void DrawRect(const Rect& rect, Color color);

    void DrawImage(int x, int y, int cx, int cy, const Image& img, const Rect& src);
    void DrawImage(int x, int y, int cx, int cy, const Image& img);
    void DrawImage(int x, int y, int cx, int cy, const Image& img, const Rect& src, Color color);
    void DrawImage(int x, int y, int cx, int cy, const Image& img, Color color);

    void DrawImage(const Rect& r, const Image& img, const Rect& src);
    void DrawImage(const Rect& r, const Image& img);
    void DrawImage(const Rect& r, const Image& img, const Rect& src, Color color);
    void DrawImage(const Rect& r, const Image& img, Color color);

    void DrawImage(int x, int y, const Image& img, const Rect& src);
    void DrawImage(int x, int y, const Image& img);
    void DrawImage(int x, int y, const Image& img, const Rect& src, Color color);
    void DrawImage(int x, int y, const Image& img, Color color);

    void DrawData(int x, int y, int cx, int cy, const String& data, const char *type);
    void DrawData(const Rect& r, const String& data, const char *type);

    void DrawLine(int x1, int y1, int x2, int y2, int width = 0, Color color = DefaultInk);
    void DrawLine(Point p1, Point p2, int width = 0, Color color = DefaultInk);

    void DrawEllipse(const Rect& r, Color color = DefaultInk,
                     int pen = Null, Color pencolor = DefaultInk);
    void DrawEllipse(int x, int y, int cx, int cy, Color color = DefaultInk,
                     int pen = Null, Color pencolor = DefaultInk);

    void DrawArc(const Rect& rc, Point start, Point end, int width = 0, Color color = DefaultInk);

    void DrawPolyPolyline(const Point *vertices, int vertex_count,
                          const int *counts, int count_count,
                          int width = 0, Color color = DefaultInk, Color doxor = Null);
    void DrawPolyPolyline(const Vector<Point>& vertices, const Vector<int>& counts,
                          int width = 0, Color color = DefaultInk, Color doxor = Null);
    void DrawPolyline(const Point *vertices, int count,
                      int width = 0, Color color = DefaultInk, Color doxor = Null);
    void DrawPolyline(const Vector<Point>& vertices,
                      int width = 0, Color color = DefaultInk, Color doxor = Null);

    void   DrawPolyPolyPolygon(const Point *vertices, int vertex_count,
                               const int *subpolygon_counts, int subpolygon_count_count,
                               const int *disjunct_polygon_counts, int disjunct_polygon_count_count,
                               Color color = DefaultInk, int width = 0, Color outline = Null,
                               uint64 pattern = 0, Color doxor = Null);
    void   DrawPolyPolyPolygon(const Vector<Point>& vertices,
                               const Vector<int>& subpolygon_counts,
                               const Vector<int>& disjunct_polygon_counts,
                               Color color = DefaultInk, int width = 0, Color outline = Null, uint64 pattern = 0, Color doxor = Null);
    void   DrawPolyPolygon(const Point *vertices, int vertex_count,
                           const int *subpolygon_counts, int subpolygon_count_count,
                           Color color = DefaultInk, int width = 0, Color outline = Null, uint64 pattern = 0, Color doxor = Null);
    void   DrawPolyPolygon(const Vector<Point>& vertices, const Vector<int>& subpolygon_counts,
                           Color color = DefaultInk, int width = 0, Color outline = Null, uint64 pattern = 0, Color doxor = Null);
    void   DrawPolygons(const Point *vertices, int vertex_count,
                        const int *polygon_counts, int polygon_count_count,
                        Color color = DefaultInk, int width = 0, Color outline = Null, uint64 pattern = 0, Color doxor = Null);
    void   DrawPolygons(const Vector<Point>& vertices, const Vector<int>& polygon_counts,
                        Color color = DefaultInk, int width = 0, Color outline = Null, uint64 pattern = 0, Color doxor = Null);
    void   DrawPolygon(const Point *vertices, int vertex_count,
                       Color color = DefaultInk, int width = 0, Color outline = Null, uint64 pattern = 0, Color doxor = Null);
    void   DrawPolygon(const Vector<Point>& vertices,
                       Color color = DefaultInk, int width = 0, Color outline = Null, uint64 pattern = 0, Color doxor = Null);

    void DrawDrawing(const Rect& r, const Drawing& iw) {
        DrawDrawingOp(r, iw);
    }
    void DrawDrawing(int x, int y, int cx, int cy, const Drawing& iw);

    void DrawPainting(const Rect& r, const Painting& iw) {
        DrawPaintingOp(r, iw);
    }
    void DrawPainting(int x, int y, int cx, int cy, const Painting& iw);

    void DrawText(int x, int y, int angle, const wchar *text, Font font = StdFont(),
                  Color ink = DefaultInk, int n = -1, const int *dx = NULL);
    void DrawText(int x, int y, const wchar *text, Font font = StdFont(),
                  Color ink = DefaultInk, int n = -1, const int *dx = NULL);

    void DrawText(int x, int y, const WString& text, Font font = StdFont(),
                  Color ink = DefaultInk, const int *dx = NULL);
    void DrawText(int x, int y, int angle, const WString& text, Font font = StdFont(),
                  Color ink = DefaultInk, const int *dx = NULL);

    void DrawText(int x, int y, int angle, const char *text, byte charset,
                  Font font = StdFont(), Color ink = DefaultInk, int n = -1, const int *dx = NULL);
    void DrawText(int x, int y, const char *text, byte charset, Font font = StdFont(),
                  Color ink = DefaultInk, int n = -1, const int *dx = NULL);

    void DrawText(int x, int y, int angle, const char *text,
                  Font font = StdFont(), Color ink = DefaultInk, int n = -1, const int *dx = NULL);
    void DrawText(int x, int y, const char *text, Font font = StdFont(),
                  Color ink = DefaultInk, int n = -1, const int *dx = NULL);

    void DrawText(int x, int y, const String& text, Font font = StdFont(),
                  Color ink = DefaultInk, const int *dx = NULL);
    void DrawText(int x, int y, int angle, const String& text, Font font = StdFont(),
                  Color ink = DefaultInk, const int *dx = NULL);

    static void SinCos(int angle, double& sina, double& cosa);

    // deprecated:
    static void SetStdFont(Font font) {
        UPP::SetStdFont(font);
    }
    static Font GetStdFont() {
        return UPP::GetStdFont();
    }
    static Size GetStdFontSize() {
        return UPP::GetStdFontSize();
    }
    static int  GetStdFontCy() {
        return GetStdFontSize().cy;
    }
    Size   GetPagePixels() const {
        return GetPageSize();
    }

    static void Flush();
    HDC   BeginGdi();
    void  EndGdi();
};

void DrawImageBandRLE(Draw& w, int x, int y, const Image& m, int minp);

class DataDrawer {
    typedef DataDrawer *(*Factory)();
    template <class T> static DataDrawer *FactoryFn() {
        return new T;
    }
    static void AddFormat(const char *id, Factory f);
    static VectorMap<String, void *>& Map();

public:
    virtual void  Open(const String& data, int cx, int cy) = 0;
    virtual void  Render(ImageBuffer& ib) = 0;
    virtual ~DataDrawer();

    static  One<DataDrawer> Create(const String& id);

    template <class T>  static void Register(const char *id) {
        AddFormat(id, &DataDrawer::FactoryFn<T>);
    }
};

class Drawing : AssignValueTypeNo<Drawing, 49, Moveable<Drawing> > {
    Size       size;
    String     data;
    ValueArray val;

    friend class DrawingDraw;
    friend class Draw;

public:
    operator bool() const {
        return !data.IsEmpty();
    }
    Size GetSize() const {
        return size;
    }
    void SetSize(Size sz) {
        size = sz;
    }
    void SetSize(int cx, int cy) {
        size = Size(cx, cy);
    }

    Size RatioSize(int cx, int cy) const;
    Size RatioSize(Size sz) const {
        return RatioSize(sz.cx, sz.cy);
    }

    void Clear() {
        data.Clear();
        size = Null;
    }

    void Append(Drawing& dw);

    void Serialize(Stream& s);

    bool    IsNullInstance() const {
        return data.IsEmpty();
    }
    bool    operator==(const Drawing& b) const {
        return val == b.val && data == b.data && size == b.size;
    }
    String  ToString() const {
        return "drawing " + AsString(size);
    }
    unsigned GetHashValue() const {
        return CombineHash(data, val);
    }

    operator Value() const {
        return RichValue<Drawing>(*this);
    }
    Drawing(const Value& src);

    Drawing() {
        size = Null;
    }
    Drawing(const Nuller&) {
        size = Null;
    }
};

class DrawingDraw : public Draw {
public:
    virtual dword GetInfo() const;
    virtual Size  GetPageSize() const;
    virtual Rect  GetPaintRect() const;

    virtual void BeginOp();
    virtual void EndOp();
    virtual void OffsetOp(Point p);
    virtual bool ClipOp(const Rect& r);
    virtual bool ClipoffOp(const Rect& r);
    virtual bool ExcludeClipOp(const Rect& r);
    virtual bool IntersectClipOp(const Rect& r);
    virtual bool IsPaintingOp(const Rect& r) const;

    virtual void DrawRectOp(int x, int y, int cx, int cy, Color color);
    virtual void DrawImageOp(int x, int y, int cx, int cy, const Image& img, const Rect& src, Color color);
    virtual void DrawDataOp(int x, int y, int cx, int cy, const String& data, const char *id);
    virtual void DrawLineOp(int x1, int y1, int x2, int y2, int width, Color color);
    virtual void DrawPolyPolylineOp(const Point *vertices, int vertex_count,
                                    const int *counts, int count_count,
                                    int width, Color color, Color doxor);
    virtual void DrawPolyPolyPolygonOp(const Point *vertices, int vertex_count,
                                       const int *subpolygon_counts, int scc,
                                       const int *disjunct_polygon_counts, int dpcc,
                                       Color color, int width, Color outline,
                                       uint64 pattern, Color doxor);
    virtual void DrawEllipseOp(const Rect& r, Color color, int pen, Color pencolor);
    virtual void DrawArcOp(const Rect& rc, Point start, Point end, int pen, Color pencolor);
    virtual void DrawTextOp(int x, int y, int angle, const wchar *text, Font font,
                            Color ink, int n, const int *dx);

    virtual void DrawDrawingOp(const Rect& target, const Drawing& w);
    virtual void DrawPaintingOp(const Rect& target, const Painting& w);

private:
    Size         size;
    bool         dots;
    StringStream drawing;
    ValueArray   val;

    Stream&      DrawingOp(int code);

public:
    void     Create(int cx, int cy, bool dots = true);
    void     Create(Size sz, bool dots = true);

    Size     GetSize() const {
        return size;
    }

    Drawing  GetResult();
    operator Drawing() {
        return GetResult();
    }

    DrawingDraw();
    DrawingDraw(int cx, int cy, bool dots = true);
    DrawingDraw(Size sz, bool dots = true);
};

class NilDraw : public Draw {
public:
    virtual dword GetInfo() const;
    virtual Size  GetPageSize() const;
    virtual void BeginOp();
    virtual void EndOp();
    virtual void OffsetOp(Point p);
    virtual bool ClipOp(const Rect& r);
    virtual bool ClipoffOp(const Rect& r);
    virtual bool ExcludeClipOp(const Rect& r);
    virtual bool IntersectClipOp(const Rect& r);
    virtual bool IsPaintingOp(const Rect& r) const;
    virtual Rect GetPaintRect() const;

    virtual void DrawRectOp(int x, int y, int cx, int cy, Color color);
    virtual void DrawImageOp(int x, int y, int cx, int cy, const Image& img, const Rect& src, Color color);
    virtual void DrawDataOp(int x, int y, int cx, int cy, const String& data, const char *id);
    virtual void DrawLineOp(int x1, int y1, int x2, int y2, int width, Color color);
    virtual void DrawPolyPolylineOp(const Point *vertices, int vertex_count,
                                    const int *counts, int count_count,
                                    int width, Color color, Color doxor);
    virtual void DrawPolyPolyPolygonOp(const Point *vertices, int vertex_count,
                                       const int *subpolygon_counts, int scc,
                                       const int *disjunct_polygon_counts, int dpcc,
                                       Color color, int width, Color outline,
                                       uint64 pattern, Color doxor);
    virtual void DrawArcOp(const Rect& rc, Point start, Point end, int width, Color color);
    virtual void DrawEllipseOp(const Rect& r, Color color, int pen, Color pencolor);
    virtual void DrawTextOp(int x, int y, int angle, const wchar *text, Font font,
                            Color ink, int n, const int *dx);
    virtual void DrawDrawingOp(const Rect& target, const Drawing& w);
    virtual void DrawPaintingOp(const Rect& target, const Painting& w);
};

class ImageAnyDraw : public Draw {
    Draw *draw;

    void Init(Size sz);

public:
    virtual dword GetInfo() const;
    virtual Size  GetPageSize() const;
    virtual void BeginOp();
    virtual void EndOp();
    virtual void OffsetOp(Point p);
    virtual bool ClipOp(const Rect& r);
    virtual bool ClipoffOp(const Rect& r);
    virtual bool ExcludeClipOp(const Rect& r);
    virtual bool IntersectClipOp(const Rect& r);
    virtual bool IsPaintingOp(const Rect& r) const;
    virtual Rect GetPaintRect() const;

    virtual void DrawRectOp(int x, int y, int cx, int cy, Color color);
    virtual void DrawImageOp(int x, int y, int cx, int cy, const Image& img, const Rect& src, Color color);
    virtual void DrawDataOp(int x, int y, int cx, int cy, const String& data, const char *id);
    virtual void DrawLineOp(int x1, int y1, int x2, int y2, int width, Color color);
    virtual void DrawPolyPolylineOp(const Point *vertices, int vertex_count,
                                    const int *counts, int count_count,
                                    int width, Color color, Color doxor);
    virtual void DrawPolyPolyPolygonOp(const Point *vertices, int vertex_count,
                                       const int *subpolygon_counts, int scc,
                                       const int *disjunct_polygon_counts, int dpcc,
                                       Color color, int width, Color outline,
                                       uint64 pattern, Color doxor);
    virtual void DrawArcOp(const Rect& rc, Point start, Point end, int width, Color color);
    virtual void DrawEllipseOp(const Rect& r, Color color, int pen, Color pencolor);
    virtual void DrawTextOp(int x, int y, int angle, const wchar *text, Font font,
                            Color ink, int n, const int *dx);
    virtual void DrawDrawingOp(const Rect& target, const Drawing& w);
    virtual void DrawPaintingOp(const Rect& target, const Painting& w);

public:
    operator Image() const;

    ImageAnyDraw(Size sz);
    ImageAnyDraw(int cx, int cy);

    ~ImageAnyDraw();
};

void         AddNotEmpty(Vector<Rect>& result, int left, int right, int top, int bottom);
bool         Subtract(const Rect& r, const Rect& sub, Vector<Rect>& result);
bool         Subtract(const Vector<Rect>& rr, const Rect& sub, Vector<Rect>& result);
Vector<Rect> Subtract(const Vector<Rect>& rr, const Rect& sub, bool& changed);
Vector<Rect> Intersect(const Vector<Rect>& b, const Rect& a, bool& changed);

void Subtract(Vector<Rect>& rr, const Rect& sub);
void Union(Vector<Rect>& rr, const Rect& add);

void DrawRect(Draw& w, const Rect& rect, const Image& img, bool ralgn = false);   //??? TODO
void DrawRect(Draw& w, int x, int y, int cx, int cy, const Image& img, bool ra = false);

void DrawTiles(Draw& w, int x, int y, int cx, int cy, const Image& img);
void DrawTiles(Draw& w, const Rect& rect, const Image& img);

void DrawFatFrame(Draw& w, int x, int y, int cx, int cy, Color color, int n);
void DrawFatFrame(Draw& w, const Rect& r, Color color, int n);

void DrawFrame(Draw& w, int x, int y, int cx, int cy,
               Color leftcolor, Color topcolor, Color rightcolor, Color bottomcolor);
void DrawFrame(Draw& w, const Rect& r,
               Color leftcolor, Color topcolor, Color rightcolor, Color bottomcolor);
void DrawFrame(Draw& w, int x, int y, int cx, int cy,
               Color topleftcolor, Color bottomrightcolor);
void DrawFrame(Draw& w, const Rect& r,
               Color topleftcolor, Color bottomrightcolor);
void DrawFrame(Draw& w, int x, int y, int cx, int cy, Color color);
void DrawFrame(Draw& w, const Rect& r, Color color);

void DrawBorder(Draw& w, int x, int y, int cx, int cy, const ColorF *colors_ltrd);   //TODO
void DrawBorder(Draw& w, const Rect& r, const ColorF *colors_ltrd);

const ColorF *BlackBorder();
const ColorF *ButtonPushBorder();
const ColorF *EdgeButtonBorder();
const ColorF *DefButtonBorder();
const ColorF *ButtonBorder();
const ColorF *InsetBorder();
const ColorF *OutsetBorder();
const ColorF *ThinOutsetBorder();
const ColorF *ThinInsetBorder();

void DrawBorder(Draw& w, int x, int y, int cx, int cy, const ColorF * (*colors_ltrd)());
void DrawBorder(Draw& w, const Rect& r, const ColorF * (*colors_ltrd)());

void DrawRectMinusRect(Draw& w, const Rect& rect, const Rect& inner, Color color);

void DrawHighlightImage(Draw& w, int x, int y, const Image& img, bool highlight = true,
                        bool enabled = true, Color maskcolor = SColorPaper);

Color GradientColor(Color fc, Color tc, int i, int n);

enum {
    BUTTON_NORMAL, BUTTON_OK, BUTTON_HIGHLIGHT, BUTTON_PUSH, BUTTON_DISABLED, BUTTON_CHECKED,
    BUTTON_VERTICAL = 0x100,
    BUTTON_EDGE = 0x200,
    BUTTON_TOOL = 0x400,
    BUTTON_SCROLL = 0x800,
};

void DrawXPButton(Draw& w, Rect r, int type);

void DrawTextEllipsis(Draw& w, int x, int y, int cx, const char *text, const char *ellipsis,
                      Font font = StdFont(), Color ink = SColorText(), int n = -1);
void DrawTextEllipsis(Draw& w, int x, int y, int cx, const wchar *text, const char *ellipsis,
                      Font font = StdFont(), Color ink = SColorText(), int n = -1);
Size GetTLTextSize(const wchar *text, Font font = StdFont());
int  GetTLTextHeight(const wchar *s, Font font = StdFont());
void DrawTLText(Draw& draw, int x, int y, int cx, const wchar *text, Font font = StdFont(),
                Color ink = SColorText(), int accesskey = 0);


typedef String(*DrawingToPdfFnType)(const Array<Drawing>& report, Size pagesize, int margin);

void SetDrawingToPdfFn(DrawingToPdfFnType Pdf);
DrawingToPdfFnType GetDrawingToPdfFn();

#include "Display.h"
#include "Cham.h"

END_UPP_NAMESPACE




#endif
