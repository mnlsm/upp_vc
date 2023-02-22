#include "stdafx.h"
#include "UppUIHelper.h"

#ifndef _ATL_NO_DEFAULT_LIBS
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "gdiplus.lib")
#if WINVER >= 0x0500
#pragma comment(lib, "msimg32.lib")
#endif  // WINVER >= 0x0500
#endif  // !_ATL_NO_DEFAULT_LIBS


#define UPP_BOOL_RETURN( ret , boolptr , boolval ) do{ if( boolptr ){ *boolptr = (boolval);} return (ret); }while(false)

using namespace Upp;

namespace {
class CStreamOnByteArray 
    :public IStream {
public:
	BYTE *m_pArray;
	DWORD m_dwRead;
    DWORD m_dwLength;

	CStreamOnByteArray(BYTE *pBytes, DWORD dwLen) throw() {
		ATLASSERT(pBytes);
		m_pArray = pBytes;
        m_dwLength = dwLen;
		m_dwRead = 0;
	}

	STDMETHOD(Read)(void *pv, ULONG cb, ULONG *pcbRead) throw() {
		if (!pv)
			return E_INVALIDARG;
		if (cb == 0)
			return S_OK;
		if (!m_pArray)
			return E_UNEXPECTED;
		BYTE *pCurr  = m_pArray;
		pCurr += m_dwRead;
		Checked::memcpy_s(pv, cb, pCurr, cb);
		if (pcbRead)
			*pcbRead = cb;
		m_dwRead += cb;
		return S_OK;
	}

	STDMETHOD(Write)(const void* , ULONG , ULONG* ) throw() {
		return E_UNEXPECTED;
	}

	STDMETHOD(Seek)(LARGE_INTEGER diff, DWORD dwOrigin, ULARGE_INTEGER *pNewPos) throw()
	{
        if(STREAM_SEEK_CUR == dwOrigin) {
            if(m_dwRead + diff.LowPart > m_dwLength) {
                return E_FAIL;
            }
            m_dwRead = m_dwRead + diff.LowPart;
        } else if(STREAM_SEEK_SET == dwOrigin) {
            if(diff.LowPart > m_dwLength) {
                return E_FAIL;
            }
            m_dwRead = diff.LowPart;
        } else if(STREAM_SEEK_END == dwOrigin) {
            if(diff.LowPart > m_dwLength) {
                return E_FAIL;
            }
            m_dwRead = m_dwLength - diff.LowPart;
        } else {
            return E_FAIL;
        }
        if(pNewPos != NULL) {
            (*pNewPos).HighPart = 0;
            (*pNewPos).LowPart = m_dwRead;
        }
		return S_OK;
	}

	STDMETHOD(SetSize)(ULARGE_INTEGER ) throw() {
		return E_NOTIMPL;
	}

	STDMETHOD(CopyTo)(IStream *, ULARGE_INTEGER , ULARGE_INTEGER *,
		ULARGE_INTEGER *) throw() {
		return E_NOTIMPL;
	}

	STDMETHOD(Commit)(DWORD ) throw() {
		return E_NOTIMPL;
	}

	STDMETHOD(Revert)( void) throw() {
		return E_NOTIMPL;
	}

	STDMETHOD(LockRegion)(ULARGE_INTEGER , ULARGE_INTEGER , DWORD ) throw() {
		return E_NOTIMPL;
	}

	STDMETHOD(UnlockRegion)(ULARGE_INTEGER , ULARGE_INTEGER ,
		DWORD ) throw() {
		return E_NOTIMPL;
	}

	STDMETHOD(Stat)(STATSTG *pstatstg, DWORD flags) throw() {
        if(pstatstg != NULL) {
            pstatstg->cbSize.HighPart = 0;
            pstatstg->cbSize.LowPart = m_dwLength;
        }
        return S_OK;
		return E_NOTIMPL;
	}

	STDMETHOD(Clone)(IStream **) throw() {
		return E_NOTIMPL;
	}

	STDMETHOD(QueryInterface)(REFIID iid, void **ppUnk) throw() {
		*ppUnk = NULL;
		if (::InlineIsEqualGUID(iid, IID_IUnknown) ||
			::InlineIsEqualGUID(iid, IID_ISequentialStream) ||
			::InlineIsEqualGUID(iid, IID_IStream)) {
			*ppUnk = (void*)(IStream*)this;
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	ULONG STDMETHODCALLTYPE AddRef( void)  throw() {
		return (ULONG)1;
	}

	ULONG STDMETHODCALLTYPE Release( void)  throw() {
		return (ULONG)1;
	}
};



}

NAMESPACE_UPPEX

UppUIHelper::UppUIHelper() {
}

UppUIHelper::~UppUIHelper() {
}

void UppUIHelper::AdjustMarisaInnerFileData(Upp::String& filedata) {
    char mask = (char)(filedata.GetLength() % 0xFF);
    for(size_t ii = 0; ii < filedata.GetLength(); ii++) {
        filedata.Set(ii, (filedata[ii] ^ mask));
    }
}

Upp::RGBA UppUIHelper::ParseColor(const char *str , bool *suc) {
    RGBA rgba ;
    rgba.r = rgba.g = rgba.b = 0x00;
    rgba.a = 0xFF;
    if(str == NULL) {
        UPP_BOOL_RETURN(rgba , suc , false);
    }
    String ustr(str);
    TrimBoth(ustr);
    if(ustr.StartsWith("0x") ||ustr.StartsWith("0X") || ustr.StartsWith("#")) {
        char* h = NULL;
        if(ustr.GetLength() == 8) {
            rgba.r = (Upp::byte)strtol(ustr.Mid(2, 2), &h, 16);
            rgba.g = (Upp::byte)strtol(ustr.Mid(4, 2), &h, 16);
            rgba.b = (Upp::byte)strtol(ustr.Mid(6, 2), &h, 16);
            UPP_BOOL_RETURN(rgba , suc , true);            
        } else if(ustr.GetLength() == 10) {
            rgba.a = (Upp::byte)strtol(ustr.Mid(2, 2), &h, 16);
            rgba.r = (Upp::byte)strtol(ustr.Mid(4, 2), &h, 16);
            rgba.g = (Upp::byte)strtol(ustr.Mid(6, 2), &h, 16);
            rgba.b = (Upp::byte)strtol(ustr.Mid(8, 2), &h, 16);
            UPP_BOOL_RETURN(rgba , suc , true);
        }
        UPP_BOOL_RETURN(rgba , suc , false);
    }
    Vector<String> v = Upp::Split(str , ",");
    int count = v.GetCount();
    if(count != 4 && count != 3) {
        UPP_BOOL_RETURN(rgba , suc , false);
    }
    rgba.r = (Upp::byte)atoi(TrimBoth(v[0]));
    rgba.g = (Upp::byte)atoi(TrimBoth(v[1]));
    rgba.b = (Upp::byte)atoi(TrimBoth(v[2]));
    if(count == 4) {
        rgba.a = (Upp::byte)atoi(TrimBoth(v[3]));
    }
    UPP_BOOL_RETURN(rgba , suc , true);
}

Rect UppUIHelper::ParseRect(const char *str , bool *suc) {
    Rect rc(0 , 0 , 1 , 1);
    if(str == NULL) {
        UPP_BOOL_RETURN(rc , suc , false);
    }
    Vector<String> v = Upp::Split(str , ",");
    if(v.GetCount() != 4) {
        UPP_BOOL_RETURN(rc , suc , false);
    }
    rc.Set(atoi(TrimBoth(v[0])) , atoi(TrimBoth(v[1])) , atoi(TrimBoth(v[2])) , atoi(TrimBoth(v[3])));
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
    if(str == NULL) {
        UPP_BOOL_RETURN(sz , suc , false);
    }
    Vector<String> v = Upp::Split(str , ",");
    if(v.GetCount() != 2) {
        UPP_BOOL_RETURN(sz , suc , false);
    }
    sz.cx = atoi(TrimBoth(v[0])) ;
    sz.cy = atoi(TrimBoth(v[1])) ;
    if(sz.cx < 1) sz.cx = 1;
    if(sz.cy < 1) sz.cy = 1;
    UPP_BOOL_RETURN(sz , suc , true);
}

Upp::Point UppUIHelper::ParsePoint(const char *str , bool *suc) {
    Point pt(0 , 0);
    if(str == NULL) {
        UPP_BOOL_RETURN(pt , suc , false);
    }
    Vector<String> v = Upp::Split(str , ",");
    if(v.GetCount() != 2) {
        UPP_BOOL_RETURN(pt , suc , false);
    }
    pt.x = atoi(TrimBoth(v[0]));
    pt.y = atoi(TrimBoth(v[1]));
    UPP_BOOL_RETURN(pt , suc , true);
}

bool UppUIHelper::ParseBool(const char *str , bool *suc) {
    bool b = false;
    if(str == NULL) {
        UPP_BOOL_RETURN(b , suc , false);
    }
    if(stricmp(str , "true") == 0 || strcmp(str , "1") == 0) {
        b = true;
    }
    UPP_BOOL_RETURN(b , suc , false);
}

BOOL UppUIHelper::LoadResFromXml(const char *xmlstr , const char *respath , UppResData *ret) {
    if(ret == NULL) return FALSE;
    bool suc = false;
    XmlNode xml = ParseXML(xmlstr);
    if(xml.IsEmpty()) {
        assert(false);     
        return FALSE;
    }
    if(xml.GetCount() != 1) {
        assert(false);
        return FALSE;
    }
    const XmlNode &skinnode = xml.At(0);
    if(!skinnode.IsTag("skin")) {
        assert(false);
        return FALSE;
    }
    ret->resname_ = skinnode.Attr("name");
    if(ret->resname_.IsEmpty()) {
        assert(false);
        return FALSE;
    }
    VectorMap<String , Upp::Color> colors;
    VectorMap<String , Upp::Image> images;
    VectorMap<String , Upp::Font > fonts;
    Upp::VectorMap< Upp::String , Upp::Image > small_icons;
    Upp::VectorMap< Upp::String , Upp::Image > large_icons;
    Upp::VectorMap< Upp::String , Upp::Image > cursors;
    for(int i = 0 ; i < skinnode.GetCount() ; i++) {
        const XmlNode &resnode = skinnode.Node(i);
        if(resnode.IsTag("color")) {
            bool suc = false;
            String id = resnode.Attr("id");
            if(id.IsEmpty()) {
                assert(false);
                return FALSE;
            }
            RGBA rgba = UppUIHelper::ParseColor(resnode.Attr("value") , &suc);
            if(suc) {
                colors.Add(id , Color(rgba));
            }
        }else if(resnode.IsTag("font")) {
            String id = resnode.Attr("id");
            if(id.IsEmpty()) {
                assert(false);
                return FALSE;
            }
            Font fdata;
            String fontface = resnode.Attr("facename");
            if(fontface.IsEmpty()) fontface = "STDFONT";
            fdata.FaceName(fontface) ;
            int font_size = abs(resnode.AttrInt("pointsize" , 9));
            int nHeight = -MulDiv(font_size, GetDeviceCaps(Win32_IC(), LOGPIXELSY), 72);
            fdata.Height(nHeight);
            fdata.Width(resnode.AttrInt("width" , 0));
            fdata.Bold(UppUIHelper::ParseBool(resnode.Attr("bold")));
            fdata.Italic(UppUIHelper::ParseBool(resnode.Attr("italic")));
            fdata.Underline(UppUIHelper::ParseBool(resnode.Attr("underline")));
            if(fontface == "STDFONT") {
                SetStdFont(fdata);
            }
            fonts.Add(id , fdata);
        }else if(resnode.IsTag("image")) {
            String id = resnode.Attr("id");
            if(id.IsEmpty()) {
                assert(false);
                return FALSE;
            }
            if(!resnode.Attr("file").IsEmpty()) {
				String filename = resnode.Attr("file");
				filename.Replace(DIR_UNIX_SEPS, DIR_SEPS);
                String file = Upp::AppendFileName(respath , filename);
                FileIn in;
                if(!in.Open(file)) {
                    assert(false);
                    return FALSE;
                }
                Image img = StreamRaster::LoadAny(in);
                if(img.IsEmpty()) {
                    assert(false);
                    return FALSE;
                }
                RGBA rgba = UppUIHelper::ParseColor(resnode.Attr("transcolor") , &suc);
                if(suc) {
                    img = UppUIHelper::GetTransColorImage(img , rgba);
                }
                images.Add(id , img);
            }else if(!resnode.Attr("mono").IsEmpty()) {
                bool suc = false;
                RGBA rgba = UppUIHelper::ParseColor(resnode.Attr("mono") , &suc);
                if(!suc) {
                    assert(false);
                    return FALSE;
                }
                Size sz = UppUIHelper::ParseSize(resnode.Attr("size") , &suc);
                if(!suc) {
                    assert(false);
                    return FALSE;
                }
                images.Add(id , UppUIHelper::CreateMonoImage(rgba , sz));
            }
        }else if(resnode.IsTag("images")) {
			String filename = resnode.Attr("file");
			filename.Replace(DIR_UNIX_SEPS, DIR_SEPS);
            String file = Upp::AppendFileName(respath , filename);
            FileIn in;
            if(!in.Open(file)) {
                assert(false);
                return FALSE;
            }
            Image img = StreamRaster::LoadAny(in);
            if(img.IsEmpty()) {
                assert(false);
                return FALSE;
            }
            for(int j = 0 ; j < resnode.GetCount() ; j++) {
                const XmlNode &partimg = resnode.Node(j);
                if(!partimg.IsTag("partimage")) {
                    assert(false);
                    return FALSE;
                }
                String id = partimg.Attr("id");
                if(id.IsEmpty()) {
                    assert(false);
                    return FALSE;
                }
                Rect rc = UppUIHelper::ParseSizeRect(partimg.Attr("sizerect") , &suc);
                if(!suc) {
                    assert(false);
                    return FALSE;
                }
                Image retimg = Upp::Crop(img , rc);
                RGBA rgba = UppUIHelper::ParseColor(partimg.Attr("transcolor") , &suc);
                if(suc) {
                    retimg = UppUIHelper::GetTransColorImage(retimg , rgba);
                }
                images.Add(id , retimg);
            }
        }else if(resnode.IsTag("icon")) {
            String id = resnode.Attr("id");
            if(id.IsEmpty()) {
                assert(false);
                return FALSE;
            }
            if(!resnode.Attr("file").IsEmpty()) {
				String filename = resnode.Attr("file");
				filename.Replace(DIR_UNIX_SEPS, DIR_SEPS);
                String file = Upp::AppendFileName(respath , filename);
                file = Upp::ToSystemCharset(file);
                HICON small_icon = (HICON)::LoadImageA(NULL, ~file, IMAGE_ICON, 
                    GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CXSMICON), 
                    LR_DEFAULTCOLOR|LR_DEFAULTSIZE|LR_LOADFROMFILE);
                HICON large_icon = (HICON)::LoadImageA(NULL, ~file, IMAGE_ICON, 
                    GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CXICON),
                    LR_DEFAULTCOLOR|LR_DEFAULTSIZE|LR_LOADFROMFILE);
                if(small_icon == NULL || large_icon == NULL) {
                    assert(false);
                    return FALSE;
                } else {
                    small_icons.Add(id, Upp::sWin32Icon(small_icon, false));
                    large_icons.Add(id, Upp::sWin32Icon(large_icon, false));
                }
            }
        }else if(resnode.IsTag("cursor")) {
            String id = resnode.Attr("id");
            if(id.IsEmpty()) {
                assert(false);
                return FALSE;
            }
            if(!resnode.Attr("file").IsEmpty()) {
				String filename = resnode.Attr("file");
				filename.Replace(DIR_UNIX_SEPS, DIR_SEPS);
                String file = Upp::AppendFileName(respath , filename);
                file = Upp::ToSystemCharset(file);
                HCURSOR hc = (HCURSOR)::LoadImageA(NULL, ~file, IMAGE_CURSOR, 
                    GetSystemMetrics(SM_CXCURSOR), GetSystemMetrics(SM_CXCURSOR),
                    LR_DEFAULTCOLOR|LR_DEFAULTSIZE|LR_LOADFROMFILE);
                if(hc == NULL) {
                    assert(false);
                    return FALSE;                
                } else {
                    cursors.Add(id, Upp::sWin32Icon(hc, true));
                }
            }
        }
    }
    ret->colors_ = colors;
    ret->images_ = images;
    ret->fonts_ = fonts;
    ret->small_icons_ = small_icons;
    ret->large_icons_ = large_icons;
    ret->cursors_ = cursors;
    return TRUE;
}

BOOL UppUIHelper::LoadResFromMarisa(marisa::SecTrie& trie , const char *respath , UppResData *ret) {
    String xmlstr;
    try {
        String resid_key("res.xml");
        UppUIHelper::ExtraXmlFromTrie(trie, resid_key, xmlstr);
    } catch(marisa::Exception& e) {
        assert(false);
        e;
        return FALSE;
    } catch(...) {
        assert(false);
        return FALSE;
    }
    bool suc = false;
    XmlNode xml = ParseXML(xmlstr);
    if(xml.IsEmpty()) {
        assert(false);
        return FALSE;
    }
    if(xml.GetCount() != 1) {
        assert(false);
        return FALSE;
    }
    const XmlNode &skinnode = xml.At(0);
    if(!skinnode.IsTag("skin")) {
        assert(false);
        return FALSE;
    }
    ret->resname_ = skinnode.Attr("name");
    if(ret->resname_.IsEmpty()) {
        assert(false);    
        return FALSE;
    }
    VectorMap<String , Upp::Color> colors;
    VectorMap<String , Upp::Image> images;
    VectorMap<String , Upp::Font > fonts;
    Upp::VectorMap< Upp::String , Upp::Image > small_icons;
    Upp::VectorMap< Upp::String , Upp::Image > large_icons;
    Upp::VectorMap< Upp::String , Upp::Image > cursors;
    for(int i = 0 ; i < skinnode.GetCount() ; i++) {
        const XmlNode &resnode = skinnode.Node(i);
        if(resnode.IsTag("color")) {
            bool suc = false;
            String id = resnode.Attr("id");
            if(id.IsEmpty()) {
                assert(false);
                return FALSE;
            }
            RGBA rgba = UppUIHelper::ParseColor(resnode.Attr("value") , &suc);
            if(suc) {
                colors.Add(id , Color(rgba));
            }
        }else if(resnode.IsTag("font")) {
            String id = resnode.Attr("id");
            if(id.IsEmpty()) {
                assert(false);
                return FALSE;
            }
            Font fdata;
            String fontface = resnode.Attr("facename");
            if(fontface.IsEmpty()) fontface = "STDFONT";
            fdata.FaceName(fontface) ;
            int font_size = abs(resnode.AttrInt("pointsize" , 9));
            int nHeight = -MulDiv(font_size, GetDeviceCaps(Win32_IC(), LOGPIXELSY), 72);
            fdata.Height(nHeight);
            fdata.Width(resnode.AttrInt("width" , 0));
            fdata.Bold(UppUIHelper::ParseBool(resnode.Attr("bold")));
            fdata.Italic(UppUIHelper::ParseBool(resnode.Attr("italic")));
            fdata.Underline(UppUIHelper::ParseBool(resnode.Attr("underline")));
            if(fontface == "STDFONT") {
                SetStdFont(fdata);
            }
            fonts.Add(id , fdata);
        }else if(resnode.IsTag("image")) {
            String id = resnode.Attr("id");
            if(id.IsEmpty()) {
                assert(false);
                return FALSE;
            }
            if(!resnode.Attr("file").IsEmpty()) {
                String file = resnode.Attr("file");
                Image img;
                if(!ExtraImageFromTrie(trie, file, img)) {
                    assert(false);
                    return FALSE;
                }
                RGBA rgba = UppUIHelper::ParseColor(resnode.Attr("transcolor") , &suc);
                if(suc) {
                    img = UppUIHelper::GetTransColorImage(img , rgba);
                }
                images.Add(id , img);
            }else if(!resnode.Attr("mono").IsEmpty()) {
                bool suc = false;
                RGBA rgba = UppUIHelper::ParseColor(resnode.Attr("mono") , &suc);
                if(!suc) {
                    assert(false);
                    return FALSE;
                }
                Size sz = UppUIHelper::ParseSize(resnode.Attr("size") , &suc);
                if(!suc) {
                    assert(false);
                    return false;
                }
                images.Add(id , UppUIHelper::CreateMonoImage(rgba , sz));
            }
        }else if(resnode.IsTag("images")) {
            String file = resnode.Attr("file");
            Image img;
            if(!ExtraImageFromTrie(trie, file, img)) {
                assert(false);
                return FALSE;
            }
            for(int j = 0 ; j < resnode.GetCount() ; j++) {
                const XmlNode &partimg = resnode.Node(j);
                if(!partimg.IsTag("partimage")) {
                    assert(false);
                    return FALSE;
                }
                String id = partimg.Attr("id");
                if(id.IsEmpty()) {
                    assert(false);
                    return FALSE;
                }
                Rect rc = UppUIHelper::ParseSizeRect(partimg.Attr("sizerect") , &suc);
                if(!suc) {
                    assert(false);
                    return FALSE;
                }
                Image retimg = Upp::Crop(img , rc);
                RGBA rgba = UppUIHelper::ParseColor(partimg.Attr("transcolor") , &suc);
                if(suc) {
                    retimg = UppUIHelper::GetTransColorImage(retimg , rgba);
                }
                images.Add(id , retimg);
            }
        }else if(resnode.IsTag("icon")) {
            String id = resnode.Attr("id");
            if(id.IsEmpty()) {
                assert(false);
                return FALSE;
            }
            if(!resnode.Attr("file").IsEmpty()) {
                String file = resnode.Attr("file");
                String filedata;
                if(!UppUIHelper::ExtraDataFromTrie(trie, file, filedata)) {
                    assert(false);
                    return FALSE;
                }
                file = GetTempFileName("icon_");
                if(true) {
                    FileOut out(~file);
                    out.Put(filedata);
                }
                filedata = file;
                file = Upp::ToSystemCharset(file);
                HICON small_icon = (HICON)::LoadImageA(NULL, ~file, IMAGE_ICON, 
                    GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CXSMICON), 
                    LR_DEFAULTCOLOR|LR_DEFAULTSIZE|LR_LOADFROMFILE);
                HICON large_icon = (HICON)::LoadImageA(NULL, ~file, IMAGE_ICON, 
                    GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CXICON),
                    LR_DEFAULTCOLOR|LR_DEFAULTSIZE|LR_LOADFROMFILE);
                FileDelete(~filedata);
                if(small_icon != NULL) {
                    small_icons.Add(id, Upp::sWin32Icon(small_icon, false));
                } else {
                    assert(false);
                    return FALSE;
                }
                if(large_icon != NULL) {
                    large_icons.Add(id, Upp::sWin32Icon(large_icon, false));
                } else {
                    assert(false);
                    return FALSE;
                }
            }
        }else if(resnode.IsTag("cursor")) {
            String id = resnode.Attr("id");
            if(id.IsEmpty()) {
                assert(false);             
                return FALSE;
            }
            if(!resnode.Attr("file").IsEmpty()) {
                String file = resnode.Attr("file");
                String filedata;
                if(!UppUIHelper::ExtraDataFromTrie(trie, file, filedata)) {
                    assert(false);
                    return FALSE;
                }
                file = GetTempFileName("cursor_");
                if(true) {
                    FileOut out(~file);
                    out.Put(filedata);
                }
                filedata = file;
                file = Upp::ToSystemCharset(file);
                HCURSOR hc = (HCURSOR)::LoadImageA(NULL, ~file, IMAGE_CURSOR, 
                    GetSystemMetrics(SM_CXCURSOR), GetSystemMetrics(SM_CXCURSOR),
                    LR_DEFAULTCOLOR|LR_DEFAULTSIZE|LR_LOADFROMFILE);
                FileDelete(~filedata);
                if(hc != NULL) {
                    cursors.Add(id, Upp::sWin32Icon(hc, true));
                } else {
                    assert(false);
                    return FALSE;
                }
            }
        }
    }
    ret->colors_ = colors;
    ret->images_ = images;
    ret->fonts_ = fonts;
    ret->small_icons_ = small_icons;
    ret->large_icons_ = large_icons;
    ret->cursors_ = cursors;
    return TRUE;
}

BOOL UppUIHelper::ExtraDataFromTrie(marisa::SecTrie& trie, const Upp::String& key, Upp::String& filedata) {
    filedata.Clear();
    size_t keysize = key.GetLength() + 1;
    marisa::Agent agent;
    agent.set_query(~key, keysize);
    if(!trie.predictive_search(agent)) {
        assert(false);
        return FALSE;
    }
    filedata.Cat(agent.key().ptr() + keysize, agent.key().length() - keysize);
    AdjustMarisaInnerFileData(filedata);
    return TRUE;
}

BOOL UppUIHelper::ExtraImageFromTrie(marisa::SecTrie& trie, const Upp::String& key, Upp::Image& image) {
    Upp::String filedata;
    if(!ExtraDataFromTrie(trie, key, filedata)) {
        return FALSE;
    }
    StringStream in(filedata);
    image = StreamRaster::LoadAny(in);
    if(image.IsEmpty()) {
        assert(false);            
        return FALSE;
    }
    /*
    if(key.Find("floatwnd_ad_bg.png") >= 0) {
        GdiplusLoadImageToString(filedata, image);
    }
    */
    return TRUE;
}

BOOL UppUIHelper::ExtraXmlFromTrie(marisa::SecTrie& trie, const Upp::String& key, Upp::String& xmlstr) {
    if(!ExtraDataFromTrie(trie, key, xmlstr)) {
        return FALSE;
    }    
    bool hasBOM = false;
    const unsigned char bom[] = {0xef, 0xbb, 0xbf, 0xbf, 0xbb, 0xef};
    if(xmlstr.GetLength() >= 3) {
        if(memcmp(~xmlstr, bom, 3) == 0) {
            hasBOM = true;
        } else if(memcmp(~xmlstr, bom + 3, 3) == 0) {
            hasBOM = true;
        }
    }
    if(hasBOM) {
        xmlstr.Remove(0, 3);
    }
    xmlstr.Cat(1, '\0');
    return TRUE;
}

BOOL UppUIHelper::SetCtrlParam(Upp::Ctrl* ctrl, String attrId, String attrVal) {
    Size sz(0 , 0);
    Point pt(0 , 0);
    if(attrId == "layid") {
        ctrl->LayoutId(attrVal);
    } else if(attrId == "layidc") {
        ctrl->LayoutId(attrVal);
    } else if(attrId == "tip") {
        ctrl->Tip(attrVal);
    } else if(attrId == "tabstop") {
        ctrl->WantFocus(UppUIHelper::ParseBool(attrVal));
    }else if(attrId == "HSizePos") {
        sz = UppUIHelper::ParseSize(attrVal);
        ctrl->HSizePos(sz.cx , sz.cy);
    }else if(attrId == "HSizePosZ") {
        sz = UppUIHelper::ParseSize(attrVal);
        ctrl->HSizePosZ(sz.cx , sz.cy);
    }else if(attrId == "VSizePos") {
        sz = UppUIHelper::ParseSize(attrVal);
        ctrl->VSizePos(sz.cx , sz.cy);
    }else if(attrId == "VSizePosZ") {
        sz = UppUIHelper::ParseSize(attrVal);
        ctrl->VSizePosZ(sz.cx , sz.cy);
    }else if(attrId == "HCenterPos") {
        pt = UppUIHelper::ParsePoint(attrVal);
        ctrl->HCenterPos(pt.x , pt.y);
    }else if(attrId == "HCenterPosZ") {
        pt = UppUIHelper::ParsePoint(attrVal);
        ctrl->HCenterPosZ(pt.x , pt.y);
    }else if(attrId == "VCenterPos") {
        pt = UppUIHelper::ParsePoint(attrVal);
        ctrl->VCenterPos(pt.x , pt.y);
    }else if(attrId == "VCenterPosZ") {
        pt = UppUIHelper::ParsePoint(attrVal);
        ctrl->VCenterPosZ(pt.x , pt.y);
    }else if(attrId == "LeftPos") {
        pt = UppUIHelper::ParsePoint(attrVal);
        ctrl->LeftPos(pt.x , pt.y);
    }else if(attrId == "LeftPosZ") {
        pt = UppUIHelper::ParsePoint(attrVal);
        ctrl->LeftPosZ(pt.x , pt.y);
    }else if(attrId == "RightPos") {
        pt = UppUIHelper::ParsePoint(attrVal);
        ctrl->RightPos(pt.x , pt.y);
    }else if(attrId == "RightPosZ") {
        pt = UppUIHelper::ParsePoint(attrVal);
        ctrl->RightPosZ(pt.x , pt.y);
    }else if(attrId == "TopPos") {
        pt = UppUIHelper::ParsePoint(attrVal);
        ctrl->TopPos(pt.x , pt.y);
    }else if(attrId == "TopPosZ") {
        pt = UppUIHelper::ParsePoint(attrVal);
        ctrl->TopPosZ(pt.x , pt.y);
    }else if(attrId == "BottomPos") {
        pt = UppUIHelper::ParsePoint(attrVal);
        ctrl->BottomPos(pt.x , pt.y);
    }else if(attrId == "BottomPosZ") {
        pt = UppUIHelper::ParsePoint(attrVal);
        ctrl->BottomPosZ(pt.x , pt.y);
    } else {
        return FALSE;
    }
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


BOOL UppUIHelper::LoadGdiplusImageFromBuffer(PBYTE buffer, DWORD bufferSize, 
                             std::tr1::shared_ptr<Gdiplus::Bitmap>& ret) {
    ret.reset();
    if(bufferSize == 0) {
        return FALSE;
    }
    do {
        CStreamOnByteArray stream(buffer, bufferSize);
        ret.reset(new Gdiplus::Bitmap((IStream*)&stream));
        if(ret.get() != NULL && ret->GetLastStatus() != Gdiplus::Ok ) {
	        ret.reset();
            break;
        }
    } while(false);
    return (ret.get() != NULL);
}


//-----------------------------------------------------------------------------------------------
CInitGDIPlus::CInitGDIPlus() :
        m_dwToken(0), m_nCImageObjects(0) {
	__try {
		InitializeCriticalSection(&m_sect);
        Init();
	} __except(EXCEPTION_EXECUTE_HANDLER) {
		assert(false);
	}
}

CInitGDIPlus::~CInitGDIPlus() {
	ReleaseGDIPlus();
	DeleteCriticalSection(&m_sect);
}

bool CInitGDIPlus::Init() {
	EnterCriticalSection(&m_sect);
	bool fRet = true;
	if( m_dwToken == 0 ) {
		Gdiplus::GdiplusStartupInput input;
		Gdiplus::GdiplusStartupOutput output;
		Gdiplus::Status status = Gdiplus::GdiplusStartup( &m_dwToken, &input, &output );
		if( status != Gdiplus::Ok )
			fRet = false;
	}
	LeaveCriticalSection(&m_sect);
	return fRet;
}

void CInitGDIPlus::ReleaseGDIPlus() {
	EnterCriticalSection(&m_sect);
	if( m_dwToken != 0 )
	{
		Gdiplus::GdiplusShutdown( m_dwToken );
	}
	m_dwToken = 0;
	LeaveCriticalSection(&m_sect);
}

END_UPPEX_NAMESPACE