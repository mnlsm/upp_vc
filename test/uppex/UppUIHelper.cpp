#include "stdafx.h"
#include "UppUIHelper.h"

#define UPP_BOOL_RETURN( ret , boolptr , boolval ) do{ if( boolptr ){ *boolptr = (boolval);} return (ret); }while(false)

using namespace Upp;

UppUIHelper::UppUIHelper() {

}

UppUIHelper::~UppUIHelper() {

}

Upp::RGBA UppUIHelper::ParseColor(const char *str , bool *suc) {
    RGBA rgba ;
    rgba.r = rgba.g = rgba.b = 0x00;
    rgba.a = 0x00;
    if(str == NULL)
        UPP_BOOL_RETURN(rgba , suc , false);
    Vector<String> v = Upp::Split(str , ",");
    int count = v.GetCount();
    if(count != 4 && count != 3)
        UPP_BOOL_RETURN(rgba , suc , false);
    rgba.r = (Upp::byte)atoi(v[0]);
    rgba.g = (Upp::byte)atoi(v[1]);
    rgba.b = (Upp::byte)atoi(v[2]);
    if(count == 4)
        rgba.a = (Upp::byte)atoi(v[3]);
    UPP_BOOL_RETURN(rgba , suc , true);
}

Rect UppUIHelper::ParseRect(const char *str , bool *suc) {
    Rect rc(0 , 0 , 1 , 1);
    if(str == NULL)
        UPP_BOOL_RETURN(rc , suc , false);
    Vector<String> v = Upp::Split(str , ",");
    if(v.GetCount() != 4)
        UPP_BOOL_RETURN(rc , suc , false);
    rc.Set(atoi(v[0]) , atoi(v[1]) , atoi(v[2]) , atoi(v[3]));
    UPP_BOOL_RETURN(rc , suc , true);
}

Rect UppUIHelper::ParseSizeRect(const char *str , bool *suc) {
    Rect rc = ParseRect(str , suc);
    if(suc != NULL && *suc) {
        rc.SetSize(rc.right , rc.bottom);
    }
    return rc;
}

Upp::Size UppUIHelper::ParseSize(const char *str , bool *suc) {
    Size sz(1 , 1);
    if(str == NULL)
        UPP_BOOL_RETURN(sz , suc , false);
    Vector<String> v = Upp::Split(str , ",");
    if(v.GetCount() != 2)
        UPP_BOOL_RETURN(sz , suc , false);
    sz.cx = atoi(v[0]) ;
    sz.cy = atoi(v[1]) ;
    if(sz.cx < 1) sz.cx = 1;
    if(sz.cy < 1) sz.cy = 1;
    UPP_BOOL_RETURN(sz , suc , true);
}

Upp::Point UppUIHelper::ParsePoint(const char *str , bool *suc) {
    Point pt(0 , 0);
    if(str == NULL)
        UPP_BOOL_RETURN(pt , suc , false);
    Vector<String> v = Upp::Split(str , ",");
    if(v.GetCount() != 2)
        UPP_BOOL_RETURN(pt , suc , false);
    pt.x = atoi(v[0]);
    pt.y = atoi(v[1]);
    UPP_BOOL_RETURN(pt , suc , true);
}

bool UppUIHelper::ParseBool(const char *str , bool *suc) {
    bool b = false;
    if(str == NULL)
        UPP_BOOL_RETURN(b , suc , false);
    if(stricmp(str , "true") == 0 || strcmp(str , "1") == 0)
        b = true;
    UPP_BOOL_RETURN(b , suc , false);
}

BOOL UppUIHelper::LoadResFromXml(const char *xmlstr , const char *respath , UppResData *ret) {
    if(ret == NULL) return FALSE;
    bool suc = false;
    XmlNode xml = ParseXML(xmlstr);
    if(xml.IsEmpty())
        return FALSE;
    if(xml.GetCount() != 1)
        return FALSE;
    const XmlNode &skinnode = xml.At(0);
    if(!skinnode.IsTag("skin"))
        return FALSE;
    VectorMap<String , Upp::Color> colors;
    VectorMap<String , Upp::Image> images;
    VectorMap<String , Upp::Font > fonts;
    for(int i = 0 ; i < skinnode.GetCount() ; i++) {
        const XmlNode &resnode = skinnode.Node(i);
        if(resnode.IsTag("color")) {
            String id = resnode.Attr("id");
            if(id.IsEmpty()) return FALSE;
            Color clr(resnode.AttrInt("value") , 0);
            colors.Add(id , clr);
        }else if(resnode.IsTag("font")) {
            String id = resnode.Attr("id");
            if(id.IsEmpty()) return FALSE;
            Font fdata;
            String fontattr = resnode.Attr("facename");
            if(fontattr.IsEmpty()) fontattr = "STDFONT";
            fdata.FaceName(fontattr) ;
            fdata.Height(resnode.AttrInt("height" , 0));
            fdata.Width(resnode.AttrInt("width" , 0));
            fdata.Bold(UppUIHelper::ParseBool(resnode.Attr("bold")));
            fdata.Italic(UppUIHelper::ParseBool(resnode.Attr("italic")));
            fdata.Underline(UppUIHelper::ParseBool(resnode.Attr("underline")));
            fonts.Add(id , fdata);
        }else if(resnode.IsTag("image")) {
            String id = resnode.Attr("id");
            if(id.IsEmpty()) return FALSE;
            if(!resnode.Attr("file").IsEmpty()) {
                String file = Upp::AppendFileName(respath , resnode.Attr("file"));
                FileIn in;
                if(!in.Open(file)) return FALSE;
                Image img = StreamRaster::LoadAny(in);
                if(img.IsEmpty()) return FALSE;
                RGBA rgba = UppUIHelper::ParseColor(resnode.Attr("transcolor") , &suc);
                if(suc) {
                    img = UppUIHelper::GetTransColorImage(img , rgba);
                }
                images.Add(id , img);
            }else if(!resnode.Attr("mono").IsEmpty()) {
                bool suc = false;
                RGBA rgba = UppUIHelper::ParseColor(resnode.Attr("mono") , &suc);
                if(!suc) return false;
                Size sz = UppUIHelper::ParseSize(resnode.Attr("size") , &suc);
                if(!suc) return false;
                images.Add(id , UppUIHelper::CreateMonoImage(rgba , sz));
            }
        }else if(resnode.IsTag("images")) {
            String file = Upp::AppendFileName(respath , resnode.Attr("file"));
            FileIn in;
            if(!in.Open(file)) return FALSE;
            Image img = StreamRaster::LoadAny(in);
            if(img.IsEmpty()) return FALSE;
            for(int j = 0 ; j < resnode.GetCount() ; j++) {
                const XmlNode &partimg = resnode.Node(j);
                if(!partimg.IsTag("partimage")) return FALSE;
                String id = partimg.Attr("id");
                if(id.IsEmpty()) return FALSE;
                Rect rc = UppUIHelper::ParseSizeRect(partimg.Attr("sizerect") , &suc);
                if(!suc) return FALSE;
                Image retimg = Upp::Crop(img , rc);
                RGBA rgba = UppUIHelper::ParseColor(partimg.Attr("transcolor") , &suc);
                if(suc) {
                    retimg = UppUIHelper::GetTransColorImage(retimg , rgba);
                }
                images.Add(id , retimg);
            }
        }
    }
    ret->colors_ = colors;
    ret->images_ = images;
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Upp::Image UppUIHelper::CreateMonoImage(Upp::RGBA rgba , Upp::Size sz) {
    ImageBuffer buf(sz);
    RGBA *b = ~buf;
    RGBA *e = b + buf.GetLength();
    while(b != e) {
        *b++ = rgba;
    }
    buf.SetKind(Upp::IMAGE_ALPHA);
    return buf;
}

Upp::Image UppUIHelper::GetTransColorImage(Upp::Image img , Upp::RGBA rgba) {
    ImageBuffer buf(img);
    for(int i = 0 ; i < buf.GetHeight() ; i++) {
        for(int j = 0 ; j < buf.GetWidth() ; j++) {
            RGBA *rgbaCur = (buf[i] + j) ;
            if(rgbaCur->r == rgba.r && rgbaCur->g == rgba.g && rgbaCur->b == rgba.b) {
                memset(rgbaCur , '\0' , sizeof(RGBA));
            }
        }
    }
    return buf;
}

void UppUIHelper::DrawGridImage(Draw& w , Upp::Rect rc , Upp::Image images[9]) {
    int width = rc.GetWidth();
    int height = rc.GetHeight();
    if(width < images[0].GetWidth() + images[1].GetWidth() + images[2].GetWidth())
        width = images[0].GetWidth() + images[1].GetWidth() + images[2].GetWidth();
    if(height < images[0].GetHeight() + images[3].GetHeight() + images[6].GetHeight())
        height = images[0].GetHeight() + images[3].GetHeight() + images[6].GetHeight();
    w.DrawImage(rc.left , rc.top , images[0].GetWidth() , images[0].GetHeight() , images[0]);
    w.DrawImage(rc.left + images[0].GetWidth() , rc.top , width - images[0].GetWidth() - images[2].GetWidth(), images[1].GetHeight() , images[1]);
    w.DrawImage(rc.right - images[2].GetWidth() , rc.top , images[2].GetWidth() , images[2].GetHeight() , images[2]);

    w.DrawImage(rc.left , rc.top + images[0].GetHeight() , images[3].GetWidth() ,
                height - images[0].GetHeight() - images[6].GetHeight() , images[3]);
    w.DrawImage(rc.left + images[3].GetWidth() , rc.top + images[1].GetHeight() , width - images[3].GetWidth() - images[5].GetWidth(),
                height - images[1].GetHeight() - images[7].GetHeight() , images[4]);
    w.DrawImage(rc.right - images[5].GetWidth() , rc.top + images[2].GetHeight() , images[5].GetWidth() ,
                height - images[2].GetHeight() - images[8].GetHeight() , images[5]);

    w.DrawImage(rc.left , rc.bottom - images[6].GetHeight() , images[6].GetWidth() , images[6].GetHeight() , images[6]);
    w.DrawImage(rc.left + images[6].GetWidth() , rc.bottom - images[7].GetHeight() , width - images[6].GetWidth() - images[8].GetWidth(), images[7].GetHeight() , images[7]);
    w.DrawImage(rc.right - images[8].GetWidth() , rc.bottom - images[8].GetHeight() , images[8].GetWidth() , images[8].GetHeight() , images[8]);
}

void UppUIHelper::DrawHorz3Image(Upp::Draw& w , Upp::Rect rc , Upp::Image images[3]) {
    int width = rc.GetWidth();
    int height = rc.GetHeight();
    if(width < images[0].GetWidth() + images[1].GetWidth() + images[2].GetWidth())
        width = images[0].GetWidth() + images[1].GetWidth() + images[2].GetWidth();

    w.DrawImage(rc.left , rc.top , images[0].GetWidth() , images[0].GetHeight() , images[0]);
    w.DrawImage(rc.left + images[0].GetWidth() , rc.top , width - images[0].GetWidth() - images[2].GetWidth(), images[1].GetHeight() , images[1]);
    w.DrawImage(rc.right - images[2].GetWidth() , rc.top , images[2].GetWidth() , images[2].GetHeight() , images[2]);

}

void UppUIHelper::DrawVert3Image(Upp::Draw& w , Upp::Rect rc , Upp::Image images[3]) {
    int width = rc.GetWidth();
    int height = rc.GetHeight();
    if(height < images[0].GetWidth() + images[1].GetWidth() + images[2].GetWidth())
        height = images[0].GetWidth() + images[1].GetWidth() + images[2].GetWidth();
    w.DrawImage(rc.left , rc.top , images[0].GetWidth() , images[0].GetHeight() , images[0]);
    w.DrawImage(rc.left , rc.top + images[0].GetHeight() , images[1].GetWidth() ,
                height - images[0].GetHeight() - images[2].GetHeight() , images[1]);
    w.DrawImage(rc.left , rc.bottom - images[2].GetHeight() , images[2].GetWidth() , images[2].GetHeight() , images[2]);
}

HRGN UppUIHelper::CalcImageDrawRegion(Upp::Image &w) {
    HRGN hRgn = NULL;
    Size sz = w.GetSize();
    const RGBA *rgba  = ~w;
    DWORD maxRects = 500;
    RGNDATA *pData = (RGNDATA *)malloc(sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects));
    if(pData == NULL) return hRgn;
    pData->rdh.dwSize = sizeof(RGNDATAHEADER);
    pData->rdh.iType = RDH_RECTANGLES;
    pData->rdh.nCount = pData->rdh.nRgnSize = 0;
//  SetRect( &pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0 );
    SetRect(&pData->rdh.rcBound, 0 , 0 , sz.cx, sz.cy);

    for(int y = 0 ; y < sz.cy ; y++) {
        for(int x = 0 ; x < sz.cx ; x++) {
            int x0 = x;
            const RGBA *cur = rgba + x;
            while(x < sz.cx) {
                if(cur->a == 0)
                    break;
                x++;
                cur++;
            }
            if(x > x0) {
                if(pData->rdh.nCount >= maxRects) {
                    maxRects += 500;
                    pData = (RGNDATA *)realloc(pData , sizeof(RGNDATAHEADER) + sizeof(RECT) * maxRects);
                    if(pData == NULL) {
                        if(hRgn != NULL)
                            DeleteObject(hRgn);
                        hRgn = NULL;
                        return hRgn;
                    }
                }
                RECT *pr = (RECT *)&pData->Buffer;
                SetRect(&pr[pData->rdh.nCount], x0, y, x, y + 1);
                //if( x0 < pData->rdh.rcBound.left )
                //  pData->rdh.rcBound.left = x0;
                //if( y < pData->rdh.rcBound.top )
                //  pData->rdh.rcBound.top = y;
                //if( x > pData->rdh.rcBound.right )
                //  pData->rdh.rcBound.right = x;
                //if( y + 1 > pData->rdh.rcBound.bottom )
                //  pData->rdh.rcBound.bottom = y + 1;
                pData->rdh.nCount++;
                if(pData->rdh.nCount == 2000) {
                    HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
                    if(hRgn) {
                        CombineRgn(hRgn, hRgn, h, RGN_OR);
                        DeleteObject(h);
                    }else {
                        hRgn = h;
                    }
                    pData->rdh.nCount = 0;
                    SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);
                }
            }
        }
        rgba += sz.cx;
    }
    HRGN h = ExtCreateRegion(NULL , sizeof(RGNDATAHEADER) + sizeof(RECT) * maxRects , pData);
    if(hRgn) {
        CombineRgn(hRgn, hRgn, h, RGN_OR);
        DeleteObject(h);
    }else {
        hRgn = h;
    }
    free(pData);
    return hRgn;
}

void UppUIHelper::GetImageDrawClipRegionData(Upp::Image &w , std::vector<ClipRegionData> *vClips) {
    if(vClips == NULL) return ;
    vClips->clear();
    Size sz = w.GetSize();
    const RGBA *rgba  = ~w;
    for(int y = 0 ; y < sz.cy ; y++) {
        for(int x = 0 ; x < sz.cx ; x++) {
            int x0 = -1;
            int x1 = -1;
            const RGBA *cur = rgba + x;
            while(x < sz.cx && cur->a == 0) {
                if(x0 < 0) {
                    x0 = x;
                    x1 = x;
                }else
                    x1 = x;
                x++;
                cur++;
            }
            if(x0 <= x1 && x1 != -1) {
                ClipRegionData crd;
                crd.pos.x = Ctrl::PosLeft(x0 , x1 - x0 + 1);
                crd.pos.y = Ctrl::PosTop(y , 1);
                vClips->push_back(crd);
            }
        }
        rgba += sz.cx;
    }
    return;
}

HRGN UppUIHelper::CalcClipRegion(const RECT &rc , const std::vector<ClipRegionData> *vClips) {
    if(vClips == NULL) return NULL;
    if(vClips->empty()) return NULL;
    HRGN hRgn = CreateRectRgn(0 , 0 , rc.right - rc.left + 1, rc.bottom - rc.top + 1);
    for(int i = 0 ; i < vClips->size() ; i++) {
        const ClipRegionData &posdata = vClips->at(i);
        RECT rcItem = {0};
        if(posdata.pos.x.GetAlign() == Ctrl::LEFT) {
            rcItem.left = posdata.pos.x.GetA();
            rcItem.right = rcItem.left + (posdata.pos.x.GetB());
        }else if(posdata.pos.x.GetAlign() == Ctrl::RIGHT) {
            rcItem.left = rc.right - posdata.pos.x.GetA() - posdata.pos.x.GetB() ;
            rcItem.right = rcItem.left + (posdata.pos.x.GetB());
        }
        if(posdata.pos.y.GetAlign() == Ctrl::TOP) {
            rcItem.top = posdata.pos.y.GetA();
            rcItem.bottom = rcItem.top + posdata.pos.y.GetB() ;
        }else if(posdata.pos.y.GetAlign() == Ctrl::BOTTOM) {
            rcItem.top = rc.bottom - posdata.pos.y.GetA() - posdata.pos.y.GetB() ;
            rcItem.bottom = rcItem.top + posdata.pos.y.GetB() ;
        }
        HRGN tmp = CreateRectRgn(rcItem.left , rcItem.top , rcItem.right , rcItem.bottom);
        if(tmp != NULL) {
            CombineRgn(hRgn , hRgn , tmp , RGN_DIFF);
            DeleteObject(tmp);
        }
    }
    OffsetRgn(hRgn , rc.left , rc.top);
    return hRgn;
}
