#include "Core.h"
#include <winnls.h>

NAMESPACE_UPP

bool PanicMode;

bool    IsPanicMode() {
    return PanicMode;
}

static  void (*sPanicMessageBox)(const char *title, const char *text);

void InstallPanicMessageBox(void (*mb)(const char *title, const char *text)) {
    sPanicMessageBox = mb;
}

void PanicMessageBox(const char *title, const char *text) {
    if(sPanicMessageBox)
        (*sPanicMessageBox)(title, text);
    else {
        write(2, text, (int)strlen(text));
        write(2, "\n", 1);
    }
}

void    Panic(const char *msg) {
    if(PanicMode)
        return;
    PanicMode = true;
    LOG(msg);
    BugLog() << "PANIC: " << msg << "\n";
    UsrLogT("===== PANIC ================================================");
    UsrLogT(msg);
    PanicMessageBox("Fatal error", msg);

#   ifdef __NOASSEMBLY__
#       if  defined(WIN64)
    DebugBreak();
#       endif
#   else
#       if defined(_DEBUG) && defined(CPU_X86)
#           ifdef COMPILER_MSC
    _asm int 3
#           endif
#           ifdef COMPILER_GCC
    asm("int $3");
#           endif
#       endif
#   endif

#ifdef _DEBUG
    __BREAK__;
#endif
    abort();
}

static void (*s_assert_hook)(const char *);

void    SetAssertFailedHook(void (*h)(const char *)) {
    s_assert_hook = h;
}

void    AssertFailed(const char *file, int line, const char *cond) {
    if(PanicMode)
        return;
    PanicMode = true;
    char s[2048];
    sprintf(s, "Assertion failed in %s, line %d\n%s\n", file, line, cond);
    LOG(s);
    LOG(GetLastErrorMessage());
    if(s_assert_hook)
        (*s_assert_hook)(s);
    BugLog() << "ASSERT FAILED: " << s << "\n";
    UsrLogT("===== ASSERT FAILED ================================================");
    UsrLogT(s);

    PanicMessageBox("Fatal error", s);

#   ifdef __NOASSEMBLY__
#       if  defined(WIN64)
    DebugBreak();
#       endif
#   else
#       if defined(_DEBUG) && defined(CPU_X86)
#           ifdef COMPILER_MSC
    _asm int 3
#           endif
#           ifdef COMPILER_GCC
    asm("int $3");
#           endif
#       endif
#   endif
    __BREAK__;
    abort();
}



void TimeStop::Reset() {
    starttime = GetTickCount();
}

TimeStop::TimeStop() {
    Reset();
}

String TimeStop::ToString() const {
    dword time = Elapsed();
    return Format("%d.%03d", int(time / 1000), int(time % 1000));
}

int RegisterTypeNo__(const char *type) {
    INTERLOCKED {
        static Index<String> types;
        return types.FindAdd(type);
    }
    return -1;
}

char *PermanentCopy(const char *s) {
    char *t = (char *)MemoryAllocPermanent(strlen(s) + 1);
    strcpy(t, s);
    return t;
}



int MemICmp(const void *dest, const void *src, int count) {

    const byte *a = (const byte *)dest;
    const byte *b = (const byte *)src;
    const byte *l = a + count;
    while(a < l) {
        if(ToUpper(*a) != ToUpper(*b))
            return ToUpper(*a) - ToUpper(*b);
        a++;
        b++;
    }
    return 0;
}

Stream& Pack16(Stream& s, Point& p) {
    return Pack16(s, p.x, p.y);
}

Stream& Pack16(Stream& s, Size& sz) {
    return Pack16(s, sz.cx, sz.cy);
}

Stream& Pack16(Stream& s, Rect& r) {
    return Pack16(s, r.left, r.top, r.right, r.bottom);
}

int InScListIndex(const char *s, const char *list) {
    int ii = 0;
    for(;;) {
        const char *q = s;
        for(;;) {
            if(*q == '\0' && *list == '\0') return ii;
            if(*q != *list) {
                if(*q == '\0' && *list == ';') return ii;
                if(*list == '\0') return -1;
                break;
            }
            q++;
            list++;
        }
        while(*list && *list != ';') list++;
        if(*list == '\0') return -1;
        list++;
        ii++;
    }
}

bool InScList(const char *s, const char *list) {
    return InScListIndex(s, list) >= 0;
}

void StringC::Free() {
    if(IsString()) delete(String *) bap.GetPtr();
}

StringC::~StringC() {
    Free();
}

StringC::operator const char *() const {
    if(IsEmpty()) return NULL;
    if(IsString()) return *(String *) bap.GetPtr();
    return (const char *)bap.GetPtr();
}

StringC::operator String() const {
    if(IsEmpty()) return (const char *)NULL;
    if(IsString()) return *(String *) bap.GetPtr();
    return (const char *)bap.GetPtr();
}

bool StringC::IsEmpty() const {
    if(IsString()) return (*(String *) bap.GetPtr()).IsEmpty();
    if(!bap.GetPtr()) return true;
    return !*(const char *)bap.GetPtr();
}

void StringC::SetString(const String& s) {
    Free();
    String *ptr = new String;
    *ptr = s;
    bap.Set1(ptr);
}

void StringC::SetCharPtr(const char *s) {
    Free();
    bap.Set0((void *)s);
}

CharFilterTextTest::CharFilterTextTest(int (*filter)(int)) : filter(filter) {}
CharFilterTextTest::~CharFilterTextTest() {}

const char *CharFilterTextTest::Accept(const char *s) const {
    if(!(*filter)((byte)*s++)) return NULL;
    return s;
}

Vector<String> Split(const char *s, const TextTest& delim, bool ignoreempty) {
    Vector<String> r;
    const char *t = s;
    while(*t) {
        const char *q = delim.Accept(t);
        if(q) {
            if(!ignoreempty || t > s)
                r.Add(String(s, t));
            t = s = q;
        }else
            t++;
    }
    if(!ignoreempty || t > s)
        r.Add(String(s, t));
    return r;
}

Vector<String> Split(const char *s, int (*filter)(int), bool ignoreempty) {
    return Split(s, CharFilterTextTest(filter), ignoreempty);
}

struct chrTextTest : public TextTest {
    int chr;
    virtual const char *Accept(const char *s) const {
        return chr == *s ? s + 1 : NULL;
    }
};

Vector<String> Split(const char *s, int chr, bool ignoreempty) {
    chrTextTest ct;
    ct.chr = chr;
    return Split(s, ct, ignoreempty);
}

struct StringSplit : TextTest {
    String test;

    virtual const char *Accept(const char *s) const {
        int l = test.GetCount();
        if(l && memcmp(~test, s, l) == 0)
            return s + l;
        return NULL;
    }
};

Vector<String> Split(const char *s, const String& delim, bool ignoreempty) {
    StringSplit ss;
    ss.test = delim;
    return Split(s, ss, ignoreempty);
}

String Join(const Vector<String>& im, const String& delim) {
    String r;
    for(int i = 0; i < im.GetCount(); i++) {
        if(i) r.Cat(delim);
        r.Cat(im[i]);
    }
    return r;
}

WString Join(const Vector<WString>& im, const WString& delim) {
    WString r;
    for(int i = 0; i < im.GetCount(); i++) {
        if(i) r.Cat(delim);
        r.Cat(im[i]);
    }
    return r;
}
// ---------------------------

VectorMap<String, String> LoadIniStream(Stream &sin) {
    Stream *in = &sin;
    FileIn fin;
    VectorMap<String, String> key;
    int c;
    if((c = in->Get()) < 0) return key;
    for(;;) {
        String k, v;
        for(;;) {
            if(IsAlNum(c) || c == '_')
                k.Cat(c);
            else
                break;
            if((c = in->Get()) < 0) return key;
        }
        for(;;) {
            if(c != '=' && c != ' ') break;
            if((c = in->Get()) < 0) return key;
        }
        for(;;) {
            if(c < ' ') break;
            v.Cat(c);
            if((c = in->Get()) < 0) break;
        }
        if(!k.IsEmpty())
            key.Add(k, v);
        if(k == "LINK") {
            if(in == &fin)
                fin.Close();
            if(!fin.Open(v) || (c = in->Get()) < 0) return key;
            in = &fin;
        }else
            for(;;) {
                if(IsAlNum(c) || c == '_') break;
                if((c = in->Get()) < 0) return key;
            }
    }
}

VectorMap<String, String> LoadIniFile(const char *filename) {
    FileIn in(filename);
    if(!in) return VectorMap<String, String>();
    return LoadIniStream(in);
}

static StaticMutex sMtx;
static char  sIniFile[256];
static bool s_ini_loaded;

void SetIniFile(const char *name) {
    Mutex::Lock __(sMtx);
    strcpy(sIniFile, name);
}

String GetIniKey(const char *id, const String& def) {
    Mutex::Lock __(sMtx);
    static VectorMap<String, String> key;
    if(!s_ini_loaded) {
        s_ini_loaded = true;
        key = LoadIniFile(*sIniFile ? sIniFile : ~ConfigFile("q.ini"));
        if(key.GetCount() == 0)
            key = LoadIniFile(~GetExeDirFile("q.ini"));
        if(key.GetCount() == 0)
            key = LoadIniFile("c:\\q.ini");

    }
    return key.Get(id, def);
}

String GetIniKey(const char *id) {
    return GetIniKey(id, String());
}

void TextSettings::Load(const char *filename) {
    FileIn in(filename);
    int themei = 0;
    settings.Add("");
    while(!in.IsEof()) {
        String ln = in.GetLine();
        const char *s = ln;
        if(*s == '[') {
            s++;
            String theme;
            while(*s && *s != ']')
                theme.Cat(*s++);
            themei = settings.FindAdd(theme);
        }else {
            if(themei >= 0) {
                String key;
                while(*s && *s != '=') {
                    key.Cat(*s++);
                }
                if(*s == '=') s++;
                String value;
                while(*s) {
                    value.Cat(*s++);
                }
                if(!IsEmpty(key))
                    settings[themei].GetAdd(key) = value;
            }
        }
    }
}

String TextSettings::Get(const char *group, const char *key) const {
    int itemi = settings.Find(group);
    return itemi < 0 ? Null : settings.Get(group).Get(key, Null);
}

String TextSettings::Get(int groupIndex, const char *key) const {
    return groupIndex >= 0 && groupIndex < settings.GetCount() ?
           settings[groupIndex].Get(key, Null) : Null;
}

String TextSettings::Get(int groupIndex, int keyIndex) const {
    if(groupIndex >= 0 && groupIndex < settings.GetCount())
        return keyIndex >= 0 && keyIndex < settings[groupIndex].GetCount() ?
               settings[groupIndex][keyIndex] : Null;
    else
        return Null;
}

// --------------------------------------------------------------

String timeFormat(double s) {
    if(s < 0.000001) return Sprintf("%5.2f ns", s * 1.0e9);
    if(s < 0.001) return Sprintf("%5.2f us", s * 1.0e6);
    if(s < 1) return Sprintf("%5.2f ms", s * 1.0e3);
    return Sprintf("%5.2f s ", s);
}

String Garble(const char *s, const char *e) {
    int c = 0xAA;
    String result;
    if(!e)
        e = s + strlen(s);
    while(s != e) {
        result.Cat(*s++ ^ (char)c);
        if((c <<= 1) & 0x100)
            c ^= 0x137;
    }
    return result;
}

String Garble(const String& s) {
    return Garble(~s, ~s + s.GetLength());
}

String Encode64(const String& s) {
    String enc;
    int l = s.GetLength();
    enc << l << ':';
    for(int i = 0; i < l;) {
        char a = 0, b = 0, c = 0;
        if(i < l) a = s[i++];
        if(i < l) b = s[i++];
        if(i < l) c = s[i++];
        enc.Cat(' ' + 1 + ((a >> 2) & 0x3F));
        enc.Cat(' ' + 1 + ((a << 4) & 0x30) + ((b >> 4) & 0x0F));
        enc.Cat(' ' + 1 + ((b << 2) & 0x3C) + ((c >> 6) & 0x03));
        enc.Cat(' ' + 1 + (c & 0x3F));
    }
    return enc;
}

String Decode64(const String& s) {
    if(!IsDigit(*s))
        return s;
    const char *p = s;
    char *h;
    int len = strtol(p, &h, 10);
    p = h;
    if(*p++ != ':' || len < 0 || (len + 2) / 3 * 4 > (s.End() - p))
        return s; // invalid encoding
    if(len == 0)
        return Null;
    String dec;
    for(;;) {
        byte ea = *p++ - ' ' - 1, eb = *p++ - ' ' - 1, ec = *p++ - ' ' - 1, ed = *p++ - ' ' - 1;
        byte out[3] = { (ea << 2) | (eb >> 4), (eb << 4) | (ec >> 2), (ec << 6) | (ed >> 0) };
        switch(len) {
        case 1:
            dec.Cat(out[0]);
            return dec;
        case 2:
            dec.Cat(out, 2);
            return dec;
        case 3:
            dec.Cat(out, 3);
            return dec;
        default:
            dec.Cat(out, 3);
            len -= 3;
            break;
        }
    }
}

String HexString(const byte *s, int count, int sep, int sepchr) {
    ASSERT(count >= 0);
    if(count == 0)
        return String();
    StringBuffer b(2 * count + (count - 1) / sep);
    static const char itoc[] = "0123456789abcdef";
    int i = 0;
    char *t = b;
    for(;;) {
        for(int q = 0; q < sep; q++) {
            if(i >= count)
                return b;
            *t++ = itoc[(s[i] & 0xf0) >> 4];
            *t++ = itoc[s[i] & 0x0f];
            i++;
        }
        if(i >= count)
            return b;
        *t++ = sepchr;
    }
}

String HexString(const String& s, int sep, int sepchr) {
    return HexString(~s, s.GetCount(), sep, sepchr);
}

String ScanHexString(const char *s, const char *lim) {
    String r;
    r.Reserve(int(lim - s) / 2);
    for(;;) {
        byte b = 0;
        while(!IsXDigit(*s)) {
            if(s >= lim)
                return r;
            s++;
        }
        b = ctoi(*s++);
        if(s >= lim)
            return r;
        while(!IsXDigit(*s)) {
            if(s >= lim) {
                r.Cat(b);
                return r;
            }
            s++;
        }
        b = (b << 4) + ctoi(*s++);
        r.Cat(b);
        if(s >= lim)
            return r;
    }
}

String NormalizeSpaces(const char *s) {
    StringBuffer r;
    while(*s && (byte)*s <= ' ')
        s++;
    while(*s) {
        if((byte)*s <= ' ') {
            while(*s && (byte)*s <= ' ')
                s++;
            if(*s)
                r.Cat(' ');
        }else
            r.Cat(*s++);
    }
    return r;
}

String NormalizeSpaces(const char *s, const char *end) {
    StringBuffer r;
    while(*s && (byte)*s <= ' ')
        s++;
    while(s < end) {
        if((byte)*s <= ' ') {
            while(s < end && (byte)*s <= ' ')
                s++;
            if(*s)
                r.Cat(' ');
        }else
            r.Cat(*s++);
    }
    return r;
}

int ChNoInvalid(int c) {
    return c == DEFAULTCHAR ? '_' : c;
}

String ToSystemCharset(const String& src) {
    WString s = src.ToWString();
    int l = s.GetLength() * 6;
    StringBuffer b(l);
    int q = WideCharToMultiByte(CP_ACP, 0, (const WCHAR *)~s, s.GetLength(), b, l, NULL, NULL);
    if(q <= 0)
        return src;
    b.SetCount(q);
    return b;
}

String FromWin32Charset(const String& src, int cp) {
    WStringBuffer b(src.GetLength());
    int q = MultiByteToWideChar(cp, MB_PRECOMPOSED, ~src, src.GetLength(), (WCHAR*)~b, src.GetLength());
    if(q <= 0)
        return src;
    b.SetCount(q);
    return WString(b).ToString();
}

String FromOEMCharset(const String& src) {
    return FromWin32Charset(src, CP_OEMCP);
}

String FromSystemCharset(const String& src) {
    return FromWin32Charset(src, CP_ACP);
}

WString ToSystemCharsetW(const char *src) {
    return String(src).ToWString();
}

String FromSystemCharsetW(const wchar *src) {
    return WString(src).ToString();
}

static VectorMap<String, String>& sGCfg() {
    static VectorMap<String, String> m;
    return m;
}

static StaticCriticalSection sGCfgLock;

static Vector<Callback>& sGFlush() {
    static Vector<Callback> m;
    return m;
}

static StaticCriticalSection sGFlushLock;

void    RegisterGlobalConfig(const char *name) {
    INTERLOCKED_(sGCfgLock) {
        ASSERT(sGCfg().Find(name) < 0);
        sGCfg().Add(name);
    }
}

void    RegisterGlobalConfig(const char *name, Callback WhenFlush) {
    RegisterGlobalConfig(name);
    INTERLOCKED_(sGFlushLock) {
        sGFlush().Add(WhenFlush);
    }
}

String GetGlobalConfigData(const char *name) {
    INTERLOCKED_(sGCfgLock) {
        return sGCfg().GetAdd(name);
    }
    return String();
}

void SetGlobalConfigData(const char *name, const String& data) {
    INTERLOCKED_(sGCfgLock) {
        sGCfg().GetAdd(name) = data;
    }
}

void  SerializeGlobalConfigs(Stream& s) {
    INTERLOCKED_(sGFlushLock) {
        for(int i = 0; i < sGFlush().GetCount(); i++)
            sGFlush()[i]();
    }
    INTERLOCKED_(sGCfgLock) {
        VectorMap<String, String>& cfg = sGCfg();
        int version = 0;
        s / version;
        int count = cfg.GetCount();
        s / count;
        for(int i = 0; i < count; i++) {
            String name;
            if(s.IsStoring())
                name = cfg.GetKey(i);
            s % name;
            int q = cfg.Find(name);
            if(q >= 0)
                s % cfg[q];
            else {
                String dummy;
                s % dummy;
            }
        }
        s.Magic();
    }
}

Exc::Exc() : String(GetLastErrorMessage()) {}

AbortExc::AbortExc() :
    Exc(t_("Aborted by user.")) {}



String GetErrorMessage(DWORD dwError) {
    char h[2048];
    sprintf(h, "%08x", dwError);

    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, dwError, 0, h, 2048, NULL);
    String result = h;
    String modf;
    const char* s = result;
    BYTE c;
    while((c = *s++) != 0)
        if(c <= ' ') {
            if(!modf.IsEmpty() && modf[modf.GetLength() - 1] != ' ')
                modf += ' ';
        }else if(c == '%' && *s >= '0' && *s <= '9') {
            s++;
            modf += "<###>";
        }else
            modf += (char)c;
    const char* p = modf;
    for(s = p + modf.GetLength(); s > p && s[-1] == ' '; s--);
    return FromSystemCharset(modf.Left((int)(s - p)));
}

String GetLastErrorMessage() {
    return GetErrorMessage(GetLastError());
}




void BeepInformation() {
    MessageBeep(MB_ICONINFORMATION);
}

void BeepExclamation() {
    MessageBeep(MB_ICONEXCLAMATION);
}

void BeepQuestion() {
    MessageBeep(MB_ICONQUESTION);
}

#if defined(COMPILER_MSC) && (_MSC_VER < 1300)
//hack for linking libraries built using VC7 with VC6 standard lib's
extern "C" long _ftol(double);
extern "C" long _ftol2(double dblSource) {
    return _ftol(dblSource);
}
#endif



END_UPP_NAMESPACE
