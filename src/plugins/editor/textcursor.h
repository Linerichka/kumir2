#ifndef TEXTCURSOR_H
#define TEXTCURSOR_H

#include <QtGui>

#include "interfaces/analizerinterface.h"
#include "keycommand.h"

namespace Editor {

class TextCursor : public QObject
{
    Q_OBJECT
public:
    enum EditMode { EM_Insert, EM_Overwrite };
    enum MoveMode { MM_Move, MM_Select, MM_RectSelect };
    enum ViewMode { VM_Blinking, VM_Hidden, VM_Visible };
    explicit TextCursor(class TextDocument * document, class Clipboard * clipboard);
    inline void setEmitCompilationBlocked(bool v) { b_emitCompilationBlocked = v; }
    ~TextCursor();
    inline int row() const { return i_row; }
    inline int column() const { return i_column; }
    void moveTo(int row, int col);
    void selectRangeBlock(int fromRow, int fromCol, int toRow, int toCol);
    void selectRangeText(int fromRow, int fromCol, int toRow, int toCol);
    inline EditMode mode() const { return e_mode; }
    inline void setMode(EditMode m) { e_mode = m; emit updateRequest();}
    inline bool isEnabled() const { return b_enabled; }
    inline bool isVisible() const { return b_enabled && b_visible; }
    void setViewMode(ViewMode mode);
    inline void setEnabled(bool v) { b_enabled = v; emit updateRequest();}
    bool hasSelection() const;
    inline bool hasRectSelection() const { return rect_selection.x()!=-1 && rect_selection.y()!=-1; }
    inline QRect selectionRect() const { return rect_selection; }
    void selectionBounds(int &fromRow, int &fromCol, int &toRow, int &toCol) const;
    QStringList rectSelectionText() const;
    void removeRectSelection();
    void insertText(const QString &text);
    void insertBlock(const QStringList & block);
    void removePreviousChar();
    void removeCurrentChar();
    void removeSelection();
    void removeSelectedText();
    void removeCurrentLine();
    void removeLineTail();
    void movePosition(QTextCursor::MoveOperation, MoveMode, int n=1);
    void clearUndoRedoStacks();
    bool isModified() const;
    QString selectedText() const;
    void removeSelectedBlock();
    void emitCompilationRequest();

    void evaluateCommand(const KeyCommand & command);


signals:
    void positionChanged(int row, int col);
    void lineAndTextChanged(const QStack<Shared::ChangeTextTransaction> & changes);
    void updateRequest();
    void updateRequest(int fromLine, int toLine);


protected:

    bool forceCompileRequest() const;
    void addLineToRemove(int no);
    void addLineToNew(int no);
    void pushTransaction();
    void timerEvent(QTimerEvent *e);
    void emitPositionChanged();
    class TextDocument * m_document;
    class Clipboard * m_clipboard;
    QStack<Shared::ChangeTextTransaction> l_changes;
    QSet<int> l_remLines;
    QSet<int> l_nLines;
    EditMode e_mode;
    ViewMode e_viewMode;
    int i_timerId;
    bool b_enabled;
    bool b_visible;
    int i_row;
    int i_column;
    int i_prevRow;
    int i_prevCol;
    QRect rect_selection;
    bool b_emitCompilationBlocked;

};

} // namespace Editor

#endif // TEXTCURSOR_H
