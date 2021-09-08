#include <boost/exception/diagnostic_information.hpp>
#include <boost/throw_exception.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>

#include <liblangutil/CharStreamProvider.h>
#include <liblangutil/SourceReferenceFormatter.h>
#include <libsolidity/interface/OptimiserSettings.h>
#include <libyul/AST.h>
#include <libyul/AsmJsonConverter.h>
#include <solc/CommandLineInterface.h>
#include <tools/yulPhaser/Program.h>

#include "Prepass.h"

using namespace solidity;
using namespace std;

string slurpFile(string_view path)
{
	constexpr size_t BUF_SIZE = 4096;

	string result;
	ifstream is{path.data()};
	is.exceptions(ifstream::badbit);
	string buf(BUF_SIZE, '\0');
	while (is.read(buf.data(), BUF_SIZE))
		result.append(buf, 0, is.gcount());
	result.append(buf, 0, is.gcount());

	return result;
}

langutil::CharStream generateIR(char const* sol_filepath)
{
	std::string yulOptimiserSteps = frontend::OptimiserSettings::DefaultYulOptimiserSteps;
	erase(yulOptimiserSteps, 'i'); // remove FullInliner
	yulOptimiserSteps += " x"; // that flattens function calls: only one
							   // function call per statement is allowed
	constexpr int solc_argc = 6;
	char const* solc_argv[solc_argc] = {
		"irToJson",
		"--optimize",
		"--ir-optimized",
		"--yul-optimizations",
		yulOptimiserSteps.c_str(),
		sol_filepath,
	};

	std::istringstream sin; // never used, but the CLI requires it
	std::ostringstream sout;
	frontend::CommandLineInterface cli{sin, sout, std::cerr};
	if (not cli.parseArguments(solc_argc, solc_argv))
		BOOST_THROW_EXCEPTION(std::runtime_error{"solc CLI failed to parse arguments"});
	if (not cli.readInputFiles())
		BOOST_THROW_EXCEPTION(std::runtime_error{"solc failed to read input files"});
	if (not cli.processInput())
		BOOST_THROW_EXCEPTION(std::runtime_error{"solc failed to process input"});
	if (not cli.actOnInput())
		BOOST_THROW_EXCEPTION(std::runtime_error{"solc failed to act on input"});

	std::string_view ir = sout.view();
	constexpr std::string_view IR_HEADER = "Optimized IR:";
	if (ir.substr(0, IR_HEADER.size()) != IR_HEADER)
	{
		std::ostringstream es;
		es << "Expected '" << IR_HEADER << "' header in solc IR output but not found" << std::endl;
		BOOST_THROW_EXCEPTION(std::runtime_error{es.str()});
	}
	ir.remove_prefix(IR_HEADER.size());

	return langutil::CharStream{ir.data(), "ir_stream"};
}

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cerr << "USAGE: " << argv[0] << " SOLIDITY-FILE " << "MAIN-CONTRACT-NAME" << std::endl;
		std::cerr << "where MAIN CONTRACT NAME is the name of the primary contract" << std::endl;
		return 1;
	}
	char const* sol_filepath = argv[1];
	string main_contract = argv[2];
	langutil::CharStream irStream;
	try
	{
		irStream = generateIR(sol_filepath);
	}
	catch (boost::exception const& exc)
	{
		std::cerr << boost::diagnostic_information(exc) << std::endl;
		return 1;
	}
	string irSource = irStream.source();
	auto yul = cleanYul(irSource, main_contract);
	langutil::CharStream ir = langutil::CharStream(yul, sol_filepath);

	std::variant<phaser::Program, langutil::ErrorList> maybeProgram
		= phaser::Program::load(ir);
	if (auto* errorList = std::get_if<langutil::ErrorList>(&maybeProgram))
	{
		langutil::SingletonCharStreamProvider streamProvider{ir};
		langutil::SourceReferenceFormatter{std::cerr, streamProvider, true, false}
			.printErrorInformation(*errorList);
		std::cerr << std::endl;
		return 1;
	}

	yul::Block const& ast = get<phaser::Program>(maybeProgram).ast();
	yul::AsmJsonConverter jsonConverter{{}};
	std::cout << jsonConverter(ast) << std::endl;

	return 0;
}
