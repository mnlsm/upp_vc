#include "Core.h"

#include <wingdi.h>
#include <winnls.h>


NAMESPACE_UPP

#define LLOG(x)  LOG(x)

String LNGAsText(int d) {
    String result;
    int c = (d >> 15) & 31;
    if(c) {
        result.Cat(c + 'A' - 1);
        c = (d >> 10) & 31;
        if(c) {
            result.Cat(c + 'A' - 1);
            c = (d >> 5) & 31;
            if(c) {
                result.Cat('-');
                result.Cat(c + 'A' - 1);
                c = d & 31;
                if(c) result.Cat(c + 'A' - 1);
            }
        }
    }
    c = (d >> 20) & 255;
    if(c)
        result << ' ' << CharsetName(c);
    return result;
}

byte GetLNGCharset(int d) {
    byte cs = byte(d >> 20);
    return cs ? cs : CHARSET_UTF8;
}

int  SetLNGCharset(int lng, byte chrset) {
    return (lng & ~(0xffffffff << 20)) | (chrset << 20);
}

int LNGFromText(const char *s) {
    int l = 0;
    if(IsAlpha(*s)) {
        l = (ToUpper(*s++) - 'A' + 1) << 15;
        if(IsAlpha(*s)) {
            l |= (ToUpper(*s++) - 'A' + 1) << 10;
            if(*s && !IsAlpha(*s))
                s++;
            if(IsAlpha(*s)) {
                l |= (ToUpper(*s++) - 'A' + 1) << 5;
                if(IsAlpha(*s)) {
                    l |= (ToUpper(*s++) - 'A' + 1);
                    while(*s && *s != ' ')
                        s++;
                    if(*s == ' ') {
                        s++;
                        int cs = CharsetByName(s);
                        if(cs > 0)
                            l |= (cs << 20);
                        else
                            return 0;
                    }
                    return l;
                }
            }
        }
    }
    return 0;
}



String GetUserLocale(dword type) {
    char h[256];
    int n =:: GetLocaleInfoA(GetUserDefaultLCID(), type, h, 256);
    return n ? String(h, n - 1) : String();
}

int GetSystemLNG() {
    static int lang;
    ONCELOCK {
        lang = LNGFromText(GetUserLocale(LOCALE_SISO639LANGNAME) + GetUserLocale(LOCALE_SISO3166CTRYNAME));
        if(!lang)
            lang = LNG_ENGLISH;
        int cs = atoi(GetUserLocale(LOCALE_IDEFAULTANSICODEPAGE));
        if(cs >= 1250 && cs <= 1258)
            lang = SetLNGCharset(lang, CHARSET_WIN1250 + cs - 1250);
    }
    return lang;
}



class LangConvertClass : public Convert {
    virtual Value  Format(const Value& q) const {
        return LNGAsText((int)q);
    }

    virtual Value  Scan(const Value& text) const {
        if(IsNull(text)) return 0;
        int q = LNGFromText((String)text);
        if(!q) return ErrorValue(t_("Invalid language specification."));
        return (int) q;
    }

    virtual int    Filter(int chr) const {
        return chr == ' ' || chr == '-' || IsDigit(chr) ? chr : IsAlpha(chr) ? ToUpper(chr) : 0;
    }
};

Convert& LNGConvert() {
    return Single<LangConvertClass>();
}

static int sCurrentLanguage = -1;

int  GetCurrentLanguage() {
    return sCurrentLanguage;
}

void SetLanguage(int lang) {
    if(lang != LNG_CURRENT) {
        sCurrentLanguage = lang;
        SetDefaultCharset(GetLNGCharset(lang));
    }
    SetCurrentLanguage(lang);
}

String GetLangName(int language) {
    return GetLanguageInfo(language).english_name;
}

String GetNativeLangName(int language) {
    return GetLanguageInfo(language).native_name.ToString();
}

END_UPP_NAMESPACE
