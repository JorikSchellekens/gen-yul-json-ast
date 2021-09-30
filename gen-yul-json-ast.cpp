#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include <liblangutil/CharStreamProvider.h>
#include <liblangutil/SourceReferenceFormatter.h>
#include <libyul/AST.h>
#include <libyul/AsmJsonConverter.h>
#include <solc/CommandLineInterface.h>
#include <tools/yulPhaser/Program.h>

#include "solidity_prepass/WarpVisitor.hpp"
#include "yul_prepass/Prepass.hpp"

std::vector<std::string> splitStr(const std::string& str);

std::string slurpFile(std::string_view path)
{
	constexpr size_t BUF_SIZE = 4096;

	std::string	  result;
	std::ifstream is{path.data()};
	is.exceptions(std::ifstream::badbit);
	std::string buf(BUF_SIZE, '\0');
	while (is.read(buf.data(), BUF_SIZE))
		result.append(buf, 0, is.gcount());
	result.append(buf, 0, is.gcount());

	return result;
}

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cerr << "USAGE: " << argv[0] << " SOLIDITY-FILE "
				  << "MAIN-CONTRACT-NAME" << std::endl;
		std::cerr
			<< "Where MAIN-CONTRACT-NAME is the name of the primary contract "
			   "(non-interface,  non-library, non-abstract contract)"
			<< std::endl;
		return 1;
	}

	char const* sol_filepath	 = argv[1];
	std::string main_contract	 = argv[2];
	std::string contractContents = slurpFile(sol_filepath);

	// =============== Solidity pre-pass ===============
	solidity::langutil::CharStream charStream{contractContents, sol_filepath};
	solidity::langutil::ErrorList  errors;
	solidity::langutil::ErrorReporter errorReporter{errors};
	solidity::frontend::Parser		  parser{errorReporter,
										 solidity::langutil::EVMVersion()};

	auto	   sourceUnit = parser.parse(charStream);
	SourceData sourceData(main_contract, contractContents, sol_filepath);
	sourceData.m_srcSplit		  = splitStr(sourceData.m_src);
	sourceData.m_srcSplitOriginal = splitStr(sourceData.m_src);
	sourceData.compressSigs();
	sourceData.prepareSoliditySource(sourceData.m_filepath.c_str());


    // std::regex r("(\\b|_)to\\b");
    // std::string str = "goto todo a.to";
	// std::cout << std::regex_replace(str, r, "to_addr_t") << std::endl;

	return 0;
}
