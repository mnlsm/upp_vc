#ifndef UPPSKINMGR_H__
#define UPPSKINMGR_H__


#include "UppUIHelper.h"
#include "UppCtrlCreator.h"



NAMESPACE_UPPEX

class CUppSkinMgr
        : Upp::NoCopy {
public:
    CUppSkinMgr();
    ~CUppSkinMgr();

public:
    static CUppSkinMgr& GetInstance();
    void Swap(CUppSkinMgr& other);

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
#define theSkinMgr CUppSkinMgr::GetInstance()
#else
#define theSkinMgr Uppex::CUppSkinMgr::GetInstance()
#endif

#endif