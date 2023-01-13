//
// IRefPtr.h
// Copyright 1992 - 2006 Dennis Robinson (www.menasoft.com)
// Template for a type specific Ref

#ifndef _INC_IRefPtr_H
#define _INC_IRefPtr_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#include <unknwn.h>
#include <assert.h>

#define IREF_GETPPTR(p,_TYPE)	(p).get_PPtr()
#define IREF_GETPPTRV(p,_TYPE)	(p).get_PPtrV()

template<class _TYPE = IUnknown>
class IRefPtrOpen;

template<class _TYPE = IUnknown> 
class IRefPtr
{
	// Smart pointer to an IUnknown based object. 
	// like ATL CComPtr<> or "com_ptr_t"
	// _TYPE must be based on IUnknown

	friend class IRefPtrOpen<_TYPE>;
	friend class IRefPtrOpen<IUnknown>;

protected:

#ifdef _DEBUG
	void AssertIREF( _TYPE* pObj ) const
	{ 
		if ( pObj == NULL )
			return;
		//assert( PTR_CAST(_TYPE,pObj));
		//assert( PTR_CAST(IUnknown,pObj)); // ambigious in the case of multiple non virtual IUnknown bases !!! and fails !
	}
#else
	void AssertIREF( _TYPE* pObj ) const { pObj;}
#endif

	void SetFirstRefObj( _TYPE* pObj )
	{
		// NOTE: IncRefCount can throw !
		if ( pObj )
		{
			pObj->AddRef();
			AssertIREF(pObj);
		}
		m_pObj = pObj;
	}
public:
	bool IsValidRefObj() const
	{
		AssertIREF(m_pObj);
		return m_pObj != NULL;
	}
	_TYPE** get_PPtr()
	{
		// QueryInterface or similiar wants a pointer to a pointer to fill in my interface.
		return( &m_pObj );
	}
	void** get_PPtrV()
	{
		// QueryInterface and others dont like the typing.
		return((void**) &m_pObj );
	}
	_TYPE* get_RefObj() const
	{
		return( m_pObj );
	}
	void put_RefObj( _TYPE* pObj )
	{
		ReleaseRefObj();
		SetFirstRefObj(pObj);
	}
	int ReleaseRefObj()
	{
		if ( m_pObj )
		{
			_TYPE* pObj = m_pObj;
			AssertIREF(m_pObj);
			m_pObj = NULL;	// make sure possible destructors called in DecRefCount don't reuse this.
			return pObj->Release();	// this might delete this ?
		}
		return 0;
	}

	_TYPE** ReleaseAndGetAddressOf()
	{
		ReleaseRefObj();
		return &m_pObj;
	}


	// Assignment ops.
	IRefPtr<_TYPE>& operator = ( _TYPE* pRef )
	{
		if ( pRef != m_pObj )
		{
			put_RefObj( pRef );
		}
		return *this;
	}
    IRefPtr<_TYPE>& operator = ( IRefPtr<_TYPE>& ref )
    {
		return operator=((_TYPE*) ref);
	}

	// Accessor ops.
	operator _TYPE*() const 
	{ 
		return( m_pObj ); 
	}

    _TYPE& operator * () const 
	{  
		return *m_pObj; 
	}

	_TYPE* operator -> () const 
	{ 
		return( m_pObj ); 
	}

	_TYPE ** operator &( )
	{
		return &m_pObj ; 
	}

	// Comparison ops
	bool operator!() const
	{
		return( m_pObj == NULL );
	}
	bool operator != ( /*const*/ _TYPE* pRef ) const
	{
		return( pRef != m_pObj );
	}
	bool operator == ( /*const*/ _TYPE* pRef ) const
	{
		return( pRef == m_pObj );
	}

	// Construct and destruct
	IRefPtr()
		: m_pObj(NULL)
	{
	}
	IRefPtr( _TYPE* pObj )
	{
		SetFirstRefObj(pObj);
	}

	IRefPtr( const IRefPtr<_TYPE>& ref )
	{
		// using the assingment auto constructor is not working so use this.
		SetFirstRefObj( ref.get_RefObj());
	}

	~IRefPtr()
	{
		ReleaseRefObj();
	}

private:
	_TYPE* m_pObj;	// object we are referring to MUST be based on IUnknown
};

#endif
