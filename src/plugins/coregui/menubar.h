#ifndef MENUBAR_H
#define MENUBAR_H

#include <QMenuBar>
#include <QKeyEvent>
#include <QLabel>

class MenuBar : public QMenuBar
{
	Q_OBJECT
public:
	MenuBar(QWidget *parent) :
		QMenuBar(parent),
		contextMenuWidget_(0)
	{}

	void setContextMenuWidget(QWidget *w)
	{
		contextMenuWidget_ = w;
	}

protected:
	bool eventFilter(QObject *object, QEvent *event)
	{
		bool catched = false;

		if (event->type() == QEvent::KeyRelease) {
			QKeyEvent *kev = static_cast<QKeyEvent *>(event);
			if (kev->key() == Qt::Key_Alt || kev->key() == Qt::Key_Meta) {
				catched = true;
			}
		}
//        if (QEvent::ContextMenu == event->type()) {
//            QContextMenuEvent * e =
//                    static_cast<QContextMenuEvent*>(event);
//            if (contextMenuWidget_ && e) {
//                const QPoint position = e->globalPos();
//                QRect contextRect(position, contextMenuWidget_->size());
//                QDesktopWidget* screen = qApp->desktop();
//                const QRect screenRect = screen->availableGeometry(this);
//                if (contextRect.right() > screenRect.right()) {
//                    contextRect.moveRight(screenRect.right());
//                }
//                if (contextRect.left() < screenRect.left()) {
//                    contextRect.moveLeft(screenRect.left());
//                }
//                contextMenuWidget_->move(contextRect.topLeft());
//                contextMenuWidget_->show();
//                catched = true;
//            }
//        }

		if (catched) {
			return false;
		} else {
			return QMenuBar::eventFilter(object, event);
		}
	}
	QWidget *contextMenuWidget_;
};

#endif // MENUBAR_H
