#ifndef UPPSKINWND_H__
#define UPPSKINWND_H__

#include <CtrlCore/CtrlCore.h>
#include <CtrlLib/CtrlLib.h>
#include <draw/draw.h>

#include "UppUIHelper.h"


class CUppSkinWnd
        : public Upp::TopWindow {
public:
    CUppSkinWnd();
    virtual ~CUppSkinWnd();
    typedef Upp::TopWindow BaseWnd;

public:
    virtual void Paint(Upp::Draw& w);

public:
    virtual BOOL BuildFromXml(const char *xmlstr);
    virtual BOOL BuildFromXmlFile(const char *xmlfile , bool filebom);

    virtual Upp::Image CursorImage(Upp::Point p, Upp::dword keyflags);

protected:
    LRESULT OnCreate(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled);
    LRESULT OnNcPaint(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled);
    LRESULT OnNcCalcSize(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled);
    LRESULT OnNcHitTest(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled);
    LRESULT OnNcMouseEvent0(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled);
    LRESULT OnNcMouseEvent1(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled);
    LRESULT OnNCActivate(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled);
    LRESULT OnActivate(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled);
    LRESULT OnSetCursor(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled);
    LRESULT OnInitMenuPopup(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled);
    LRESULT OnGetMinMaxInfo(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled);
    LRESULT OnWindowPosChanged(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled);
    LRESULT OnSize(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled);

protected:
    virtual LRESULT OnCreateFinished(UINT message, WPARAM wParam, LPARAM lParam);


protected:
    LRESULT OnDelayRefreshWindow(UINT message, WPARAM wParam, LPARAM lParam , BOOL &bHandled);
    int delayrefreshnum_;

public:
    virtual LRESULT  WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    virtual void WndInvalidateRect(const Upp::Rect& r);
    virtual void WndScrollView(const Upp::Rect& r, int dx, int dy);

    virtual Upp::Ctrl* GetCtrlByLayoutId(const char *id);


protected:
    virtual void SyncCaption();
    virtual void OnWindowClose();
    virtual void OnWindowMaximize();
    virtual void OnWindowMinimize();

    virtual LRESULT HitTest(const POINT &pt);

protected:
    virtual void DrawBackground(Upp::Draw& w);
    virtual BOOL CreateChildCtrls(Upp::Ctrl *parent , const Upp::XmlNode &node);
    virtual BOOL SetCtrlParams(Upp::Ctrl* ctrl , const Upp::XmlNode &node);
    virtual void UpdateLayedWindowShow(BYTE byteConstantAlpha);
    virtual void ResetClippings(const Upp::XmlNode &node);

public:
    struct Style : Upp::ChStyle<Style> {
        enum { FIXED , HORZ3 , VERT3 , GRID };
        Upp::Image image_[9];
        int bkgnd_type_;
        Upp::Size GetImageSize();
    };
    const Style& GetStyle();
    void SetStyle(const Style& style);
    static const Style StyleNormal();

protected:
    UINT topborder_ ;
    UINT bottomborder_;
    UINT leftborder_;
    UINT rightborder_;

    Upp::Ptr<Upp::ParentCtrl> titlebar_;
    Upp::Ptr<Upp::Button> minboxbtn_;
    Upp::Ptr<Upp::Button> maxboxbtn_;
    Upp::Ptr<Upp::Button> restoreboxbtn_;
    Upp::Ptr<Upp::Button> closeboxbtn_;
    Upp::Ptr<Upp::Button> sizeboxbtn_;

    BOOL transmouseing_;
    Style skinstyle_;
    BOOL mouseclientmove_; //是否鼠标左键点击窗口客户区,移动窗口
    BOOL skinalpha_;
    Upp::byte constalpha_;
    BOOL calc_skin_rgn_;

    std::vector< ClipRegionData > vClippings_;
    std::map<std::string, Upp::Ptr<Upp::Ctrl> > vCacheCtrls_;

};


























#endif