#include "CtrlCore.h"

NAMESPACE_UPP

#define LLOG(x)   // DLOG(x)

TimeCallback::~TimeCallback() {
    Kill();
}

void TimeCallback::Set(int delay, Callback cb) {
    UPP::SetTimeCallback(delay, cb, this);
}

void TimeCallback::Post(Callback cb) {
    UPP::PostCallback(cb, this);
}

void TimeCallback::Kill() {
    UPP::KillTimeCallback(this);
}

void TimeCallback::KillSet(int delay, Callback cb) {
    Kill();
    Set(delay, cb);
}

void TimeCallback::KillPost(Callback cb) {
    Kill();
    Post(cb);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Ctrl::Logc::LSGN(dword d) {
    return int16(d & 0x7fff | ((d & 0x4000) << 1));
}

bool Ctrl::Logc::operator==(Logc q) const {
    return data == q.data;
}

bool Ctrl::Logc::operator!=(Logc q) const {
    return data != q.data;
}

int Ctrl::Logc::GetAlign() const {
    return (data >> 30) & 3;
}

int Ctrl::Logc::GetA() const {
    return LSGN(data >> 15);
}

int Ctrl::Logc::GetB() const {
    return LSGN(data);
}

void  Ctrl::Logc::SetAlign(int align) {
    data = (data & ~(3 << 30)) | (align << 30);
}

void  Ctrl::Logc::SetA(int a) {
    data = (data & ~(0x7fff << 15)) | ((a & 0x7fff) << 15);
}

void  Ctrl::Logc::SetB(int b) {
    data = (data & ~0x7fff) | (b & 0x7fff);
}

Ctrl::Logc::Logc(int al, int a, int b) {
    data = (al << 30) | ((a & 0x7fff) << 15) | (b & 0x7fff);
}

Ctrl::Logc::Logc() {
    data = 0xffffffff;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Ctrl::LogPos::LogPos(Logc x, Logc y)
    : x(x),
      y(y) {

}

Ctrl::LogPos::LogPos() {

}

bool Ctrl::LogPos::operator==(LogPos b) const {
    return x == b.x && y == b.y;
}
bool Ctrl::LogPos::operator!=(LogPos b) const {
    return !(*this == b);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Ctrl::Logc Ctrl::PosLeft(int pos, int size) {
    return Logc(LEFT, pos, size);
}

Ctrl::Logc Ctrl::PosRight(int pos, int size) {
    return Logc(RIGHT, pos, size);
}

Ctrl::Logc Ctrl::PosTop(int pos, int size) {
    return Logc(TOP, pos, size);
}

Ctrl::Logc Ctrl::PosBottom(int pos, int size) {
    return Logc(BOTTOM, pos, size);
}

Ctrl::Logc Ctrl::PosSize(int lpos, int rpos) {
    return Logc(SIZE, lpos, rpos);
}

Ctrl::Logc Ctrl::PosCenter(int size, int offset) {
    return Logc(CENTER, offset, size);
}

Ctrl::Logc Ctrl::PosCenter(int size) {
    return Logc(CENTER, 0, size);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef flagSO
Ptr<Ctrl> Ctrl::FocusCtrl() {
    return focusCtrl;
}
void Ctrl::FocusCtrl(Ptr<Ctrl> fc) {
    focusCtrl = fc;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Ctrl::IsEndSession() {
    return EndSession();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HWND Ctrl::GetHWND() const {
    return parent_ ? NULL : top_ ? top_->hwnd : NULL;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Ctrl& Ctrl::Unicode() {
    unicode = true;
    return *this;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Ctrl* Ctrl::GetParent() const {
    return parent_;
}

Ctrl* Ctrl::GetLastChild() const {
    return lastchild;
}

Ctrl* Ctrl::GetFirstChild() const {
    return firstchild;
}

Ctrl* Ctrl::GetPrev() const {
    return parent_ ? prev : NULL;
}

Ctrl* Ctrl::GetNext() const {
    return parent_ ? next : NULL;
}

bool Ctrl::IsChild() const {
    return parent_;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Ctrl& Ctrl::SetFrame(CtrlFrame& frm) {
    return SetFrame(0, frm);
}
const CtrlFrame& Ctrl::GetFrame(int i) const {
    return *frame[i].frame;
}
CtrlFrame& Ctrl::GetFrame(int i) {
    return *frame[i].frame;
}

int Ctrl::GetFrameCount() const {
    return frame.GetCount();
}

void Ctrl::Shutdown() {
    destroying = true;
}

bool Ctrl::IsShutdown() const {
    return destroying;
}

Ctrl& Ctrl::SetPos(Logc x, Logc y) {
    return SetPos(LogPos(x, y));
}

Ctrl& Ctrl::SetFramePos(Logc x, Logc y) {
    return SetFramePos(LogPos(x, y));
}

bool Ctrl::InFrame() const {
    return inframe_;
}

bool Ctrl::InView() const {
    return !inframe_;
}

Ctrl::LogPos Ctrl::GetPos() const {
    return pos;
}

void Ctrl::RefreshLayout() {
    SyncLayout(1);
}

void Ctrl::RefreshLayoutDeep() {
    SyncLayout(2);
}

void Ctrl::RefreshParentLayout() {
    if(parent_)
        parent_->RefreshLayout();
}

void Ctrl::UpdateLayout() {
    SyncLayout();
}

void Ctrl::UpdateParentLayout() {
    if(parent_)
        parent_->UpdateLayout();
}

Size Ctrl::AddFrameSize(Size sz) const {
    return AddFrameSize(sz.cx, sz.cy);
}

bool Ctrl::IsFullRefresh() const {
    return fullrefresh;
}

bool Ctrl::IsPainting() {
    return painting;
}

void Ctrl::ScrollView(const Rect& r, Size delta) {
    ScrollView(r, delta.cx, delta.cy);
}

void Ctrl::ScrollView(Size delta) {
    ScrollView(delta.cx, delta.cy);
}

Ctrl& Ctrl::IgnoreMouse(bool b) {
    ignoremouse = b;
    return *this;
}

Ctrl& Ctrl::NoIgnoreMouse() {
    return IgnoreMouse(false);
}

bool Ctrl::HasFocus() const {
    return FocusCtrl() == this;
}

Ctrl& Ctrl::WantFocus(bool ft) {
    wantfocus = ft;
    return *this;
}

Ctrl& Ctrl::NoWantFocus() {
    return WantFocus(false);
}

bool Ctrl::IsWantFocus() const {
    return wantfocus;
}

Ctrl& Ctrl::InitFocus(bool ft) {
    initfocus = ft;
    return *this;
}

Ctrl& Ctrl::NoInitFocus() {
    return InitFocus(false);
}

bool Ctrl::IsInitFocus() const {
    return initfocus;
}

Ctrl* Ctrl::GetFocusChild() const {
    return HasChild(FocusCtrl()) ? ~FocusCtrl() : NULL;
}

Ctrl* Ctrl::GetFocusChildDeep() const {
    return HasChildDeep(FocusCtrl()) ? ~FocusCtrl() : NULL;
}

Ctrl* Ctrl::GetFocusCtrl() {
    return FocusCtrl();
}

Ctrl* Ctrl::GetEventTopCtrl() {
    return eventCtrl;
}

void Ctrl::Hide() {
    Show(false);
}

bool Ctrl::IsShown() const {
    return visible;
}

void Ctrl::Disable() {
    Enable(false);
}

bool Ctrl::IsEnabled() const {
    return enabled;
}

Ctrl& Ctrl::SetReadOnly() {
    return SetEditable(false);
}

bool Ctrl::IsEditable() const {
    return editable;
}

bool Ctrl::IsReadOnly() const {
    return !editable;
}

void Ctrl::ResetModify() {
    modify = false;
}

bool Ctrl::IsModifySet() const {
    return modify;
}

Ctrl& Ctrl::BackPaint(int bp) {
    backpaint = bp;
    return *this;
}

Ctrl& Ctrl::TransparentBackPaint() {
    backpaint = TRANSPARENTBACKPAINT;
    return *this;
}

Ctrl& Ctrl::NoBackPaint() {
    return BackPaint(NOBACKPAINT);
}

int Ctrl::GetBackPaint() const {
    return backpaint;
}

Ctrl& Ctrl::Transparent(bool bp) {
    transparent = bp;
    return *this;
}

Ctrl& Ctrl::NoTransparent() {
    return Transparent(false);
}

bool Ctrl::IsTransparent() const {
    return transparent;
}

Ctrl& Ctrl::ActiveX(bool ax) {
    activex = ax;
    return *this;
}

Ctrl& Ctrl::NoActiveX() {
    return ActiveX(false);
}

bool Ctrl::IsActiveX() const {
    return activex;
}

Ctrl& Ctrl::Info(const char *txt) {
    info = txt;
    return *this;
}

String Ctrl::GetInfo() const {
    return info;
}

void Ctrl::Add(Ctrl& ctrl) {
    AddChild(&ctrl);
}

Ctrl& Ctrl::operator<<(Ctrl& ctrl) {
    Add(ctrl);
    return *this;
}

Value Ctrl::operator~() const {
    return GetData();
}

const Value& Ctrl::operator<<=(const Value& v) {
    SetData(v);
    return v;
}

bool Ctrl::IsNullInstance() const {
    return GetData().IsNull();
}

Callback Ctrl::operator<<=(Callback action) {
    WhenAction = action;
    return action;
}

Callback& Ctrl::operator<<(Callback action) {
    return WhenAction << action;
}

bool Ctrl::IsPopUp() const {
    return popup;
}

int Ctrl::GetLoopLevel() {
    return LoopLevel;
}

Ctrl* Ctrl::GetLoopCtrl() {
    return LoopCtrl;
}

bool Ctrl::IsDragAndDropSource() {
    return this == GetDragAndDropSource();
}

bool Ctrl::IsDragAndDropTarget() {
    return this == GetDragAndDropTarget();
}

Size Ctrl::StdSampleSize() {
    return Size(126, 106);
}

void Ctrl::SetMinSize(Size sz) {

}

int Ctrl::HZoom(int cx) {
    return HorzLayoutZoom(cx);
}

void Ctrl::GuiFlush() {
    ::GdiFlush();
}

int64 Ctrl::GetEventId() {
    return eventid;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LocalLoop::SetMaster(Ctrl& m) {
    master = &m;
}

Ctrl& LocalLoop::GetMaster() const {
    return *master;
}

LocalLoop::LocalLoop() {
    master = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Rect RectTracker::Round(const Rect& r) {
    return rounder ? rounder->Round(r) : r;
}

RectTracker& RectTracker::SetCursorImage(const Image& m) {
    cursorimage = m;
    return *this;
}

RectTracker& RectTracker::MinSize(Size sz) {
    minsize = sz;
    return *this;
}

RectTracker& RectTracker::MaxSize(Size sz) {
    maxsize = sz;
    return *this;
}

RectTracker& RectTracker::MaxRect(const Rect& mr) {
    maxrect = mr;
    return *this;
}

RectTracker& RectTracker::Clip(const Rect& c) {
    clip = c;
    return *this;
}

RectTracker& RectTracker::Width(int n) {
    width = n;
    return *this;
}

RectTracker& RectTracker::SetColor(Color c) {
    color = c;
    return *this;
}

RectTracker& RectTracker::Pattern(uint64 p) {
    pattern = p;
    return *this;
}

RectTracker& RectTracker::Animation(int step_ms) {
    animation = step_ms;
    return *this;
}

RectTracker& RectTracker::KeepRatio(bool b) {
    keepratio = b;
    return *this;
}

RectTracker& RectTracker::Round(Rounder& r) {
    rounder = &r;
    return *this;
}

Rect RectTracker::Get() {
    return rect;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AutoWaitCursor::Cancel() {
    time0 = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////







END_UPP_NAMESPACE