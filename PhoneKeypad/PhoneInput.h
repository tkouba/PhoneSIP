// PhoneInput.h : Declaration of the CPhoneInput

#pragma once
#ifdef STANDARDSHELL_UI_MODEL
#include "resource.h"
#endif
#ifdef POCKETPC2003_UI_MODEL
#include "resource.h"
#endif
#ifdef SMARTPHONE2003_UI_MODEL
#include "resource.h"
#endif
#ifdef AYGSHELL_UI_MODEL
#include "resource.h"
#endif

#include <sip.h>
#include <winnls.h>

#include "PhoneKeypad.h"

#define NO_KEY			-1          // no selected key

#define KEYS_LEN		12			// Max length of alternate key string

#define STATE_NUM		0			// Keyboard mode numbers
#define STATE_LWR		1			// Keyboard state lower cases
#define STATE_UPR		2			// Keyboard state upper cases
#define STATE_MAX		2			// Keyboard state max

#define MAX_KEY_LINE	100			// Max keys ...

// CPhoneInput

class ATL_NO_VTABLE CPhoneInput :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CPhoneInput, &CLSID_PhoneInput>,
	//public IDispatchImpl<IPhoneInput, &IID_IPhoneInput, &LIBID_PhoneKeypadLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IPhoneInput,
	public IInputMethod
{
public:
	CPhoneInput()
	{
	}

#ifndef _CE_DCOM
DECLARE_REGISTRY_RESOURCEID(IDR_PHONEINPUT)
#endif

DECLARE_NOT_AGGREGATABLE(CPhoneInput)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CPhoneInput)
	COM_INTERFACE_ENTRY(IPhoneInput)
	//COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IInputMethod)
END_COM_MAP()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:
	// IInputMethod
        virtual HRESULT STDMETHODCALLTYPE Select( /* [in] */ HWND hwndSip);

        virtual HRESULT STDMETHODCALLTYPE Deselect( void);

        virtual HRESULT STDMETHODCALLTYPE Showing( void);

        virtual HRESULT STDMETHODCALLTYPE Hiding( void);

        virtual HRESULT STDMETHODCALLTYPE GetInfo( /* [out] */ IMINFO *pInfo);

        virtual HRESULT STDMETHODCALLTYPE ReceiveSipInfo( /* [in] */ SIPINFO  *psi);

        virtual HRESULT STDMETHODCALLTYPE RegisterCallback( /* [in] */ IIMCallback *lpIMCallback);

        virtual HRESULT STDMETHODCALLTYPE GetImData( /* [in] */ DWORD dwSize, /* [out] */ void __RPC_FAR *pvImData)
		{
			//MessageBox(NULL,L"GetImData",L"",MB_OK);
			return E_NOTIMPL;
		};

        virtual HRESULT STDMETHODCALLTYPE SetImData( /* [in] */ DWORD dwSize, /* [in] */ void __RPC_FAR *pvImData)
		{
			//MessageBox(NULL,L"SetImData",L"",MB_OK);
			return E_NOTIMPL;
		};

        virtual HRESULT STDMETHODCALLTYPE UserOptionsDlg( /* [in] */ HWND hwndParent)
		{
			//MessageBox(NULL,L"UserOptionsDlg",L"",MB_OK);
			//TODO: Show about box
			// GetFileVersion example http://www.tech-archive.net/Archive/VC/microsoft.public.vc.language/2006-01/msg00151.html
			return E_NOTIMPL;
		};

};

OBJECT_ENTRY_AUTO(__uuidof(PhoneInput), CPhoneInput)
