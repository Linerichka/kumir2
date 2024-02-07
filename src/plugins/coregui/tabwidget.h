#ifndef COREGUI_TABWIDGET_H
#define COREGUI_TABWIDGET_H

#include <QTabWidget>

namespace CoreGUI
{

class TabWidget : public QTabWidget
{
	Q_OBJECT
public:
	explicit TabWidget(QWidget *parent = 0);
	void disableTabs();
	QSize minimumSizeHint() const /*override*/;
	void setFont(const QFont &font);
protected:
	void paintEvent(QPaintEvent *);
	void customizeStyle();
};

} // namespace CoreGUI

#endif // COREGUI_TABWIDGET_H
