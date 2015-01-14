/*! 
 * \file  include/TFEL/Math/QR/QRException.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 12 avr 2009
 * \copyright Copyright (C) 2006-2014 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#ifndef _LIB_TFEL_MATH_QREXCEPTION_H_
#define _LIB_TFEL_MATH_QREXCEPTION_H_ 

#include"TFEL/Config/TFELConfig.hxx"
#include"TFEL/Math/MathException.hxx"
 
namespace tfel
{

  namespace math
  {
    
    /*!
     * Base class for the exception thrown by
     * the QRDecomp class
     */
    struct TFELMATH_VISIBILITY_EXPORT QRException
      : public MathRunTimeException
    {
      //! destructor
      virtual ~QRException() noexcept;
    }; // end of struct QRException

    /*!
     * Base class for the exception thrown by
     * the QRDecomp class
     */
    struct TFELMATH_VISIBILITY_EXPORT QRUnmatchedSize
      : public QRException
    {
      //! \return a string describing the error
      virtual const char* what() const noexcept final;
      //! destructor
      virtual ~QRUnmatchedSize() noexcept;
    }; // end of struct QRException

    /*!
     * Base class for the exception thrown by
     * the QRDecomp class
     */
    struct TFELMATH_VISIBILITY_EXPORT QRInvalidMatrixSize
      : public QRException
    {
      //! \return a string describing the error
      virtual const char* what() const noexcept final;
      //! destructor
      virtual ~QRInvalidMatrixSize() noexcept;
    }; // end of struct QRException

    /*!
     * Base class for the exception thrown by
     * the QRDecomp class
     */
    struct TFELMATH_VISIBILITY_EXPORT QRNullPivot
      : public QRException
    {
      //! \return a string describing the error
      virtual const char* what() const noexcept final;
      //! destructor
      virtual ~QRNullPivot() noexcept;
    }; // end of struct QRException
    
  } // end of namespace math

} // end of namespace tfel

#endif /* _LIB_TFEL_MATH_QREXCEPTION_H */

