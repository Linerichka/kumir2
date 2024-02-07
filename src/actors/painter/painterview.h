#ifndef PAINTERVIEW_H
#define PAINTERVIEW_H

#include <QWidget>
class QImage;
class QMutex;

namespace ActorPainter
{

class PainterView : public QWidget
{
	Q_OBJECT
public:
	explicit PainterView(QWidget *parent = 0);
	void setCanvasSize(const QSize &size);
	void setCanvasData(QImage data); // not a reference to avoid TLS problems
	QImage *canvas();
	void setZoom(qreal v);
	qreal zoom() const
	{
		return r_zoom;
	}

signals:
	void cursorOver(int x, int y, const QColor &color);

protected:
	void updateSizeFromCanvas();
	void paintEvent(QPaintEvent *event);
	void mouseMoveEvent(QMouseEvent *event);


private:
	QImage *m_canvas;
	QMutex *m_locker;
	qreal r_zoom;
};

}

#endif // PAINTERVIEW_H
