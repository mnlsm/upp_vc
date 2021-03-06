
struct RGBA : Moveable<RGBA> {
    byte b;
    byte g;
    byte r;
    byte a;
};



const int COLOR_V = 39;

class Color : AssignValueTypeNo<Color, COLOR_V, Moveable<Color> > {
public:
    Color() {
        color = 0xffffffff;
    }

    Color(int r, int g, int b) {
        color = RGB(r, g, b);
    }

    Color(int n, int) {
        color = 0x80000000 | n;
    }

    Color(const Nuller&) {
        color = 0xffffffff;
    }

    Color(const Value& q) {
        color = RichValue<Color>::Extract(q).color;
    }

    Color(Color(*fn)()) {
        color = (*fn)().color;
    }

public:
    dword GetRaw() const {
        return color;
    }

    int GetR() const {
        return GetRValue(Get());
    }
    int GetG() const {
        return GetGValue(Get());
    }
    int GetB() const {
        return GetBValue(Get());
    }

    bool IsNullInstance() const {
        return color == 0xffffffff;
    }

    unsigned GetHashValue() const {
        return color;
    }

    bool operator==(Color c) const {
        return color == c.color;
    }

    bool operator!=(Color c) const {
        return color != c.color;
    }

    void Serialize(Stream& s) {
        s % color;
    }


    operator RGBA() const;
    Color(RGBA rgba);

    operator Value() const {
        return RichValue<Color>(*this);
    }

    static Color FromRaw(dword co) {
        Color c;
        c.color = co;
        return c;
    }

    operator COLORREF() const {
        return (COLORREF) Get();
    }

    static  Color FromCR(COLORREF cr) {
        Color c;
        c.color = (dword)cr;
        return c;
    }
protected:
    dword color;
    dword Get() const;


};

RGBA operator*(int alpha, Color c);

inline Color StraightColor(RGBA rgba) {
    return Color(rgba.r, rgba.g, rgba.b);
}

typedef Color(*ColorF)();

inline unsigned GetHashValue(Color c) {
    return c.GetHashValue();
}
inline Color    Nvl(Color a, Color b) {
    return IsNull(a) ? b : a;
}

template<>
String AsString(const Color& c);


inline Color GrayColor(int a = 128) {
    return Color(a, a, a);
}

inline Color Black() {
    return Color(0, 0, 0);
}
inline Color Gray() {
    return Color(128, 128, 128);
}
inline Color LtGray() {
    return Color(192, 192, 192);
}
inline Color WhiteGray() {
    return Color(224, 224, 224);
}
inline Color White() {
    return Color(255, 255, 255);
}

inline Color Red() {
    return Color(128, 0, 0);
}
inline Color Green() {
    return Color(0, 128, 0);
}
inline Color Brown() {
    return Color(128, 128, 0);
}
inline Color Blue() {
    return Color(0, 0, 128);
}
inline Color Magenta() {
    return Color(128, 0, 255);
}
inline Color Cyan() {
    return Color(0, 128, 128);
}
inline Color Yellow() {
    return Color(255, 255, 0);
}
inline Color LtRed() {
    return Color(255, 0, 0);
}
inline Color LtGreen() {
    return Color(0, 255, 0);
}
inline Color LtYellow() {
    return Color(255, 255, 192);
}
inline Color LtBlue() {
    return Color(0, 0, 255);
}
inline Color LtMagenta() {
    return Color(255, 0, 255);
}
inline Color LtCyan() {
    return Color(0, 255, 255);
}

void   RGBtoHSV(double r, double g, double b, double& h, double& s, double& v);
void   HSVtoRGB(double h, double s, double v, double& r, double& g, double& b);

Color  HsvColorf(double h, double s, double v);

Color  Blend(Color c1, Color c2, int alpha = 128);

String ColorToHtml(Color color);

int  Grayscale(const Color& c);
bool IsDark(Color c);


inline bool operator==(const Value& v, Color x) {
    return v == x.operator Value();
}
inline bool operator==(Color x, const Value& v) {
    return v == x.operator Value();
}
inline bool operator!=(const Value& v, Color x) {
    return v != x.operator Value();
}
inline bool operator!=(Color x, const Value& v) {
    return v != x.operator Value();
}

inline bool operator==(const Value& v, Color(*x)()) {
    return v == (*x)();
}
inline bool operator==(Color(*x)(), const Value& v) {
    return v == (*x)();
}
inline bool operator!=(const Value& v, Color(*x)()) {
    return v != (*x)();
}
inline bool operator!=(Color(*x)(), const Value& v) {
    return v != (*x)();
}

inline bool operator==(Color c, Color(*x)()) {
    return c == (*x)();
}
inline bool operator==(Color(*x)(), Color c) {
    return c == (*x)();
}
inline bool operator!=(Color c, Color(*x)()) {
    return c != (*x)();
}
inline bool operator!=(Color(*x)(), Color c) {
    return c != (*x)();
}
