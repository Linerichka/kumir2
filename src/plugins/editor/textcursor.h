#ifndef TEXTCURSOR_H
#define TEXTCURSOR_H

#include "keycommand.h"
#include "macro.h"
#include <kumir2/analizerinterface.h>
#include <kumir2-libs/extensionsystem/settings.h>

#include <QTextCursor>

using namespace Shared;

namespace Editor
{

class TextCursor : public QObject
{
	Q_OBJECT
public:
	enum EditMode { EM_Insert, EM_Overwrite };
	enum MoveMode { MM_Move, MM_Select, MM_RectSelect };
	enum ViewMode { VM_Blinking, VM_Hidden, VM_Visible };
	explicit TextCursor(class EditorInstance *editor);

	~TextCursor();
	uint row() const
	{
		return row_;
	}
	uint column() const
	{
		return column_;
	}
	void setRow(uint v)
	{
		row_ = v;
		emit updateRequest();
	}
	void setColumn(uint v)
	{
		column_ = v;
		emit updateRequest();
	}
	void setTeacherMode(bool v)
	{
		teacherModeFlag_ = v;
	}
	void moveTo(int row, int col);
	void selectRangeBlock(int fromRow, int fromCol, int toRow, int toCol);
	void selectRangeText(int fromRow, int fromCol, int toRow, int toCol);
	EditMode mode() const
	{
		return editMode_;
	}
	void setMode(EditMode m)
	{
		editMode_ = m;
		emit updateRequest();
	}
	bool isEnabled() const
	{
		return enabledFlag_;
	}
	bool isVisible() const
	{
		return enabledFlag_ && visibleFlag_;
	}
	void setViewMode(ViewMode mode);
	void setEnabled(bool v);
	bool hasSelection() const;
	bool hasRectSelection() const
	{
		return selectionRect_.x() != -1 && selectionRect_.y() != -1;
	}
	QRect selectionRect() const
	{
		return selectionRect_;
	}
	void selectionBounds(int &fromRow, int &fromCol, int &toRow, int &toCol) const;
	QStringList rectSelectionText() const;
	void removeRectSelection();
	void insertText(const QString &text);
	void insertImport(const QString &importableName);
	void insertBlock(const QStringList &block);
	void removePreviousChar();
	void removeCurrentChar();
	void removeSelection();
	void removeSelectedText();
	void removeCurrentLine();
	void removeLineTail();
	void movePosition(QTextCursor::MoveOperation, MoveMode, int n = 1);

	QString selectedText() const;
	void removeSelectedBlock();
	void evaluateCommand(const KeyCommand &command);

	bool isFreeCursorMovement() const;
	bool modifiesProtectedLiines() const;
	void changeSelectionToExcludeProtectedLines();

	static void normalizePlainText(QString &s);

public slots:
	void toggleComment();
	void toggleLock();
	void undo();
	void redo();
	void handleUndoChanged(bool v);
	void handleRedoChanged(bool v);
	void startRecordMacro();
	QSharedPointer<Macro> endRecordMacro();


signals:
	void positionChanged(int row, int col);
	void updateRequest();
	void updateRequest(int fromLine, int toLine);
	void undoAvailable(bool);
	void redoAvailable(bool);
	void signalizeNotEditable();


protected:

	class EditorInstance *editor_;

	int justifyLeft(const QString &text) const;

	void findLexemBound(uint &row,  uint &column, const qint8 dir) const;

	void timerEvent(QTimerEvent *e);
	void emitPositionChanged();
	EditMode editMode_;
	ViewMode viewMode_;
	int blinkTimerId_;
	bool enabledFlag_;
	bool visibleFlag_;
	uint row_;
	uint column_;
	int keptColumn_;
	bool teacherModeFlag_;

	QRect selectionRect_;
	QSharedPointer<Macro> recordingMacro_;

};

} // namespace Editor

#endif // TEXTCURSOR_H
