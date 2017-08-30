/*!
 * \file   mfront/include/MFront/FortranMaterialPropertyInterface.hxx
 * \brief  This file declares the FortranMaterialPropertyInterface class
 * \author Helfer Thomas
 * \date   1 décembre 2008
 * \copyright Copyright (C) 2006-2014 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#ifndef LIB_MFRONTFORTRANLAWINTERFACE_HXX
#define LIB_MFRONTFORTRANLAWINTERFACE_HXX 

#include"MFront/CMaterialPropertyInterfaceBase.hxx"

namespace mfront{

  struct FortranMaterialPropertyInterface
    : public CMaterialPropertyInterfaceBase
  {
    static std::string 
    getName();
    
    FortranMaterialPropertyInterface();
    /*!
     * \param[in] k  : keyword treated
     * \param[in] i:   list of interfaces to which the keyword is restricted
     * \param[in] p  : iterator to the current token
     * \param[in] pe : iterator past the end of the file
     * \return a pair. The first entry is true if the keyword was
     * treated by the interface. The second entry is an iterator after
     * the last token treated.
     */
    virtual std::pair<bool,tokens_iterator>
    treatKeyword(const std::string&,
		 const std::vector<std::string>&,
		 tokens_iterator,
		 const tokens_iterator) override;
    /*!
     * \brief : fill the target descripton
     * \param[out] d   : target description
     * \param[in]  mpd : material property description
     */
    virtual void getTargetsDescription(TargetsDescription&,
				       const MaterialPropertyDescription&) const override;
    /*!
     * \param[out] os:  output file stream
     * \param[in]  mpd: material property description
     */
    virtual void writeInterfaceSymbol(std::ostream&,
				      const MaterialPropertyDescription&) const override;
    //! destructor
    virtual ~FortranMaterialPropertyInterface();
        
  private:

    virtual void
    writeParameterList(std::ostream&,
		       const VariableDescriptionContainer&) const override;
    /*!
     * \param[out] os: output stream
     */
    virtual void
    writeInterfaceSpecificVariables(std::ostream&,
				    const VariableDescriptionContainer&) const override;
    /*!
     * \param[out] os: output stream
     * \param[in]  mpd: material property description
     */
    virtual void
    writeSrcPreprocessorDirectives(std::ostream&,
				    const MaterialPropertyDescription&) const override;
    /*!
     * \param[out] os: output stream
     */
    virtual void
    writeBeginHeaderNamespace(std::ostream&) const override;
    /*!
     * \param[out] os: output stream
     */
    virtual void
    writeEndHeaderNamespace(std::ostream&) const override;
    /*!
     * \param[out] os: output stream
     */
    virtual void
    writeBeginSrcNamespace(std::ostream&) const override;
    /*!
     * \param[out] os: output stream
     */
    virtual void
    writeEndSrcNamespace(std::ostream&) const override;

    /*!
     * \param const std::string&, name of the material
     * \param const std::string&, name of the class
     */
    virtual std::string
    getHeaderFileName(const std::string&,
		      const std::string&) const override;

    /*!
     * \param const std::string&, name of the material
     * \param const std::string&, name of the class
     */
    virtual std::string
    getSrcFileName(const std::string&,
		   const std::string&) const override;
    /*!
     * \param[in] mpd: material property description 
     */
    virtual std::string
    getFunctionName(const MaterialPropertyDescription&) const override;
    /*!
     * \param[in] mpd: material property description 
     */
    virtual std::string
    getCheckBoundsFunctionName(const MaterialPropertyDescription&) const override;

    virtual bool
    requiresCheckBoundsFunction() const override;

  }; // end of MfrontFortranMaterialPropertyInterface

} // end of namespace mfront

#endif /* LIB_MFRONTFORTRANLAWINTERFACE_HXX */

