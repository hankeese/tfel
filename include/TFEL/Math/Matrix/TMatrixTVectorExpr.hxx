/*!
 * \file   include/TFEL/Math/Matrix/TMatrixTVectorExpr.hxx
 * \brief  
 * 
 * \author Helfer Thomas
 * \date   06 avr 2008
 * \copyright Copyright (C) 2006-2014 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#ifndef _LIB_TFEL_MATH_TMATRIXTVECTOREXPR_HXX_
#define _LIB_TFEL_MATH_TMATRIXTVECTOREXPR_HXX_ 

#include<cstddef>
#include<iterator>
#include<type_traits>

#include"TFEL/Config/TFELConfig.hxx"
#include"TFEL/Metaprogramming/StaticAssert.hxx"
#include"TFEL/Metaprogramming/Implements.hxx"
#include"TFEL/TypeTraits/IsTemporary.hxx"
#include"TFEL/FSAlgorithm/inner_product.hxx"
#include"TFEL/Math/General/ResultType.hxx"
#include"TFEL/Math/General/BasicOperations.hxx"
#include"TFEL/Math/General/RunTimeCheck.hxx"
#include"TFEL/Math/General/ObjectObjectRandomAccessConstIterator.hxx"
#include"TFEL/Math/Vector/VectorConcept.hxx"
#include"TFEL/Math/tvector.hxx"
#include"TFEL/Math/tmatrix.hxx"

namespace tfel{

  namespace math {
    
    template<unsigned short N,
	     unsigned short M,
	     typename A, typename B>
    class TMatrixTVectorExpr
    {
      TFEL_STATIC_ASSERT((tfel::meta::Implements<A,MatrixConcept>::cond));
      TFEL_STATIC_ASSERT((tfel::meta::Implements<B,VectorConcept>::cond));
      
      typedef typename ComputeBinaryResult<A,B,OpMult>::Result Result;
      typedef VectorTraits<Result> traits;
      typedef typename traits::IndexType IndexType;

      typedef typename MatrixTraits<A>::NumType NumTypeA;
      typedef typename VectorTraits<B>::NumType NumTypeB;
  
      static constexpr bool IsATemporary = tfel::typetraits::IsTemporary<A>::cond;
      static constexpr bool IsBTemporary = tfel::typetraits::IsTemporary<B>::cond;

      typename std::conditional<IsATemporary,const A,const A&>::type a;
      typename std::conditional<IsBTemporary,const B,const B&>::type b;

      TMatrixTVectorExpr();

      struct RowConstIterator
      {	
	typedef typename std::conditional<IsATemporary,
					const A,
					const A&>::type MType;

	typedef typename MatrixTraits<A>::NumType NumType;
	TFEL_MATH_INLINE RowConstIterator(const A& m_,
					  const unsigned short i_)
	  : m(m_),i(i_),j(0)
	{}
	TFEL_MATH_INLINE
	RowConstIterator& operator++ ()
	{
	  ++j;
	  return *this;
	} // end of operator++
	
	TFEL_MATH_INLINE
	NumType
	operator *(void) const
	{
	  using namespace std;
	  return m(i,j);
	}
      private:
	MType m;
	const unsigned short i;
	unsigned short j;
      }; // end of struc RowConstIterator

      struct VectorConstIterator
      {	
	typedef typename std::conditional<IsBTemporary,const B,const B&>::type VType;
	typedef typename VectorTraits<B>::NumType NumType;
	TFEL_MATH_INLINE VectorConstIterator(const B& v_)
	  : v(v_),i(0)
	{}
	TFEL_MATH_INLINE VectorConstIterator& operator++ ()
	{
	  ++i;
	  return *this;
	} // end of operator++
	TFEL_MATH_INLINE const NumType &
	operator *(void) const
	{
	  return v(i);
	}
      private:
	VType v;
	unsigned short i;
      }; // end of struc VectorConstIterator

    public:

      typedef typename ComputeBinaryResult<NumTypeA,NumTypeB,OpMult>::Handle NumType;
      typedef EmptyRunTimeProperties RunTimeProperties;

    protected:

      typedef const A first_arg;
      typedef const B second_arg;

      typedef NumType        value_type;                                                
      typedef NumType*       pointer;	    						
      typedef const NumType* const_pointer; 						
      typedef NumType&       reference;	    						
      typedef const NumType& const_reference;						
      typedef IndexType      size_type;	    						
      typedef ptrdiff_t      difference_type;                                          	

      TFEL_MATH_INLINE TMatrixTVectorExpr(const A& l, const B& r)
	: a(l), b(r)
      {}

      TFEL_MATH_INLINE
      TMatrixTVectorExpr(const TMatrixTVectorExpr<N,M,A,B>& src)
	: a(src.a), b(src.b)
      {}

      TFEL_MATH_INLINE
      NumType 
      operator()(const IndexType i) const 
      {
	using namespace tfel::fsalgo;
	return inner_product<M>::template exe<NumType>(RowConstIterator(a,i),
						       VectorConstIterator(b));
      }

    public:
      
      TFEL_MATH_INLINE
      RunTimeProperties
      getRunTimeProperties(void) const
      {
	return RunTimeProperties();
      }

    };

  } // end of namespace math

} // end of namespace tfel

#endif /* _LIB_TFEL_MATH_TMATRIXTVECTOREXPR_HXX */

