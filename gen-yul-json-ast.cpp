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
	// if (argc != 3)
	// {
	// 	std::cerr << "USAGE: " << argv[0] << " SOLIDITY-FILE "
	// 			  << "MAIN-CONTRACT-NAME" << std::endl;
	// 	std::cerr
	// 		<< "Where MAIN-CONTRACT-NAME is the name of the primary contract "
	// 		   "(non-interface,  non-library, non-abstract contract)"
	// 		<< std::endl;
	// 	return 1;
	// }

	// char const* sol_filepath	 = argv[1];
	// std::string main_contract	 = argv[2];
	// std::string contractContents = slurpFile(sol_filepath);

	// // =============== Solidity pre-pass ===============
	// solidity::langutil::CharStream charStream{contractContents, sol_filepath};
	// solidity::langutil::ErrorList  errors;
	// solidity::langutil::ErrorReporter errorReporter{errors};
	// solidity::frontend::Parser		  parser{errorReporter,
	// 									 solidity::langutil::EVMVersion()};

	// auto	   sourceUnit = parser.parse(charStream);
	// SourceData sourceData(main_contract, contractContents, sol_filepath);
	// sourceData.m_srcSplit		  = splitStr(sourceData.m_src);
	// sourceData.m_srcSplitOriginal = splitStr(sourceData.m_src);
	// sourceData.compressSigs();
	// sourceData.prepareSoliditySource(sourceData.m_filepath.c_str());

  	// Anything stored here can be replaced in the string.
    std::map<std::string, std::string> vars {
        {"to", "to_addr_t"},
    };

    // Matches anything between brackets.
    std::regex r("(?:^|\\W)to(?:$|\\W)");

    std::string str = "goto todo _to";

	std::cout << std::regex_replace(str, r, "HELLOOOOOOO") << std::endl;
    // We need to keep track of where we are, or else we would need to search from the start of
    // the string everytime, which is very wasteful.
    // std::regex_iterator won't help, because the replacement may be smaller
    // than the match, and it would cause strings like "[Player1][Player1]" to not match properly.
    // auto pos=str.cbegin();
    // do {
    //     // First, we try to get a match. If there's no more matches, exit.
    //     std::smatch m;
    //     regex_search(pos, str.cend(), m, r);
    //     if (m.empty()) break;

    //     // The interface of std::match_results is terrible. Let's get what we need and
    //     // place it in apropriately named variables.
    //     auto var_name = m[1].str();
    //     auto start = m[0].first;
    //     auto end = m[0].second;

    //     auto value = std::string("to_addr_t");

    //     // This does the actual replacement
    //     str.replace(start, end, value);

    //     // We update our position. The new search will start right at the end of the replacement.
    //     pos = m[0].first + value.size();
    // } while(true);

    std::cout << str << std::endl;

	return 0;
}
