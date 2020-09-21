#ifndef LEXEM_H
#define LEXEM_H

#include "lexemtype.h"
#include <QString>
#include <QSharedPointer>

namespace AST
{

struct Lexem {
	enum ErrorStage {
		NoError,
		Lexer,
		BeforePDAutomata,
		PDAutomata,
		Tables,
		Semantics
	} errorStage;

	enum ErrorRaisePosition {
		AsIs,
		Header,
		Begin,
		End
	} errorRaisePosition;

	Lexem()
	{
		type = Shared::LxTypeEmpty;
		lineNo = -1;
		linePos = 0;
		length = 0;
		errorStage = NoError;
		errorRaisePosition = AsIs;
	}

	Shared::LexemType type;
	QString data;
	int lineNo;
	int linePos;
	int length;
	QString error;
};

typedef QSharedPointer<Lexem> LexemPtr;

}

#endif // LEXEM_H
