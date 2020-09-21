#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QObject>
#include <QStringList>

namespace Editor
{

struct ClipboardData {
	enum Type { Invalid, Text, Block } type;
	QString text;
	QByteArray rtf;
	QStringList block;
};

class Clipboard : public QObject
{
	Q_OBJECT
public:
	static QString BlockMimeType;
	static Clipboard *instance();
public slots:
	void push(const ClipboardData &data);
	void select(int index);
	bool hasContent() const;
	ClipboardData content() const;
	void clear();
	int entriesCount() const;
signals:
	void bufferEntriesCountChanged(int c);
private slots:
	void checkForChanged();
private:
	explicit Clipboard();
	QList<ClipboardData> data_;
	int selection_;

};

} // namespace Editor

#endif // CLIPBOARD_H
