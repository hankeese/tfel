/*!
 * \file   tests/Math/include/promote-header.hxx
 * \brief  
 * 
 * \author Helfer Thomas
 * \date   05 f�v 2008
 * \copyright Copyright (C) 2006-2014 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#ifndef _LIB_TFEL_PROMOTE_HEADER_HXX_
#define _LIB_TFEL_PROMOTE_HEADER_HXX_ 

#include<iostream>
#include<cstdlib>
#include<cassert>
#include<limits>
#include<string>

#include"TFEL/TypeTraits/Promote.hxx"
#include"TFEL/Math/General/Complex.hxx"

std::string Test(const float&);

std::string Test(const long double&);

std::string Test(const double&);

std::string Test(const int&);

std::string Test(const long int&);

std::string Test(const long unsigned int&);

std::string Test(const unsigned int&);

std::string Test(const short&);

std::string Test(const unsigned short&);

std::string Test(const tfel::math::Complex<unsigned short>&);

std::string Test(const tfel::math::Complex<short>&);

std::string Test(const tfel::math::Complex<unsigned int>&);

std::string Test(const tfel::math::Complex<int>&);

std::string Test(const tfel::math::Complex<float>&);

std::string Test(const tfel::math::Complex<double>&);

std::string Test(const tfel::math::Complex<long double>&);
  
#endif /* _LIB_TFEL_PROMOTE_HEADER_HXX */
