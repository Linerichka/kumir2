#ifndef EDITOR_INSTANCEINTERFACE_H
#define EDITOR_INSTANCEINTERFACE_H

#include "analizer_sourcefileinterface.h"
#include "analizer_helperinterface.h"

#include <QtPlugin>
#include <QMetaType>
#include <QList>

class QWidget;
class QMenu;
class QAction;

namespace Shared
{

namespace Analizer {
	class InstanceInterface;
}

namespace Editor
{

struct Breakpoint {
	QString fileName;
	bool enabled;
	quint32 lineNo;
	quint32 ignoreCount;
	QString condition;

	explicit Breakpoint(): enabled(true), lineNo(0u), ignoreCount(0u) {}
};



class InstanceInterface
{
public:
	virtual bool isModified() const = 0;
	virtual void setNotModified() = 0;

	virtual void loadDocument(
		QIODevice *device,
		const QString &fileNameSuffix,
		const QString &sourceEncoding,
		const QUrl &sourceUrl, QString *error
	) = 0;

	virtual void loadDocument(
		const QString &fileName, QString *error
	) /* throws QString */ = 0;

	virtual void loadDocument(
		const Analizer::SourceFileInterface::Data &data, QString *error
	) /* throws QString */ = 0;

	virtual void saveDocument(
		const QString &fileName, QString *error
	) /* throws QString */ = 0;
	virtual void saveDocument(
		QIODevice *device, QString *error
	) /* throws QString */ = 0;

	virtual Analizer::SourceFileInterface::Data documentContents() const = 0;

	virtual Analizer::InstanceInterface *analizer() = 0;
	virtual quint32 errorLinesCount() const = 0;
	virtual void ensureAnalized() = 0;

	virtual void highlightLineGreen(int lineNo, quint32 colStart, quint32 colEnd) = 0;
	virtual void highlightLineRed(int lineNo, quint32 colStart, quint32 colEnd) = 0;
	virtual void unhighlightLine() = 0;

	virtual void appendMarginText(int lineNo, const QString &text) = 0;
	virtual void setMarginText(int lineNo, const QString &text, const QColor &fgColor) = 0;
	virtual void clearMarginText(int fromLine = 0, int toLine = -1) = 0;

	virtual bool supportsContextHelp() const = 0;
	virtual Analizer::ApiHelpItem contextHelpItem() const
	{
		return Analizer::ApiHelpItem();
	}

	virtual QWidget *widget() = 0;
	virtual QList<QMenu *> menus() const = 0;
	virtual QList<QAction *> toolBarActions() const = 0;

	virtual QAction *toggleBreakpointAction() const = 0;
	virtual QList<Breakpoint> breakpoints() const = 0;
	virtual quint32 currentLineNumber() const = 0;
	virtual void forceCompleteCompilation() = 0;
};

} // namespace Editor
} // namespace Shared

Q_DECLARE_METATYPE(Shared::Editor::Breakpoint)
Q_DECLARE_INTERFACE(Shared::Editor::InstanceInterface,
	"kumir2.Editor.InstanceInterface")

#endif // EDITOR_INSTANCEINTERFACE_H
