#ifndef STATEMENT_H
#define STATEMENT_H

#include <QtCore>
#include <kumir2-libs/dataformats/lexem.h>

namespace AST
{
struct Statement;
struct Module;
struct Algorithm;
struct Variable;

typedef QSharedPointer<Statement> StatementPtr;
typedef QSharedPointer<Module> ModulePtr;
typedef QSharedPointer<Algorithm> AlgorithmPtr;
typedef QSharedPointer<Variable> VariablePtr;
}

namespace KumirAnalizer
{

typedef QSharedPointer<struct TextStatement> TextStatementPtr;

struct TextStatement {
	TextStatement(Shared::LexemType lt) : type(lt), conditionalIndex(0) {}
	QList<AST::LexemPtr> data;
	QPoint indentRank;
	Shared::LexemType type;
	AST::StatementPtr statement;
	AST::AlgorithmPtr alg;
	AST::ModulePtr mod;
	QList<AST::VariablePtr> variables;
	int conditionalIndex;
	bool hasError() const;
	void setError(const QString &err,
		const AST::Lexem::ErrorStage stage,
		const AST::Lexem::ErrorRaisePosition raise
	);
	explicit TextStatement();
	mutable QPair<QString, quint32> suggestedClosingBracket;
	mutable QStringList suggestedImportModuleNames;
};

}

#endif // STATEMENT_H
