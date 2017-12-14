
class XmlIO;

template <class T>
void XmlAttrLoad(T& var, const String& text) {
    var.XmlAttrLoad(text);
}

template <class T>
String XmlAttrStore(const T& var) {
    return var.XmlAttrStore();
}

class XmlIO {
    XmlNode& node;
    bool     loading;
    Value    userdata;

public:
    bool IsLoading() const {
        return loading;
    }
    bool IsStoring() const {
        return !loading;
    }

    XmlNode& Node() {
        return node;
    }
    const XmlNode& Node() const {
        return node;
    }

    XmlNode *operator->() {
        return &node;
    }

    String GetAttr(const char *id) {
        return node.Attr(id);
    }
    void   SetAttr(const char *id, const String& val) {
        node.SetAttr(id, val);
    }

    template <class T> XmlIO operator()(const char *tag, T& var);

    template <class T> XmlIO Attr(const char *id, T& var) {
        if(IsLoading())
            XmlAttrLoad(var, node.Attr(id));
        else
            node.SetAttr(id, XmlAttrStore(var));
        return *this;
    }

    template <class T> XmlIO Attr(const char *id, T& var, T def) {
        if(IsLoading())
            if(IsNull(node.Attr(id)))
                var = def;
            else
                XmlAttrLoad(var, node.Attr(id));
        else if(var != def)
            node.SetAttr(id, XmlAttrStore(var));
        return *this;
    }

    XmlIO At(int i) {
        XmlIO m(node.At(i), IsLoading(), userdata);
        return m;
    }
    XmlIO Add() {
        XmlIO m(node.Add(), IsLoading(), userdata);
        return m;
    }
    XmlIO Add(const char *id) {
        XmlIO m(node.Add(id), IsLoading(), userdata);
        return m;
    }
    XmlIO GetAdd(const char *id) {
        XmlIO m(node.GetAdd(id), IsLoading(), userdata);
        return m;
    }

    void  SetUserData(const Value& v) {
        userdata = v;
    }
    Value GetUserData() const {
        return userdata;
    }

    XmlIO(XmlNode& xml, bool loading, const Value& userdata) : node(xml), loading(loading), userdata(userdata) {}
    XmlIO(XmlNode& xml, bool loading) : node(xml), loading(loading) {}
    XmlIO(XmlIO xml, const char *tag) : node(xml.node.GetAdd(tag)), loading(xml.loading), userdata(xml.userdata) {}
};

template <class T>
void Xmlize(XmlIO xml, T& var) {
    var.Xmlize(xml);
}

template <class T> XmlIO XmlIO::operator()(const char *tag, T& var) {
    XmlIO n(*this, tag);
    Xmlize(n, var);
    return *this;
}

template<> inline void XmlAttrLoad(String& var, const String& text) {
    var = text;
}
template<> inline String XmlAttrStore(const String& var) {
    return var;
}

template<> void XmlAttrLoad(WString& var, const String& text);
template<> String XmlAttrStore(const WString& var);
template<> void XmlAttrLoad(int& var, const String& text);
template<> String XmlAttrStore(const int& var);
template<> void XmlAttrLoad(dword& var, const String& text);
template<> String XmlAttrStore(const dword& var);
template<> void XmlAttrLoad(double& var, const String& text);
template<> String XmlAttrStore(const double& var);
template<> void XmlAttrLoad(bool& var, const String& text);
template<> String XmlAttrStore(const bool& var);
template <> void XmlAttrLoad(int16& var, const String& text);
template <> String XmlAttrStore(const int16& var);
template <> void XmlAttrLoad(int64& var, const String& text);
template <> String XmlAttrStore(const int64& var);
template <> void XmlAttrLoad(byte& var, const String& text);
template <> String XmlAttrStore(const byte& var);
template <> void XmlAttrLoad(Date& var, const String& text);
template <> String XmlAttrStore(const Date& var);
template <> void XmlAttrLoad(Time& var, const String& text);
template <> String XmlAttrStore(const Time& var);

template<> void Xmlize(XmlIO xml, String& var);
template<> void Xmlize(XmlIO xml, WString& var);
template<> void Xmlize(XmlIO xml, int& var);
template<> void Xmlize(XmlIO xml, dword& var);
template<> void Xmlize(XmlIO xml, double& var);
template<> void Xmlize(XmlIO xml, bool& var);
template<> void Xmlize(XmlIO xml, Date& var);
template<> void Xmlize(XmlIO xml, Time& var);
template<> void Xmlize(XmlIO xml, int16& var);
template<> void Xmlize(XmlIO xml, int64& var);
template<> void Xmlize(XmlIO xml, byte& var);

template<> void Xmlize(XmlIO xml, Point& p);
template<> void Xmlize(XmlIO xml, Point16& p);
template<> void Xmlize(XmlIO xml, Point64& p);
template<> void Xmlize(XmlIO xml, Pointf& p);

template<> void Xmlize(XmlIO xml, Size& sz);
template<> void Xmlize(XmlIO xml, Size16& sz);
template<> void Xmlize(XmlIO xml, Size64& sz);
template<> void Xmlize(XmlIO xml, Sizef& sz);

template<> void Xmlize(XmlIO xml, Rect& r);
template<> void Xmlize(XmlIO xml, Rect16& r);
template<> void Xmlize(XmlIO xml, Rect64& r);
template<> void Xmlize(XmlIO xml, Rectf& r);

template<> void Xmlize(XmlIO xml, Color& c);

template<> void Xmlize(XmlIO xml, Value& v);

template<> void Xmlize(XmlIO xml, ValueArray& v);
template<> void Xmlize(XmlIO xml, ValueMap& v);

void XmlizeLangAttr(XmlIO xml, int& lang, const char *id = "lang");
void XmlizeLang(XmlIO xml, const char *tag, int& lang, const char *id = "id");

template<class T>
void XmlizeContainer(XmlIO xml, const char *tag, T& data) {
    if(xml.IsStoring())
        for(int i = 0; i < data.GetCount(); i++)
            Xmlize(xml.Add(tag), data[i]);
    else {
        data.Clear();
        for(int i = 0; i < xml->GetCount(); i++)
            if(xml->Node(i).IsTag(tag))
                Xmlize(xml.At(i), data.Add());
    }
}

template<class T>
void Xmlize(XmlIO xml, Vector<T>& data) {
    XmlizeContainer(xml, "item", data);
}

template<class T>
void Xmlize(XmlIO xml, Array<T>& data) {
    XmlizeContainer(xml, "item", data);
}

template<class T>
void XmlizeStore(XmlIO xml, const T& data) {
    ASSERT(xml.IsStoring());
    Xmlize(xml, const_cast<T&>(data));
}

template<class K, class V, class T>
void XmlizeMap(XmlIO xml, const char *keytag, const char *valuetag, T& data) {
    if(xml.IsStoring()) {
        for(int i = 0; i < data.GetCount(); i++)
            if(!data.IsUnlinked(i)) {
                XmlizeStore(xml.Add(keytag), data.GetKey(i));
                XmlizeStore(xml.Add(valuetag), data[i]);
            }
    }else {
        data.Clear();
        int i = 0;
        while(i < xml->GetCount() - 1 && xml->Node(i).IsTag(keytag) && xml->Node(i + 1).IsTag(valuetag)) {
            K key;
            Xmlize(xml.At(i++), key);
            Xmlize(xml.At(i++), data.Add(key));
        }
    }
}

template<class K, class V, class H>
void Xmlize(XmlIO xml, VectorMap<K, V, H>& data) {
    XmlizeMap<K, V>(xml, "key", "value", data);
}

template<class K, class V, class H>
void Xmlize(XmlIO xml, ArrayMap<K, V, H>& data) {
    XmlizeMap<K, V>(xml, "key", "value", data);
}

template<class K, class T>
void XmlizeIndex(XmlIO xml, const char *keytag, T& data) {
    if(xml.IsStoring()) {
        for(int i = 0; i < data.GetCount(); i++)
            if(!data.IsUnlinked(i)) {
                //XmlizeStore(xml.Add(keytag), data.GetKey(i)); //FIXME xmlize with hashfn awareness
                XmlizeStore(xml.Add(keytag), data[i]);
            }
    }else {
        data.Clear();
        int i = 0;
        //while(i < xml->GetCount() - 1 && xml->Node(i).IsTag(keytag) && xml->Node(i + 1).IsTag(valuetag)) {
        while(i < xml->GetCount() && xml->Node(i).IsTag(keytag)) {
            //K key;
            //Xmlize(xml.At(i++), key); //FIXME dexmlize with hashfn awareness
            K k;
            Xmlize(xml.At(i++), k);
            data.Add(k);
        }
    }
}

template<class K, class H>
void Xmlize(XmlIO xml, Index<K, H>& data) {
    XmlizeIndex<K>(xml, "key", data);
}

template<class K, class H>
void Xmlize(XmlIO xml, ArrayIndex<K, H>& data) {
    XmlizeIndex<K>(xml, "key", data);
}

void RegisterValueXmlize(dword type, void (*xmlize)(XmlIO xml, Value& v), const char *name);

template <class T>
void ValueXmlize(XmlIO xml, Value& v) {
    T x;
    if(xml.IsStoring())
        x = v;
    Xmlize(xml, x);
    if(xml.IsLoading())
        v = x;
}

#define REGISTER_VALUE_XMLIZE(T) \
    INITBLOCK { RegisterValueXmlize(GetValueTypeNo<T>(), &ValueXmlize<T>, #T); }

template <class T>
struct ParamHelper__ {
    T&   data;
    void Invoke(XmlIO xml) {
        Xmlize(xml, data);
    }

    ParamHelper__(T& data) : data(data) {}
};

String StoreAsXML(Callback1<XmlIO> xmlize, const char *name);
bool   LoadFromXML(Callback1<XmlIO> xmlize, const String& xml);

template <class T>
String StoreAsXML(T& data, const char *name) {
    ParamHelper__<T> p(data);
    return StoreAsXML(callback(&p, &ParamHelper__<T>::Invoke), name);
}

template <class T>
bool LoadFromXML(T& data, const String& xml) {
    ParamHelper__<T> p(data);
    return LoadFromXML(callback(&p, &ParamHelper__<T>::Invoke), xml);
}

bool StoreAsXMLFile(Callback1<XmlIO> xmlize, const char *name = NULL, const char *file = NULL);
bool LoadFromXMLFile(Callback1<XmlIO> xmlize, const char *file = NULL);

template <class T>
bool StoreAsXMLFile(T& data, const char *name = NULL, const char *file = NULL) {
    ParamHelper__<T> p(data);
    return StoreAsXMLFile(callback(&p, &ParamHelper__<T>::Invoke), name, file);
}

template <class T>
bool LoadFromXMLFile(T& data, const char *file = NULL) {
    ParamHelper__<T> p(data);
    return LoadFromXMLFile(callback(&p, &ParamHelper__<T>::Invoke), file);
}
