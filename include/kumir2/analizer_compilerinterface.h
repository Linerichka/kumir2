#ifndef ANALIZER_COMPILERINTERFACE_H
#define ANALIZER_COMPILERINTERFACE_H

#include <QtPlugin>

namespace AST
{
	struct Data;
	typedef QSharedPointer<Data> DataPtr;
}

namespace Shared
{
namespace Analizer
{

enum RunTarget {
	RegularRun,
	TestingRun
};

class ASTCompilerInterface
{
public:
	virtual const AST::DataPtr abstractSyntaxTree() const = 0;
};

class ExternalExecutableCompilerInterface
{
public:
	virtual QString prepareToRun(RunTarget target) = 0;
	virtual QString executableFilePath() const = 0;
	virtual QString debuggableSourceFileName() const = 0;
};


} // namespace Analizer
} // namespace Shared

Q_DECLARE_INTERFACE(Shared::Analizer::ASTCompilerInterface,
	"kumir2.Analizer.CompilerInterface")
Q_DECLARE_INTERFACE(Shared::Analizer::ExternalExecutableCompilerInterface,
	"kumir2.Analizer.ExternalExecutableCompilerInterface")

#endif // ANALIZER_COMPILERINTERFACE_H
