/*! 
 * \file  mfront/src/ModelDSLCommon.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 11 jun 2010
 * \copyright Copyright (C) 2006-2014 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#include<sstream>
#include<stdexcept>
#include<algorithm>

#include"TFEL/Glossary/Glossary.hxx"
#include"TFEL/Glossary/GlossaryEntry.hxx"
#include"TFEL/Utilities/StringAlgorithms.hxx"
#include"MFront/DSLUtilities.hxx"
#include"MFront/MFrontLogStream.hxx"
#include"MFront/MFrontDebugMode.hxx"
#include"MFront/ModelDSLCommon.hxx"
#include"MFront/TargetsDescription.hxx"
#include"MFront/ModelInterfaceFactory.hxx"

// fixing a bug on current glibc++ cygwin versions (19/08/2015)
#if defined __CYGWIN__ &&  (!defined _GLIBCXX_USE_C99)
#include<sstream>
namespace std{
  template<typename T>
  std::string to_string(const T& v){
    std::ostringstream s;
    s << v;
    return s.str();
  }
}
#endif /* defined __CYGWIN__ &&  (!defined _GLIBCXX_USE_C99) */

namespace mfront{

  static bool is(const VariableDescriptionContainer& vc,
		 const std::string& vn)
  {
    for(const auto& v : vc){
      if(v.name==vn){
	return true;
      }
      if(v.hasAttribute(VariableDescription::depth)){
	const auto d = v.getAttribute<unsigned short>(VariableDescription::depth);
	for(unsigned short i=1;i!=d+1;++i){
	  if(v.name+'_'+std::to_string(i)==vn){
	    return true;
	  }
	}
      }
    }
    return false;
  } // end of ModelDSLCommon::is()
  
  ModelDSLCommon::ModelDSLCommon(){
    for(const auto& v : DSLBase::getDefaultReservedNames()){
      this->reserveName(v);
    }
  }

  void ModelDSLCommon::reserveName(const std::string& n){
    this->md.reserveName(n);
  }

  bool ModelDSLCommon::isNameReserved(const std::string& n) const{
    return this->md.isNameReserved(n);
  }
  
  AbstractDSL::DSLTarget ModelDSLCommon::getTargetType() const{
    return MODELDSL;
  }

  std::string ModelDSLCommon::getClassName() const
  {
    return this->md.className;
  } // end of ModelDSLCommon::getClassName

  void ModelDSLCommon::addStaticVariableDescription(const StaticVariableDescription& v)
  {
    this->md.staticVars.push_back(v);
  } // end of ModelDSLCommon::addStaticVariableDescription

  int ModelDSLCommon::getIntegerConstant(const std::string& n) const{
    for(const auto& v:this->md.staticVars){
      if(v.name==n){
	if(v.type!="int"){
	  this->throwRuntimeError("ModelDSLCommon::getIntegerConstant",
				  "invalid type for variable '"+n+"'");
	}
	return v.value;
      }
    }
    this->throwRuntimeError("ModelDSLCommon::getIntegerConstant",
			    "unknown variable '"+n+"'");
  } // end of ModelDSLCommon::getIntegerConstant
  
  void ModelDSLCommon::treatMaterial()
  {
    if(!this->md.material.empty()){
      this->throwRuntimeError("ModelDSLCommon::treatMaterial",
			      "material name alreay defined");
    }
    this->md.material = this->readOnlyOneToken();
    if(!CxxTokenizer::isValidIdentifier(this->md.material,true)){
      this->throwRuntimeError("ModelDSLCommon::treatMaterial",
			      "invalid material name ('"+this->md.material+"')");
    }
    if(!this->md.className.empty()){
      this->md.className = this->md.material+"_"+this->md.className;
    }
  } // end of ModelDSLCommon::treatMaterial

  void ModelDSLCommon::treatLibrary()
  {
    const auto& l = this->readOnlyOneToken();
    if(!CxxTokenizer::isValidIdentifier(l,true)){
      this->throwRuntimeError("ModelDSLCommon::treatMaterial",
			      "invalid library name");
    }
    if(!this->md.library.empty()){
      this->throwRuntimeError("ModelDSLCommon::treatMaterial",
			      "library name already registred");
    }
    this->md.library = l;
  } // end of ModelDSLCommon::treatLibrary

  void ModelDSLCommon::treatModel()
  {
    if(!this->md.className.empty()){
      this->throwRuntimeError("ModelDSLCommon::treatModel",
			      "model name already defined");
    }
    this->md.className = this->readOnlyOneToken();
    if(!isValidIdentifier(this->md.className)){
      this->throwRuntimeError("ModelDSLCommon::treatModel",
			      "invalid model name");
    }
    if(!this->md.material.empty()){
      this->md.className = this->md.material+"_"+this->md.className;
    }
  } // end of ModelDSLCommon::treatModel

  void ModelDSLCommon::setInterfaces(const std::set<std::string>& inames)
  {
    using namespace std;
    auto& mbif = ModelInterfaceFactory::getModelInterfaceFactory();
    for(const auto& i : inames){
      if(this->interfaces.count(i)==0){
	this->interfaces.insert({i,mbif.getInterface(i)});
      }
    }
  } // end of ModelDSLCommon::setInterfaces

  void ModelDSLCommon::generateOutputFiles()
  {
    if(this->interfaces.empty()){
      this->throwRuntimeError("ModelDSLCommon::generateOutputFiles",
			      "no interface defined");
    }
    //! generating sources du to external material properties and models
    for(const auto& em : this->externalMFrontFiles){
      this->callMFront(em.second,{em.first});
    }
    //! generating sources by the interfaces
    for(const auto& i : this->interfaces){
      i.second->writeOutputFiles(this->fd,this->md);
    }
  } // end of ModelDSLCommon::writeOutputFiles

  void ModelDSLCommon::treatUnknownKeyword()
  {
    TokensContainer::const_iterator p2;
    auto treated = false;
    --(this->current);
    const auto key = this->current->value;
    ++(this->current);
    this->checkNotEndOfFile("ModelDSLCommon::treatUnknownKeyword");
    if(this->current->value=="["){
      ++(this->current);
      this->checkNotEndOfFile("ModelDSLCommon::treatUnknownKeyword");
      auto s = std::vector<std::string>{};
      while(this->current->value!="]"){
	this->checkNotEndOfFile("ModelDSLCommon::treatUnknownKeyword");
	const auto t = [this]() -> std::string {
	  if(this->current->flag==tfel::utilities::Token::String){
	    return this->current->value.substr(1,this->current->value.size()-2);
	  }
	  return this->current->value;
	}();
	++(this->current);
	this->checkNotEndOfFile("ModelDSLCommon::treatUnknownKeyword");
	if(std::find(s.begin(),s.end(),t)==s.end()){
	  s.push_back(t);
	}
	if(this->current->value!="]"){
	  this->readSpecifiedToken("ModelDSLCommon::treatUnknownKeyword",",");
	  this->checkNotEndOfFile("ModelDSLCommon::treatUnknownKeyword");
	  if(this->current->value=="]"){
	    this->throwRuntimeError("ModelDSLCommon::treatUnknownKeyword",
				    "unexpected token ']'");
	  }
	}
      }
      ++(this->current);
      for(auto& i : this->interfaces){
	auto p = i.second->treatKeyword(key,s,this->current,
				   this->tokens.end());
	if(p.first){
	  if(treated){
	    if(p2!=p.second){
	      this->throwRuntimeError("ModelDSLCommon::treatUnknownKeyword",
				      "the keyword '"+key+"' has been treated "
				      "by two interfaces/analysers but "
				      "results were differents");
	    }
	  }
	  p2 = p.second;
	  treated = true;
	}
      }
      if(!treated){
	this->ignoreKeyWord(key);
	return;
      }
    } else {
      for(const auto&i : this->interfaces){
	auto p = i.second->treatKeyword(key,{},
					this->current,
					this->tokens.end());
	if(p.first){
	  if(treated){
	    if(p2!=p.second){
	      this->throwRuntimeError("ModelDSLCommon::treatUnknownKeyword",
				      "the keyword '"+key+"' has been treated "
				      "by two interfaces/analysers but "
				      "results were differents");
	    }
	  }
	  p2 = p.second;
	  treated = true;
	}
      }
    }
    if(!treated){
      DSLBase::treatUnknownKeyword();
    }
    this->current = p2;
  } // end of ModelDSLCommon::treatUnknownKeyword

  void ModelDSLCommon::treatDomain()
  {
    auto throw_if = [](const bool b,const std::string& m){
      if(b){throw(std::runtime_error("ModelDSLCommon::treatDomain: "+m));}
    };    
    throw_if(!this->md.domains.empty(),"domain defined");
    this->checkNotEndOfFile("ModelDSLCommon::treatDomain");
    throw_if(this->current->flag!=tfel::utilities::Token::String,
	     "Expected to read a string (read '"+this->current->value+"').");
    throw_if(this->current->value.size()<2,"domain name too short.");
    this->md.domains.insert(this->current->value.substr(1,this->current->value.size()-2));
    ++(this->current);
    this->readSpecifiedToken("ModelDSLCommon::treatDomain",";");
  } // end of ModelDSLCommon::treatDomain()

  void ModelDSLCommon::treatDomains()
  {
    auto throw_if = [](const bool b,const std::string& m){
      if(b){throw(std::runtime_error("ModelDSLCommon::treatDomains: "+m));}
    };    
    throw_if(!this->md.domains.empty(),"domains defined");
    for(const auto& d : this->readArrayOfString("ModelDSLCommon::treatDomains")){
      throw_if(!this->md.domains.insert(d).second,
	       "domain "+d+" already defined.");
    }
    throw_if(this->md.domains.empty(),"@Domains does not set any domain");
    this->readSpecifiedToken("ModelDSLCommon::treatDomain",";");
  } // end of ModelDSLCommon::treatDomains()

  bool ModelDSLCommon::isInputVariable(const std::string& v) const
  {
    return is(this->md.inputs,v);
  } // end of ModelDSLCommon::isInputVariable()

  bool ModelDSLCommon::isOutputVariable(const std::string& v) const
  {
    return is(this->md.outputs,v);
  } // end of ModelDSLCommon::isInputVariable()
  
  void ModelDSLCommon::treatFunction()
  {
    auto throw_if = [this](const bool b,const std::string& m){
      if(b){this->throwRuntimeError("ModelDSLCommon::treatFunction",m);}
    };
    auto isStaticMemberName = [](const ModelDescription& d,
				 const std::string& n) -> bool{
      for(const auto& v : d.staticMemberNames){
	if(v==n){
	  return true;
	}
      }
      return false;
    };
    auto isMemberName = [](const ModelDescription& d,
			   const std::string& n) -> bool{
      for(const auto& v : d.memberNames){
	if(v==n){
	  return true;
	}
      }
      return false;
    };
    ModelDescription::Function f;
    unsigned int openedBrackets = 0;
    unsigned int openedParenthesis = 0;
    f.useTimeIncrement = false;
    this->md.registerMemberName("functor"+std::to_string(this->md.functions.size()));
    this->checkNotEndOfFile("ModelDSLCommon::treatFunction");
    f.name = this->current->value;
    throw_if(!this->isValidIdentifier(f.name),
	     "function name '"+f.name+"' is not valid");
    this->md.registerMemberName(f.name);
    this->reserveName(f.name+".Domain");
    this->reserveName(f.name+".Domains");
    f.line = this->current->line;
    ++(this->current);
    this->readSpecifiedToken("ModelDSLCommon::treatFunction","{");
    ++openedBrackets;
    this->checkNotEndOfFile("ModelDSLCommon::treatFunction",
			    "expected body of function '"+f.name+"'.");
    auto currentLine    = this->current->line;
    auto newInstruction = true;
    auto newLine        = true;
    if(getDebugMode()){
      f.body += "#line "+std::to_string(currentLine)+" \""+this->fd.fileName+"\"\n";
    }
    for(;(this->current!=this->tokens.end())&&(openedBrackets!=0);++(this->current)){
      if(this->current->line!=currentLine){
	currentLine=this->current->line;
	f.body += "\n";
	if(getDebugMode()){
	  f.body += "#line "+std::to_string(currentLine)+" \""+this->fd.fileName+"\"\n";
	}
	newLine = true;
      }
      if(this->current->value=="{"){
	++openedBrackets;
	f.body  +="{";
	newInstruction = true;
      } else if(this->current->value=="}"){
	--openedBrackets;
	if(openedBrackets!=0){
	  f.body  +="}";
	}
      } else if(this->current->value=="("){
	++openedParenthesis;
	f.body  +="(";
      } else if(this->current->value==")"){
	throw_if(openedParenthesis==0,"unbalanced parenthesis");
	--openedParenthesis;
	f.body  += ")";
      } else if(this->current->value==";"){
	f.body  += ";";
	newInstruction = true;
      } else {
	if(!newLine){
	  f.body  += " ";
	}
	if(isStaticMemberName(this->md,this->current->value)){
	  f.body  += "(" + this->md.className + ":: "+ this->current->value + ")";
	} else if(isMemberName(this->md,this->current->value)){
	  bool treated = false;
	  if(newInstruction){
	    const auto var = this->current->value;
	    auto op = std::string{};
	    ++(this->current);
	    throw_if(this->current==tokens.end(),
		     "unexpected end of file while reading "
		     "body of function '"+f.name+"'");
	    auto modifier = false;
	    if(this->current->value=="="){
	      op = "=";
	      modifier = true;
	    } else if(this->current->value=="+="){
	      op = "+=";
	      modifier = true;
	    } else if(this->current->value=="-="){
	      op = "-=";
	      modifier = true;
	    } else if(this->current->value=="*="){
	      op = "*=";
	      modifier = true;
	    } else if(this->current->value=="/="){
	      op = "/=";
	      modifier = true;
	    }
	    if(modifier){
	      bool found = false;
	      for(auto p=this->md.outputs.begin();(p!=this->md.outputs.end())&&(!found);){
		if(p->name==var){
		  found = true;
		} else {
		  ++p;
		}
	      }
	      throw_if(!found,"trying to modify variable '"+var+
		       "' in the body of function '"+f.name+"'");
	      f.body  += var + " " + op + " ";
	      f.modifiedVariables.insert(var);
	      auto p6 = f.depths.find(var);
	      if(p6==f.depths.end()){
		f.depths[var] = 0;
	      }	      
	      treated = true;
	    } else {
	      --(this->current);
	    }
	  }
	  if(!treated){
	    // treating the case of local parameters
	    for(auto p =this->md.parameters.begin();
		(p!=this->md.parameters.end())&&(!treated);++p){
	      if(p->name==this->current->value){
		treated = true;
		f.parameters.insert(this->current->value);
		f.body  += "(this->" + this->current->value + ")";
	      }
	    }
	    // treating the case of local parameters
	    for(auto p =this->md.constantMaterialProperties.begin();
		(p!=this->md.constantMaterialProperties.end())&&(!treated);++p){
	      if(p->name==this->current->value){
		treated = true;
		f.constantMaterialProperties.insert(this->current->value);
		f.body  += "(this->" + this->current->value + ")";
	      }
	    }
	    if(!treated){
	      if(this->isInputVariable(this->current->value)){
		f.usedVariables.insert(this->current->value);
		auto dv = this->md.decomposeVariableName(this->current->value);
		auto p6 = f.depths.find(dv.first);
		if(p6!=f.depths.end()){
		  if(dv.second>p6->second){
		    f.depths[dv.first] = dv.second;
		  }
		} else {
		  f.depths[dv.first] = dv.second;
		}
	      } else if(this->isOutputVariable(this->current->value)){
		f.usedVariables.insert(this->current->value);
		auto dv = this->md.decomposeVariableName(this->current->value);
		auto p6 = f.depths.find(dv.first);
		if(p6!=f.depths.end()){
		  if(dv.second>p6->second){
		    f.depths[dv.first] = dv.second;
		  }
		} else {
		  f.depths[dv.first] = dv.second;
		}
	      }
	      f.body += this->current->value;
	    }
	  }
	} else {
	  if(this->current->value=="dt"){
	    f.useTimeIncrement = true;
	    f.body  += "dt";
	  } else {
	    f.body  += this->current->value;
	  }
	}
	newInstruction = false;
      }
      newLine=false;
    }
    throw_if((this->current==tokens.end())&&(openedBrackets!=0),
	     "unexpected end of file while reading body of function '"+f.name+"'");
    throw_if(openedBrackets!=0,"parenthesis still opened at the end of function '"+f.name+"'");
    throw_if(f.modifiedVariables.empty(),"function " + f.name + " does not change any variable.");
    for(const auto& df: this->md.functions){
      throw_if(df.name==f.name,"function " + f.name + " already declared.");
    }
    for(auto p2=f.modifiedVariables.begin();p2!=f.modifiedVariables.end();++p2){
      auto p3=f.usedVariables.find(*p2);
      if(p3!=f.usedVariables.end()){
	f.usedVariables.erase(*p3);
      }
    }
    this->md.functions.push_back(f);
  } // end of ModelDSLCommon::treatFunction()

  void ModelDSLCommon::treatOutput()
  {
    if(!this->md.functions.empty()){
      this->throwRuntimeError("ModelDSLCommon::treatInput",
			       "outputs must be declared before "
			       "declaring functions");
    }
    VariableDescriptionContainer noutputs;
    this->readVarList(noutputs,"real",false);
    for(const auto& v : noutputs){
      this->md.registerMemberName(v.name);
      this->md.outputs.push_back(v);
    }
  } // end of ModelDSLCommon::treatOutput()

  void ModelDSLCommon::treatInput()
  {
    if(!this->md.functions.empty()){
      this->throwRuntimeError("ModelDSLCommon::treatInput",
			      "inputs must be declared before "
			      "declaring functions");
    }
    VariableDescriptionContainer ninputs;
    this->readVarList(ninputs,"real",false);
    for(const auto& v : ninputs){
      this->md.registerMemberName(v.name);
      this->md.inputs.push_back(v);
    }
  } // end of ModelDSLCommon::treatInput()

  void ModelDSLCommon::treatOutputMethod() 
  {
    if(!this->md.functions.empty()){
      this->throwRuntimeError("ModelDSLCommon::treatOutputMethod: ",
			      "variable methods must be called "
			      "before declaring functions");
    }
    this->readSpecifiedToken("ModelDSLCommon::treatOutputMethod",".");
    this->checkNotEndOfFile("ModelDSLCommon::treatOutputMethod",
			    "Expected method name.");
    const auto mn = this->current->value;
    if((mn!="setGlossaryName")&&(mn!="setEntryName")&&
       (mn!="setDefaultInitialValue")&&(mn!="setDepth")){
      this->throwRuntimeError("ModelDSLCommon::treatOutputMethod",
			      "Unknown method (valid methods for fields are "
			      "setGlossaryName, setEntryName, setDepth and "
			      "setDefaultInitialValue,"
			      "read '"+mn+"').");
    }
    ++(this->current);
    this->readSpecifiedToken("ModelDSLCommon::treatOutputMethod","(");
    if(mn=="setGlossaryName"){
      const auto gn = this->readString("ModelDSLCommon::treatOutputMethod");
      this->md.setGlossaryName(this->currentVar,gn);
    } else if (mn=="setEntryName"){
      const auto en = this->readString("ModelDSLCommon::treatOutputMethod");
      this->md.setEntryName(this->currentVar,en);
    }  else if (mn=="setDefaultInitialValue"){
      this->checkNotEndOfFile("ModelDSLCommon::treatOutputMethod",
			      "Expected intial value.");
      const auto value = this->readDouble();
      auto& v = this->md.outputs.getVariable(this->currentVar);
      v.setAttribute(VariableDescription::initialValue,value,false);
    } else if (mn=="setDepth"){
      this->checkNotEndOfFile("ModelDSLCommon::treatOutputMethod",
			      "Expected depth value.");
      unsigned short value;
      std::istringstream converter(this->current->value);
      converter >> value;
      if(!converter||(!converter.eof())){
	this->throwRuntimeError("ModelDSLCommon::treatOutputMethod",
				"Could not read depth value of field '"+this->currentVar+"'");
      }
      auto& v = this->md.outputs.getVariable(this->currentVar);
      v.setAttribute(VariableDescription::depth,value,false);
      for(unsigned short i=1;i<=value;++i){
	const auto vn = this->currentVar+"_"+std::to_string(i);
	this->md.registerMemberName(vn);
	this->md.registerMemberName("f_"+vn);
	this->md.registerMemberName("ff_"+vn);
      }
      ++(this->current);
    } else {
      this->throwRuntimeError("ModelDSLCommon::treatOutputMethod",
			      "Internal error (untreated method '"+ mn +"')");
    }
    this->readSpecifiedToken("ModelDSLCommon::treatOutputMethod",")");
    this->readSpecifiedToken("ModelDSLCommon::treatOutputMethod",";");
  } // end of ModelDSLCommon::treatOutputMethod

  void ModelDSLCommon::treatInputMethod() 
  {
    if(!this->md.functions.empty()){
      this->throwRuntimeError("ModelDSLCommon::treatInputMethod",
			      "input methods must be called "
			      "before declaring functions");
    }
    this->readSpecifiedToken("ModelDSLCommon::treatInputMethod",".");
    this->checkNotEndOfFile("ModelDSLCommon::treatInputMethod",
			    "Expected method name.");
    const auto mn = this->current->value;
    if((mn!="setGlossaryName")&&(mn!="setEntryName")&&(mn!="setDepth")){
      this->throwRuntimeError("ModelDSLCommon::treatInputMethod",
			      "Unknown method (valid methods for input fields are "
			      "setGlossaryName, setEntryName, setDepth"
			      ", read '"+mn+"')");
    }
    ++(this->current);
    this->readSpecifiedToken("ModelDSLCommon::treatInputMethod","(");
    if(mn=="setGlossaryName"){
      const auto gn = this->readString("ModelDSLCommon::treatInputMethod");
      this->md.setGlossaryName(this->currentVar,gn);
    } else if (mn=="setEntryName"){
      const auto en = this->readString("ModelDSLCommon::treatInputMethod");
      this->md.setEntryName(this->currentVar,en);
    } else if (mn=="setDepth"){
      this->checkNotEndOfFile("ModelDSLCommon::treatInputMethod",
			      "Expected depth value.");
      unsigned short value;
      std::istringstream converter(this->current->value);
      converter >> value;
      if(!converter||(!converter.eof())){
	this->throwRuntimeError("ModelDSLCommon::treatInputMethod",
				"Could not read initial value of field '"+this->currentVar+"'");
      }
      auto& v = this->md.inputs.getVariable(this->currentVar);
      v.setAttribute(VariableDescription::depth,value,false);
      for(unsigned short i=1;i<=value;++i){
	const auto vn = this->currentVar+"_"+std::to_string(i);
	this->md.registerMemberName(vn);
	this->md.registerMemberName("f_"+vn);
	this->md.registerMemberName("ff_"+vn);
      }
      ++(this->current);
    } else {
      this->throwRuntimeError("ModelDSLCommon::treatInputMethod",
			      "Internal error (untreated method '"+ mn +"')");
    }
    this->readSpecifiedToken("ModelDSLCommon::treatInputMethod",")");
    this->readSpecifiedToken("ModelDSLCommon::treatInputMethod",";");
  } // end of ModelDSLCommon::treatInputMethod

  void ModelDSLCommon::treatParameter()
  {
    auto endOfTreatment=false;
    while((this->current!=this->tokens.end())&&
	  (!endOfTreatment)){
      if(!this->isValidIdentifier(this->current->value)){
	this->throwRuntimeError("DSLBase::handleParameter : ",
				"variable given is not valid (read '"+this->current->value+"').");
      }
      const auto n = this->current->value;
      const auto lineNumber = this->current->line;
      VariableDescription v("real",n,1u,lineNumber);
      ++(this->current);
      this->checkNotEndOfFile("DSLBase::handleParameter");
      auto value = this->readInitialisationValue<double>(n,false);
      if(value.first){
	v.setAttribute(VariableDescription::defaultValue,value.second,false);
      }
      if(this->current->value==","){
	++(this->current);
      } else if (this->current->value==";"){
	endOfTreatment=true;
	++(this->current);
      } else {
	this->throwRuntimeError("DSLBase::handleParameter",
				", or ; expected after '"+n+"'");
      }
      this->md.registerMemberName(v.name);
      this->md.parameters.push_back(v);
    }
    if(!endOfTreatment){
      --(this->current);
      this->throwRuntimeError("DSLBase::handleParameter",
			      "Expected ';' before end of file");
    }
  } // end of ModelDSLCommon::treatParameter()

  void ModelDSLCommon::treatLocalParameter()
  {
    VariableDescriptionContainer gp;
    this->readVarList(gp,false);
    for(const auto& v : gp){
      this->md.registerMemberName(v.name);
      this->md.parameters.push_back(v);
    }
  } // end of ModelDSLCommon::treatLocalParameter()
    
  void ModelDSLCommon::treatParameterMethod() 
  {
    auto throw_if = [](const bool b,const std::string& m){
      if(b){throw(std::runtime_error("ModelDSLCommon::treatParameterMethod: "+m));}
    };
    this->readSpecifiedToken("ModelDSLCommon::treatParameterMethod",".");
    this->checkNotEndOfFile("ModelDSLCommon::treatParameterMethod",
			    "Expected method name.");
    const auto mn = this->current->value;
    throw_if((mn!="setGlossaryName")&&(mn!="setEntryName")&&
	     (mn!="setDefaultValue"),
	     "unknown method (valid methods for local parameters are "
	     "setGlossaryName, setEntryName and setDefaultValue"
	     ", read "+mn+").");
    ++(this->current);
    this->readSpecifiedToken("ModelDSLCommon::treatParameterMethod","(");
    if(mn=="setGlossaryName"){
      const auto gn = this->readString("ModelDSLCommon::treatInputMethod");
      this->md.setGlossaryName(this->currentVar,gn);
    } else if(mn=="setEntryName"){
      const auto en = this->readString("ModelDSLCommon::treatInputMethod");
      this->md.setEntryName(this->currentVar,en);
    } else if (mn=="setDefaultValue"){
      this->readDefaultValue();
    }
    this->readSpecifiedToken("ModelDSLCommon::treatParameterMethod",")");
    this->readSpecifiedToken("ModelDSLCommon::treatParameterMethod",";");
  } // end of ModelDSLCommon::treatParameterMethod

  void ModelDSLCommon::treatConstantMaterialProperty()
  {
    VariableDescriptionContainer cmp;
    this->readVarList(cmp,"real",false);
    for(const auto& mp : cmp){
      this->md.registerMemberName(mp.name);
      this->md.constantMaterialProperties.push_back(mp);
    }
  } // end of ModelDSLCommon::treatConstantMaterialProperty()

  void ModelDSLCommon::treatConstantMaterialPropertyMethod() 
  {
    auto throw_if = [](const bool b,const std::string& m){
      if(b){throw(std::runtime_error("ModelDSLCommon::treatConstantMaterialPropertyMethod: "+m));}
    };
    this->readSpecifiedToken("ModelDSLCommon::treatConstantMaterialPropertyMethod",".");
    this->checkNotEndOfFile("ModelDSLCommon::treatConstantMaterialPropertyMethod",
			    "Expected method name.");
    const auto mn = this->current->value;
    ++(this->current);
    this->readSpecifiedToken("ModelDSLCommon::treatConstantMaterialPropertyMethod","(");
    if(mn=="setGlossaryName"){
      const auto gn = this->readString("ModelDSLCommon::treatConstantMaterialPropertyMethod");
      this->md.setGlossaryName(this->currentVar,gn);
    } else if(mn=="setEntryName"){
      const auto en = this->readString("ModelDSLCommon::treatConstantMaterialPropertyMethod");
      this->md.setEntryName(this->currentVar,en);
    } else {
      throw_if(true,"unknown method (valid methods for constant material properties are "
	       "setGlossaryName and setEntryName, read "+mn+").");
    }
    this->readSpecifiedToken("ModelDSLCommon::treatConstantMaterialPropertyMethod",")");
    this->readSpecifiedToken("ModelDSLCommon::treatConstantMaterialPropertyMethod",";");
  } // end of ModelDSLCommon::treatConstantMaterialPropertyMethod

  void ModelDSLCommon::readDefaultValue()
  {
    auto throw_if = [](const bool b,const std::string& m){
      if(b){throw(std::runtime_error("ModelDSLCommon::readDefaultValue: "+m));}
    };
    auto& v = this->md.parameters.getVariable(this->currentVar);
    this->checkNotEndOfFile("ModelDSLCommon::readDefaultValue",
			    "Expected default value.");
    if(v.type=="DoubleArray"){
      const auto values = this->readArrayOfDouble("ModelDSLCommon::readDefaultValue");
      v.setAttribute(VariableDescription::defaultValue,values,false);
    } else if (v.type=="StringArray"){
      const auto values = this->readArrayOfDouble("ModelDSLCommon::readDefaultValue");
      v.setAttribute(VariableDescription::defaultValue,values,false);
    } else if ((v.type=="double")||(v.type=="real")){
      const auto value = this->readDouble();
      v.setAttribute(VariableDescription::defaultValue,value,false);
    } else if (v.type=="string"){
      const auto value = this->readString("ModelDSLCommon::readDefaultValue");
      v.setAttribute(VariableDescription::defaultValue,value,false);
    } else if (v.type=="bool"){
      const auto b = this->readBooleanValue("ModelDSLCommon::readDefaultValue");
      v.setAttribute(VariableDescription::defaultValue,b,false);
    } else {
      throw_if(true,"type '"+v.type+"' is not supported.");
    }
  } // end of ModelDSLCommon::readDefaultValue

  void ModelDSLCommon::treatBounds()
  {
    const auto b = mfront::readVariableBounds(this->current,this->end());
    if(this->md.outputs.contains(b.first)){
      this->md.outputs.getVariable(b.first).setBounds(b.second);
    } else if(this->md.inputs.contains(b.first)){
      this->md.inputs.getVariable(b.first).setBounds(b.second);
    } else {
      throw(std::runtime_error("ModelDSLCommon::treatBounds: "
			       "no variable named '"+b.first+"'"));
    }
    this->readSpecifiedToken("ModelDSLCommon::treatBounds",";");
  } // end of ModelDSLCommon::treatBounds

  void ModelDSLCommon::treatPhysicalBounds()
  {
    const auto b = mfront::readVariableBounds(this->current,this->end());
    if(this->md.outputs.contains(b.first)){
      this->md.outputs.getVariable(b.first).setPhysicalBounds(b.second);
    } else if(this->md.inputs.contains(b.first)){
      this->md.inputs.getVariable(b.first).setPhysicalBounds(b.second);
    } else {
      throw(std::runtime_error("ModelDSLCommon::treatPhysicalBounds: "
			       "no variable named '"+b.first+"'"));
    }
    this->readSpecifiedToken("ModelDSLCommon::treatPhysicalBounds",";");
  } // end of ModelDSLCommon::treatPhysicalBounds

  void ModelDSLCommon::addMaterialLaw(const std::string& m)
  {
    this->md.addMaterialLaw(m);
  } // end of ModelDSLCommon::addMaterialLaw

  void ModelDSLCommon::appendToIncludes(const std::string& c)
  {
    this->md.appendToIncludes(c);
  } // end of ModelDSLCommon::appendToIncludes

  void ModelDSLCommon::appendToMembers(const std::string& c)
  {
    this->md.appendToMembers(c);
  } // end of ModelDSLCommon::appendToMembers

  void ModelDSLCommon::appendToPrivateCode(const std::string& c)
  {
    this->md.appendToPrivateCode(c);
  } // end of ModelDSLCommon::appendToPrivateCode

  void ModelDSLCommon::appendToSources(const std::string& c)
  {
    this->md.appendToSources(c);
  } // end of ModelDSLCommon::appendToSources

  ModelDSLCommon::~ModelDSLCommon() = default;

} // end of namespace mfront
