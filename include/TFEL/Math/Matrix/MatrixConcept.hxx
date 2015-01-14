/*!
 * \file   include/TFEL/Math/Matrix/MatrixConcept.hxx
 * \brief  This file declares the MatrixTraits, MatrixConcept and MatrixType classes.
 * \author Helfer Thomas
 * \date   04 May 2006
 * \copyright Copyright (C) 2006-2014 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#ifndef _TFEL_MATH_MATRIX_CONCEPT_LIB_
#define _TFEL_MATH_MATRIX_CONCEPT_LIB_ 1

#include<type_traits>

#include"TFEL/Config/TFELConfig.hxx"
#include"TFEL/Metaprogramming/InvalidType.hxx"
#include"TFEL/TypeTraits/IsTemporary.hxx"
#include"TFEL/Math/General/ResultType.hxx"
#include"TFEL/Math/General/BasicOperations.hxx"
#include"TFEL/Math/Forward/MatrixConcept.hxx"

namespace tfel{

  namespace math{

    /*!
     * \class MatrixTraits
     * \brief A traits class for matrices.
     * \param T, a matrix type.
     * This traits gives acces to the two main characteristics, 
     * its underlying numerical type and its index type.
     * \warning this class shall be overloaded for each matrix type. 
     * This is du to the fact that we utilise CRTP to define the 
     * MatrixConcept class, so the characteristic of the matrix type
     * must be known \b before this type is defined.
     * \author Helfer Thomas
     * \date   04 May 2006
     */
    template<typename T>
    struct MatrixTraits{
      /*!
       * Gives acces to the numerical types upon which the matrix is build.
       * \see InvalidType
       */
      typedef tfel::meta::InvalidType NumType;
      /*!
       * Gives access to the index type used by the matrix.
       * \see InvalidType
       */
      typedef tfel::meta::InvalidType IndexType;
    };

    /*!
     * \class MatrixTag
     * \brief Helper class to characterise matrices.
     */ 
    struct MatrixTag
    {}; // end of MatrixTag

    /*!
     * \class MatrixConcept
     * \brief A class used to model the concept of matrices.
     * Here we use the curiously recurring template pattern.  
     * \param T, a matrix type.
     * \author Helfer Thomas
     * \date   04 May 2006
     */
    template<typename T>
    struct MatrixConcept 
    {
    private:
      /*!
       * A simple typedef.
       */
      typedef MatrixTraits<T> traits;
      static constexpr bool isTemporary = tfel::typetraits::IsTemporary<T>::cond;
      typedef typename std::conditional<isTemporary,
				      typename traits::NumType,
				      const typename traits::NumType&>::type ValueType;

    protected:
      MatrixConcept() = default;
      MatrixConcept(MatrixConcept&&) = default;
      MatrixConcept(const MatrixConcept&) = default;
      MatrixConcept&
      operator=(const MatrixConcept&) = default;
      ~MatrixConcept() = default;
    public :

      typedef MatrixTag ConceptTag;

      /*!
       * \param  traits::IndexType the line index
       * \param  traits::IndexType the column index
       * \return traits::NumType
       */
      ValueType
      operator()(const typename traits::IndexType,
		 const typename traits::IndexType) const ;

    };

    template<typename T>
    struct MatrixType
    {
      typedef T type;
    };
  
  } // end of namespace math

} // end of namespace tfel

#include"TFEL/Math/Matrix/MatrixConcept.ixx"

#endif /* _TFEL_MATH_MATRIX_CONCEPT_LIB_ */
