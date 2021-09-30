#pragma once

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/interface/OptimiserSettings.h>
#include <libsolidity/parsing/Parser.h>
#include <solc/CommandLineInterface.h>

#include <string>

#include "common/library.hpp"
#include "yul_prepass/Prepass.hpp"

using namespace solidity::frontend;
using namespace solidity;

struct PublicFunctions
{
	std::vector<std::string> names;
	std::vector<std::string> parameters;
	std::vector<std::string> hashes;
	std::vector<std::string> sigs;
	std::vector<std::string> markedSigs;
	std::vector<std::string> sigsToReplace;
};

struct ContractData
{
	std::vector<ASTPointer<ASTNode>> subNodes;
	std::vector<FunctionCall const*> functionCalls;
};

struct MarkedFunctions
{
	std::vector<std::string>			  names;
	std::vector<std::string>			  selectors;
	std::vector<std::vector<Type const*>> parameters;
};

struct FunctionData
{
	std::vector<Type const*> parameterTypes;
	std::vector<Type const*> returnTypes;
	std::vector<Type const*> bodyTypes;
	Block const&			 body;
	bool					 hasForLoop;
};

class SourceData: public ASTConstVisitor
{
public:
	enum class PassType
	{
		AddrTypePass,
		ConstructorPass,
		FunctionCallPass,
		FunctionDefinitionPass,
		StorageVarPass,
	};

	SourceData(std::string main_contract,
			   std::string src,
			   std::string filepath);

	CommandLineInterface	  getCli(char const* sol_filepath);
	FunctionDefinition const* resolveFunctionCall(const ContractDefinition& c,
												  FunctionCall const&		f);
	FunctionDefinition const*
		 insideWhichFunction(langutil::SourceLocation const& location);
	void removeComments();
	int	 getSigEnd(int start);
	bool visit(FunctionDefinition const& _node) override;
	bool visit(FunctionCall const& _node) override;
	bool visit(Identifier const& _node) override;
	void endVisit(FunctionDefinition const& _node) override;
	bool visitNode(ASTNode const& node) override;
	void endVisitNode(ASTNode const& node) override;
	void markAddressTypesInFunArgs(FunctionDefinition const& _node,
								   std::string				 funcFull);
	void prepareSoliditySource(const char* sol_filepath);
	void setCompilerOptions(std::shared_ptr<CompilerStack> compiler);
	void writeModifiedSolidity();
	void compressSigs();
	void dynFuncArgsPass(const char* solFilepath);
	void storageVarPass();
	void addressTypePass();

	std::vector<std::string>						 m_storageVars_str;
	std::vector<std::pair<std::string, std::string>> m_addrMarkedFuncs;
	std::vector<std::string>						 m_srcSplit;
	std::vector<std::string>						 m_srcSplitOriginal;
	std::vector<std::string>						 m_visitedIdents;
	std::vector<std::string>						 m_functionNames;
	std::map<std::string, ContractData>				 m_contracts;
	boost::filesystem::path							 m_baseFileName;

	std::string m_mainContract;
	std::string m_currentFunction;
	std::string m_currentFunctionModified;
	std::string m_contractDef;
	std::string m_filepath;
	std::string m_modifiedSolFilepath;
	std::string m_srcModified;
	std::string m_src;
	std::string m_srcDynArgsFuncPass;
	std::string m_srcDynArgsFuncCallPass;
	std::string m_srcStorageVarsPass;

	std::shared_ptr<CompilerStack>			m_compiler;
	OptimiserSettings						m_compilerOptimizerSettings;
	FileReader								m_fileReader;
	std::shared_ptr<CommandLineInterface>	m_cli;
	CommandLineOptions						m_options;
	std::vector<FunctionDefinition const*>	m_definedFunctions;
	std::vector<const VariableDeclaration*> m_storageVars_astNodes;
	PassType								m_currentPass;

private:
	bool isPublic(Visibility _visibility);
	bool hasDynamicArgs(std::string params);
	bool checkTypeEqaulity(std::vector<Type const*> const& t1,
						   std::vector<Type const*>		   t2);

	std::vector<std::string> m_storageVars;
	std::vector<std::string> m_hashNames;
	PublicFunctions			 m_publicFunctions;
	MarkedFunctions			 m_dynArgFunctions;
	int						 m_publicFunctionCount;
};
