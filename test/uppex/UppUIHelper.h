#ifndef UPPUIHELPER_H__
#define UPPUIHELPER_H__

#include <core/core.h>
#include <draw/Draw.h>
#include <ctrlcore/SystemDraw.h>
#include <ctrllib/CtrlLib.h>



typedef struct tagUppResData {
    Upp::VectorMap< Upp::String , Upp::Color > colors_;
    Upp::VectorMap< Upp::String , Upp::Image > images_;
    Upp::VectorMap< Upp::String , Upp::Font > fonts_;
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


class UppUIHelper {
private:
    UppUIHelper();
    ~UppUIHelper();

public:
    static Upp::Rect ParseRect(const char *str , bool *suc = NULL);
    static Upp::Rect ParseSizeRect(const char *str , bool *suc = NULL);
    static Upp::Size ParseSize(const char *str , bool *suc = NULL);
    static Upp::Point ParsePoint(const char *str , bool *suc = NULL);
    static bool ParseBool(const char *str , bool *suc = NULL);
    static Upp::RGBA ParseColor(const char *str , bool *suc = NULL);

public:
    static BOOL LoadResFromXml(const char *xmlstr , const char *respath , UppResData *ret);



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






};

#endif