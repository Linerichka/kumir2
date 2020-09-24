#ifndef EDITCOMMANDS_H
#define EDITCOMMANDS_H

#include <kumir2/analizerinterface.h>
#include <kumir2/analizer_sourcefileinterface.h>
#include <QRect>
#include <QUndoCommand>

namespace Editor
{
	class TextDocument;
	class TextCursor;

class InsertCommand: public QUndoCommand
{
	friend QDataStream &operator<< (QDataStream &stream, const InsertCommand &command);
	friend QDataStream &operator>> (QDataStream &stream, InsertCommand &command);
public:
	explicit InsertCommand(
		TextDocument *doc,
		TextCursor *cursor,
		Shared::Analizer::InstanceInterface *analizer,
		int line,
		int pos,
		const QString &text
	);
	explicit InsertCommand(
		TextDocument *doc,
		TextCursor *cursor,
		Shared::Analizer::InstanceInterface *analizer
	);
	void redo();
	void undo();
	int id() const
	{
		return 1;
	}
	bool mergeWith(const QUndoCommand *other);

private:
	TextDocument *doc;
	TextCursor *cursor;
	Shared::Analizer::InstanceInterface *analizer;
	int line;
	int pos;
	QString text;
	int blankLines;
	int blankChars;
	int cursorRow;
	int cursorCol;

};

class RemoveCommand: public QUndoCommand
{
	friend QDataStream &operator<< (QDataStream &stream, const RemoveCommand &command);
	friend QDataStream &operator>> (QDataStream &stream, RemoveCommand &command);
public:
	explicit RemoveCommand(
		TextDocument *doc,
		TextCursor *cursor,
		Shared::Analizer::InstanceInterface *analizer,
		int line,
		int pos,
		int count,
		bool keepCursor,
		int newCursorRow,
		int newCursorColumn
	);
	explicit RemoveCommand(
		TextDocument *doc,
		TextCursor *cursor,
		Shared::Analizer::InstanceInterface *analizer
	);
	void redo();
	void undo();
	int id() const
	{
		return 2;
	}
	bool mergeWith(const QUndoCommand *other);
private:
	TextDocument *doc;
	TextCursor *cursor;
	Shared::Analizer::InstanceInterface *analizer;
	int line;
	int pos;
	int count;
	bool keepKursor;
	QString removedText;
	int insertedSpaces;
	int cursorRow;
	int cursorCol;
	int cursorRowAfter;
	int cursorColAfter;
};

class InsertBlockCommand: public QUndoCommand
{
	friend QDataStream &operator<< (QDataStream &stream, const InsertBlockCommand &command);
	friend QDataStream &operator>> (QDataStream &stream, InsertBlockCommand &command);
public:
	explicit InsertBlockCommand(
		TextDocument *doc,
		TextCursor *cursor,
		Shared::Analizer::InstanceInterface *analizer,
		int row,
		int column,
		const QStringList &block);
	explicit InsertBlockCommand(
		TextDocument *doc,
		TextCursor *cursor,
		Shared::Analizer::InstanceInterface *analizer
	);
	void redo();
	void undo();
	int id() const
	{
		return 3;
	}

private:
	TextDocument *doc;
	TextCursor *cursor;
	Shared::Analizer::InstanceInterface *analizer;
	int row;
	int column;
	QStringList block;
	int cursorRow;
	int cursorCol;
	QStringList previousLines;
	int addedLines;
};

class RemoveBlockCommand: public QUndoCommand
{
	friend QDataStream &operator<< (QDataStream &stream, const RemoveBlockCommand &command);
	friend QDataStream &operator>> (QDataStream &stream, RemoveBlockCommand &command);
public:
	explicit RemoveBlockCommand(
		TextDocument *doc,
		TextCursor *cursor,
		Shared::Analizer::InstanceInterface *analizer,
		const QRect &block
	);
	explicit RemoveBlockCommand(
		TextDocument *doc,
		TextCursor *cursor,
		Shared::Analizer::InstanceInterface *analizer
	);
	void redo();
	void undo();
	int id() const
	{
		return 4;
	}

private:
	TextDocument *doc;
	TextCursor *cursor;
	Shared::Analizer::InstanceInterface *analizer;
	QRect block;
	int cursorRow;
	int cursorCol;
	QStringList previousLines;
};

class InsertImportCommand : public QUndoCommand
{
	friend QDataStream &operator<< (QDataStream &, const InsertImportCommand &);
	friend QDataStream &operator>> (QDataStream &, InsertImportCommand &);
public:
	explicit InsertImportCommand(
		TextDocument *document,
		TextCursor *cursor,
		Shared::Analizer::InstanceInterface *analizer,
		const QString &importName
	);

	void redo();
	void undo();
	int id() const
	{
		return 5;
	}

private:
	TextDocument *document_;
	TextCursor *cursor_;
	Shared::Analizer::InstanceInterface *analizer_;
	QString importName_;
	uint lineNo_;
};

class ToggleLineProtectedCommand: public QUndoCommand
{
	friend QDataStream &operator<< (QDataStream &stream, const ToggleLineProtectedCommand &command);
	friend QDataStream &operator>> (QDataStream &stream, ToggleLineProtectedCommand &command);
public:
	explicit ToggleLineProtectedCommand(TextDocument *Doc, int lineNo);
	void redo();
	void undo();
	int id() const
	{
		return 0xA0;
	}
private:
	TextDocument *doc;
	int lineNo;

};

class ChangeHiddenLineDelimeterCommand: public QUndoCommand
{
	friend QDataStream &operator<< (QDataStream &stream, const ChangeHiddenLineDelimeterCommand &command);
	friend QDataStream &operator>> (QDataStream &stream, ChangeHiddenLineDelimeterCommand &command);
public:
	explicit ChangeHiddenLineDelimeterCommand(
		TextDocument *doc,
		int firstHiddenLineNo
	);
	void redo();
	void undo();
private:
	TextDocument *doc;
	Shared::Analizer::SFData prevData;
	int firstHiddenLineNo;
};

class ToggleCommentCommand: public QUndoCommand
{
	friend QDataStream &operator<< (QDataStream &stream, const ToggleCommentCommand &command);
	friend QDataStream &operator>> (QDataStream &stream, ToggleCommentCommand &command);
public:
	explicit ToggleCommentCommand(
		TextDocument *Doc,
		uint FromLineInclusive,
		uint ToLineInclusive,
		TextCursor *cursor,
		Shared::Analizer::InstanceInterface *analizer
	);
	void redo();
	void undo();
	int id() const
	{
		return 0xB0;
	}
private:
	TextDocument *doc;
	TextCursor *cursor;
	Shared::Analizer::InstanceInterface *analizer;
	uint fromLineInclusive;
	uint toLineInclusive;
	QSet<int> commentedLines;
	QSet< QPair<int, int> > uncommentedLines;
};

}

#endif // EDITCOMMANDS_H
