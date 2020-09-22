#ifndef WIDGETS_UTF8TEXTBROWSER_H
#define WIDGETS_UTF8TEXTBROWSER_H

#include <QtGlobal>
#include <QTextBrowser>

#ifdef WIDGETS_LIBRARY
#define WIDGETS_EXPORT Q_DECL_EXPORT
#else
#define WIDGETS_EXPORT Q_DECL_IMPORT
#endif

namespace Widgets
{

class WIDGETS_EXPORT Utf8TextBrowser : public QTextBrowser
{
public:
	Utf8TextBrowser(QWidget *parent = 0) : QTextBrowser(parent) {}
	virtual QVariant loadResource(int type, const QUrl &name);
	virtual void setSource(const QUrl &name);
};

} // namespace Widgets

#endif // WIDGETS_UTF8TEXTBROWSER_H
