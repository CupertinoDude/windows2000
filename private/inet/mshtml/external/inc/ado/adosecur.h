#ifndef _ADOSECURITY_H_
#define _ADOSECURITY_H_

interface IADOSecurity: public IUnknown
    {
    public:
		virtual HRESULT STDMETHODCALLTYPE SetURL(BSTR bstrURL) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetSafe(BOOL fSafe) = 0;
    };

// {782D16AE-905F-11d1-AC38-00C04FC29F8F}
DEFINE_GUID(IID_IADOSecurity, 0x782d16ae, 0x905f, 0x11d1, 0xac, 0x38, 0x0, 0xc0, 0x4f, 0xc2, 0x9f, 0x8f);

// {332c4425-26cb-11d0-b483-00c04fd90119}
DEFINE_GUID(IID_IHTMLDocument2, 0x332c4425, 0x26cb, 0x11d0, 0xb4, 0x83, 0x00, 0xc0, 0x4f, 0xd9, 0x01, 0x19);

// {79eac9ee-baf9-11ce-8c82-00aa004ba90b}
DEFINE_GUID(IID_IInternetSecurityManager, 0x79eac9ee, 0xbaf9, 0x11ce, 0x8c, 0x82, 0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b);

// {7b8a2d94-0ac9-11d1-896c-00c04Fb6bfc4}
DEFINE_GUID(CLSID_InternetSecurityManager, 0x7b8a2d94, 0x0ac9, 0x11d1, 0x89, 0x6c, 0x00, 0xc0, 0x4f, 0xb6, 0xbf, 0xc4);
#endif
