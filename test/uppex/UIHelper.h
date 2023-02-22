#ifndef UIHelper_H__
#define UIHelper_H__
#pragma once

#include <memory>


#pragma warning(push, 3)
#pragma push_macro("new")
#undef new
#include <gdiplus.h>
#pragma pop_macro("new")
#pragma warning( pop )
#include <shlwapi.h>


#include <core/core.h>
#include <draw/Draw.h>
#include <ctrlcore/SystemDraw.h>
#include <ctrllib/CtrlLib.h>

#include <lib/marisa/sec-trie.h>
#include <lib/marisa/grimoire/io/mapper.h>
#include <lib/marisa/grimoire/io/writer.h>

#ifdef flagNONAMESPACE
#define NAMESPACE_UPPEX
#define END_UPPEX_NAMESPACE
#define UPPEX
#else
#define NAMESPACE_UPPEX     namespace Uppex {
#define END_UPPEX_NAMESPACE };
#define UPPEX               Uppex
#endif

NAMESPACE_UPPEX


typedef struct tagUppResData {
    Upp::String resname_;

    Upp::VectorMap< Upp::String , Upp::Color > colors_;
    Upp::VectorMap< Upp::String , Upp::Image > images_;
    Upp::VectorMap< Upp::String , Upp::Font > fonts_;

    Upp::VectorMap< Upp::String , Upp::Image > small_icons_;
    Upp::VectorMap< Upp::String , Upp::Image > large_icons_;
    Upp::VectorMap< Upp::String , Upp::Image > cursors_;

    void Swap(tagUppResData& other) {
        if(this != &other) {
            Upp::Swap(resname_, other.resname_);
            Upp::Swap(colors_, other.colors_);
            Upp::Swap(images_, other.images_);
            Upp::Swap(fonts_, other.fonts_);
            Upp::Swap(small_icons_, other.small_icons_);
            Upp::Swap(large_icons_, other.large_icons_);
            Upp::Swap(cursors_, other.cursors_);
        }
    }

} UppResData , *LPUppResData;

typedef struct tagClipRegionData {
    Upp::Ctrl::LogPos pos;
    bool operator==(const tagClipRegionData &other) {
        return pos == other.pos ;
    }
    bool operator<(const tagClipRegionData &other) {
        if(pos.x.GetAlign() >= other.pos.x.GetAlign() || pos.y.GetAlign() >= other.pos.y.GetAlign())
            return false;
        if(pos.x.GetA() >= other.pos.x.GetA() || pos.y.GetA() >= other.pos.y.GetA())
            return false;
        if(pos.x.GetB() >= other.pos.x.GetB() || pos.y.GetB() >= other.pos.y.GetB())
            return false;
        return true;
    }
} ClipRegionData, *LPClipRegionData;


class UIHelper {
private:
    UIHelper();
    ~UIHelper();

public:
    static Upp::Rect ParseRect(const char *str , bool *suc = NULL);
    static Upp::Rect ParseSizeRect(const char *str , bool *suc = NULL);
    static Upp::Size ParseSize(const char *str , bool *suc = NULL);
    static Upp::Point ParsePoint(const char *str , bool *suc = NULL);
    static bool ParseBool(const char *str , bool *suc = NULL);
    static Upp::RGBA ParseColor(const char *str , bool *suc = NULL);

public:
    static BOOL LoadResFromXml(const char *xmlstr , const char *respath , UppResData *ret);
    static BOOL LoadResFromMarisa(marisa::SecTrie& trie , const char *respath , UppResData *ret);
    static BOOL ExtraDataFromTrie(marisa::SecTrie& trie, const Upp::String& key, Upp::String& filedata);
    static BOOL ExtraImageFromTrie(marisa::SecTrie& trie, const Upp::String& key, Upp::Image& image);
    static BOOL ExtraXmlFromTrie(marisa::SecTrie& trie, const Upp::String& key, Upp::String& xmlstr);

public:
    static BOOL SetCtrlParam(Upp::Ctrl* ctrl, Upp::String attrId, Upp::String attrVal);
    


public:
    static Upp::Image CreateMonoImage(Upp::RGBA rgba , Upp::Size sz);
    static Upp::Image GetTransColorImage(Upp::Image img , Upp::RGBA rgba);
    static void DrawGridImage(Upp::Draw& w , Upp::Rect rc , Upp::Image images[9]);
    static void DrawHorz3Image(Upp::Draw& w , Upp::Rect rc , Upp::Image images[3]);
    static void DrawVert3Image(Upp::Draw& w , Upp::Rect rc , Upp::Image images[3]);

public:
    static HRGN CalcImageDrawRegion(Upp::Image &w);
    static void GetImageDrawClipRegionData(Upp::Image &w , std::vector<ClipRegionData> *vClips);
    static HRGN CalcClipRegion(const RECT &rc , const std::vector<ClipRegionData> *vClips);


public:
    static void AdjustMarisaInnerFileData(Upp::String& filedata);
    static BOOL LoadGdiplusImageFromBuffer(PBYTE buffer, DWORD bufferSize, std::tr1::shared_ptr<Gdiplus::Bitmap>& ret);

};


class CInitGDIPlus {
public:
	CInitGDIPlus();
	~CInitGDIPlus();

	bool Init();
	void ReleaseGDIPlus();

private:
	ULONG_PTR m_dwToken;
	CRITICAL_SECTION m_sect;
	LONG m_nCImageObjects;
};

END_UPPEX_NAMESPACE

#endif