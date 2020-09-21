#ifndef TEXTBLOCKDATA_H
#define TEXTBLOCKDATA_H

#include <QtCore>
#include <QColor>
class QUndoCommand;
class QUndoStack;

#include <kumir2/analizerinterface.h>
#include <kumir2/editor_instanceinterface.h>
#include <kumir2-libs/extensionsystem/settings.h>

namespace Editor
{

using Shared::AnalizerInterface;

struct TextLine {
	explicit TextLine()
	{
		indentStart = indentEnd = 0;
		lineEndSelected = false;
		changed = false;
		inserted = false;
		protecteed = false;
		hidden = false;
		multipleStatementsInLine = false;
		margin.lineEndSelected = false;
		hasBreakpoint = false;
	}
	int indentStart;
	int indentEnd;
	QList<Shared::LexemType> highlight;
	QList<bool> selected;
	bool lineEndSelected;
	bool protecteed;
	bool hidden;

	QString text;

	struct Margin {
		QStringList errors;
		QString text;
		QColor color;
		QList<bool> selected;
		bool lineEndSelected;
	} margin;

	Shared::Editor::Breakpoint breakpoint;
	bool hasBreakpoint;


	bool changed;
	bool inserted;
	bool multipleStatementsInLine;
};

class TextDocument
	: public QObject
{
	Q_OBJECT
	friend class InsertCommand;
	friend class RemoveCommand;
	friend class InsertBlockCommand;
	friend class RemoveBlockCommand;
	friend class ToggleCommentCommand;
	friend class TextCursor;
	friend class InsertImportCommand;
public:
	// Flag, set on restore session.
	// If present, do not actually undo/redo
	// while pushing actions into undo-stack.
	// Becomes false after session restore done.
	static bool noUndoRedo;
	TextLine &at(int index)
	{
		return data_[index];
	}
	const TextLine &at(int index) const
	{
		return data_[index];
	}

	explicit TextDocument(class EditorInstance *parent);

	uint indentAt(uint lineNo) const;
	bool isProtected(uint lineNo) const
	{
		return lineNo < uint(data_.size())
			? data_[lineNo].protecteed : false;
	}
	void setProtected(int lineNo, bool v)
	{
		data_[lineNo].protecteed = v;
	}
	bool isHidden(uint lineNo) const
	{
		return lineNo < uint(data_.size())
			? data_[lineNo].hidden : false;
	}
	void setHidden(int lineNo, bool v)
	{
		data_[lineNo].hidden = v;
	}
	int hiddenLineStart() const;
	uint linesCount() const
	{
		return uint(data_.size());
	}
	Shared::Analizer::SourceFileInterface::Data toKumFile() const;
	QString toHtml(int fromLine = -1, int toLine = -1) const;
	QString lineToHtml(int lineNo) const;
	QByteArray toRtf(uint fromLine, uint toLine) const;
	void setKumFile(const Shared::Analizer::SourceFileInterface::Data &data_, bool showHiddenLines);
	void setPlainText(const QString &data_);
	const QString &textAt(uint index) const
	{
		if (index < uint(data_.size())) {
			return data_.at(index).text;
		} else {
			static const QString dummyString;
			return dummyString;
		}
	}
	const TextLine::Margin &marginAt(uint index) const;
	TextLine::Margin &marginAt(uint index);

	const QList<bool> &selectionMaskAt(uint index) const
	{
		if (index < uint(data_.size())) {
			return data_.at(index).selected;
		} else {
			static const QList<bool> dummySelectionMask;
			return dummySelectionMask;
		}
	}
	void setSelectionMaskAt(int index, const QList<bool> mask)
	{
		if (index >= 0 && index < data_.size()) {
			data_[index].selected = mask;
		}
	}
	bool lineEndSelectedAt(int index) const
	{
		return index >= 0 && index < data_.size() ? data_[index].lineEndSelected : false;
	}
	const QList<Shared::LexemType> &highlightAt(uint index) const
	{
		if (index < uint(data_.size())) {
			return data_.at(index).highlight;
		} else {
			static const QList<Shared::LexemType> dummyHighlight;
			return dummyHighlight;
		}
	}
	void setIndentRankAt(int index, const QPoint &rank)
	{
		if (index >= 0 && index < data_.size()) {
			data_[index].indentStart = rank.x();
		}
		data_[index].indentEnd = rank.y();
	}
	void setHighlightAt(int index, const QList<Shared::LexemType> &highlight)
	{
		if (index >= 0 && index < data_.size()) {
			data_[index].highlight = highlight;
		}
	}

	void setSelected(int line, int pos, bool v)
	{
		if (line < data_.size()) {
			data_[line].selected[pos] = v;
		}
	}
	void setEndOfLineSelected(int line, bool v)
	{
		if (line < data_.size()) {
			data_[line].lineEndSelected = v;
		}
	}
	void evaluateCommand(const QUndoCommand &cmd);
	const QUndoStack *undoStack() const
	{
		return undoStack_;
	}
	QUndoStack *undoStack()
	{
		return undoStack_;
	}
	void removeSelection();
	void forceCompleteRecompilation(const QPoint &cursorPosition);
	void checkForCompilationRequest(const QPoint &cursorPosition);

signals:
	void completeCompilationRequest(const QStringList &visibleText,
		const QStringList &hiddenText,
		int hiddenBaseLine);
protected:
	void insertText(const QString &text, const Shared::Analizer::InstanceInterface *analizer, int line, int pos, int &blankLines, int &blankChars);
	void removeText(QString &removedText, const Shared::Analizer::InstanceInterface *analizer, int line, int pos, int  blankLines, int  blankChars, int count);
	void insertLine(const QString &text, const uint beforeLineNo);
	void removeLine(const uint lineNo);
private:
	class EditorInstance *editor_;
	QSet<int> removedLines_;
	QPoint lastCursorPos_;
	QUndoStack *undoStack_;
	QList<TextLine> data_;
	QString hiddenText_;
	bool wasHiddenTextFlag_;
	AnalizerInterface::SyntaxHighlightBehaviour _syntaxHighlightBehaviour;
};

}

#endif // TEXTBLOCKDATA_H
