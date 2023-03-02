#ifndef UPPSKINMGR_H__
#define UPPSKINMGR_H__


#include "UIHelper.h"
#include "CtrlCreator.h"



BEGIN_NAMESPACE_UPPEX

class SkinMgr
        : Upp::NoCopy {
public:
    SkinMgr();
    ~SkinMgr();

public:
    static SkinMgr& GetInstance();
    void Swap(SkinMgr& other);

public:
    BOOL CreateSkinToMarisaFile(const char* skinpath, const char* outfile);
    BOOL LoadSkinFromMarisaFile(const char *mrsfile);

    BOOL LoadSkinFromXmlFile(const char *xmlfile , bool filebom);

public:
    const char* GetName();
    Upp::Color ExtractColor(Upp::String id , Upp::byte alpha = 0xFF);
    Upp::Image ExtractImage(Upp::String id);
    Upp::Font  ExtractFont(Upp::String id);
    Upp::Image ExtractSmallIcon(Upp::String id);
    Upp::Image ExtractLargeIcon(Upp::String id);
    Upp::Image ExtractCursor(Upp::String id);

    Upp::String ExtractLayoutXml(Upp::String id);

private:
    Upp::String skinpath_;
    UppResData resdata_;
    marisa::SecTrie trie_;
    Upp::String trie_filedata_;

};

END_UPPEX_NAMESPACE

#ifdef flagNONAMESPACE
#define theSkinMgr SkinMgr::GetInstance()
#else
#define theSkinMgr uppex::SkinMgr::GetInstance()
#endif

#endif