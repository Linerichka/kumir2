#ifndef SYNTAXANALIZER_H
#define SYNTAXANALIZER_H

#include <QtCore>
#include "statement.h"

#include "dataformats/ast.h"
#include "dataformats/ast_algorhitm.h"
#include "interfaces/analizerinterface.h"

typedef AST::Data AST_Data;
typedef AST::Algorhitm AST_Algorhitm;

namespace KumirAnalizer {

class SyntaxAnalizer : public QObject
{
    Q_OBJECT
public:
    explicit SyntaxAnalizer(class Lexer * lexer, const QStringList & alwaysEnabledModules, QObject *parent = 0);
    void init(QList<Statement*> & statements
              , AST_Data * ast
              , AST_Algorhitm *algorhitm);
    void syncStatements();
    QStringList unresolvedImports() const;
    void setSourceDirName(const QString & dirName);
    void buildTables(bool allowOperatorsDeclaration);
    QList<Shared::Suggestion> suggestAutoComplete(const Statement * statementBefore,
                                                  const QList<Lexem*> lexemsAfter,
                                                  const AST::Module * contextModule,
                                                  const AST::Algorhitm * contextAlgorithm
                                                  ) const;
    void processAnalisys();
    ~SyntaxAnalizer();
private:
    struct SyntaxAnalizerPrivate * d;
};

} // namespace KumirAnalizer

#endif // SYNTAXANALIZER_H
