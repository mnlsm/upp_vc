#include "stdafx.h"
#include "SkinMgr.h"
#include "assert.h"

using namespace Upp;
using namespace std::tr1;
using namespace stdext;

BEGIN_NAMESPACE_UPPEX

SkinMgr::SkinMgr() {
}

SkinMgr::~SkinMgr() {
}

BOOL SkinMgr::LoadSkinFromMarisaFile(const char *mrsfile) {
    skinpath_ = Upp::GetFileFolder(mrsfile);
    FileIn in;
    if(!in.Open(mrsfile)) {
        return FALSE;
    }
    size_t filesize = (size_t)in.GetStreamSize();
    const size_t max_cache_filesize = 1024 * 1024 * 4; 
    if(filesize <= max_cache_filesize) {
        try {
            trie_filedata_ = in.Get(filesize);
            trie_.map((void*)~trie_filedata_, trie_filedata_.GetLength());
        } catch(marisa::Exception& e) {
            assert(false);
            e;
            return FALSE;
        } catch(...) {
            return FALSE;
        }
    } else {
        try {
            in.Close();
            trie_.mmap(mrsfile);
        } catch(marisa::Exception& e) {
            assert(false);
            e;
            return FALSE;
        } catch(...) {
            return FALSE;
        }
    }
    if(!UIHelper::LoadResFromMarisa(trie_, skinpath_, &resdata_)) {
        return FALSE;
    }
    return TRUE;
}

BOOL SkinMgr::LoadSkinFromXmlFile(const char *xmlfile , bool filebom) {
    skinpath_ = Upp::GetFileFolder(xmlfile);
    String xmldata = filebom ? LoadFileBOM(xmlfile) : LoadFile(xmlfile);
    if(!UIHelper::LoadResFromXml(~xmldata , skinpath_ , &resdata_)) {
        return FALSE;
    }
    return TRUE;
}

const char* SkinMgr::GetName() {
    return ~resdata_.resname_;
}

Upp::Color SkinMgr::ExtractColor(Upp::String id , Upp::byte alpha) {
    Color *clr = resdata_.colors_.FindPtr(id);
    if(clr != NULL) {
        if(alpha != 0xFF) {
            RGBA rgba = (RGBA) * clr;
            rgba.a = alpha;
            return Color(rgba);
        }
        return *clr;
    }
    assert(false);
    RGBA rgba;
    rgba.r = rgba.g = rgba.b = 0;
    rgba.a = alpha;
    return Color(rgba);;
}

Upp::Font SkinMgr::ExtractFont(Upp::String id) {
    if(resdata_.fonts_.Find(id) >= 0) {
        return resdata_.fonts_.Get(id);
    }
    assert(false);
    return GetStdFont();
}

Upp::Image SkinMgr::ExtractImage(Upp::String id) {
    Image *image = resdata_.images_.FindPtr(id);
    if(image != NULL) {
        return *image;
    }
    Image img;
    if(UIHelper::ExtraImageFromTrie(trie_, id, img)) {
        resdata_.images_.Add(id, img);
        image = resdata_.images_.FindPtr(id);
        if(image != NULL) {
            return *image;
        }
    }
    assert(false);
    RGBA rgba;
    rgba.r = rgba.g = rgba.b = 0;
    rgba.a = 0xFF;
    return UIHelper::CreateMonoImage(rgba, Size(1,1));
}

Upp::Image SkinMgr::ExtractSmallIcon(Upp::String id) {
    Image *image = resdata_.small_icons_.FindPtr(id);
    if(image != NULL) {
       return *image;
    }
    assert(false);
    RGBA rgba;
    rgba.r = rgba.g = rgba.b = 0;
    rgba.a = 0xFF;
    return UIHelper::CreateMonoImage(rgba, 
        Size(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CXSMICON)));
}

Upp::Image SkinMgr::ExtractLargeIcon(Upp::String id) {
    Image *image = resdata_.large_icons_.FindPtr(id);
    if(image != NULL) {
       return *image;
    }
    assert(false);
    RGBA rgba;
    rgba.r = rgba.g = rgba.b = 0;
    rgba.a = 0xFF;
    return UIHelper::CreateMonoImage(rgba, 
        Size(GetSystemMetrics(SM_CXICON),GetSystemMetrics(SM_CXICON)));
}


Upp::Image SkinMgr::ExtractCursor(Upp::String id) {
    Image *image = resdata_.cursors_.FindPtr(id);
    if(image != NULL) {
       return *image;
    }
    assert(false);
    RGBA rgba;
    rgba.r = rgba.g = rgba.b = 0;
    rgba.a = 0xFF;
    return UIHelper::CreateMonoImage(rgba, 
        Size(GetSystemMetrics(SM_CXCURSOR),GetSystemMetrics(SM_CXCURSOR)));
}

Upp::String SkinMgr::ExtractLayoutXml(Upp::String id) {
    String xmlstr;
    try {
        UIHelper::ExtraXmlFromTrie(trie_, id, xmlstr);
    } catch(marisa::Exception& e) {
        assert(false);
        e;
        return "";
    } catch(...) {
        assert(false);
        return "";
    }
    return xmlstr;
}


SkinMgr& SkinMgr::GetInstance() {
    static SkinMgr *p;
    ONCELOCK {
        static CInitGDIPlus s_initGDIPlus;
        static SkinMgr o;
        p = &o;
    }
    return *p;
}

void SkinMgr::Swap(SkinMgr& other) {
    if(this != &other) {
        Upp::Swap(skinpath_, other.skinpath_);
        Upp::Swap(trie_filedata_, other.trie_filedata_);
        resdata_.Swap(other.resdata_);
        trie_.swap(other.trie_);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void SearchForFiles( Vector<String>& files, String dir  ) {
    FindFile ff(AppendFileName(dir, "*.*"));
    while(ff) {
        if(ff.IsFolder() && *ff.GetName() != '.' && *ff.GetName() != '..') {
            files.Add( AppendFileName( dir, ff.GetName() ) );
            SearchForFiles(files, AppendFileName(dir, ff.GetName()));
        } else if(ff.IsFile()) {
            files.Add(AppendFileName(dir, ff.GetName()));
        }
        ff.Next();
    }
}

BOOL SkinMgr::CreateSkinToMarisaFile(const char* skinpath, const char* outfile) {
    Vector<String> files;
    SearchForFiles(files , skinpath);
    if(!files.IsEmpty()) {
        try {
            marisa::Keyset keyset;
            marisa::SecTrie trie;
            for(int i = 0 ; i < files.GetCount(); i++) {
                String file = files[i];
                if(DirectoryExists(file)) {
                    continue;
                }
                FileIn in;
                if(!in.Open(~file)) {
                    assert(false);
                    continue;
                }
                in.Seek(0);
                size_t filesize = (size_t)in.GetStreamSize();
                String filedata = in.Get(filesize);
                UIHelper::AdjustMarisaInnerFileData(filedata);
                String kv;
                kv.Cat(file);
                kv.Remove(0, strlen(skinpath) + 1);
                kv.Replace(DIR_SEPS, DIR_UNIX_SEPS);
                kv.Cat('\0', 1);
                kv.Cat(~filedata, filesize);
                keyset.push_back(~kv, kv.GetLength());
            }
            int build_flag = MARISA_MIN_NUM_TRIES | MARISA_BINARY_TAIL 
                    | MARISA_DEFAULT_ORDER | MARISA_TINY_CACHE;
            trie.build(keyset, build_flag);
            trie.save(outfile);
        } catch(marisa::Exception& e) {
            assert(false);
            e;
            return FALSE;
        } catch(...) {
            assert(false);
            return FALSE;
        }
    }
    return TRUE;
}

END_UPPEX_NAMESPACE