/*
This file is generated, but you can safely change it
until you run "gen_actor_source.py" with "--project" flag.

Generated file is just a skeleton for module contents.
You should change it corresponding to functionality.
*/

#ifndef DRAWMODULE_H
#define DRAWMODULE_H

// Base class include
#include "drawmodulebase.h"
#include <QDebug>
#include <QLabel>
#include <QGraphicsView>
#include <QGraphicsLineItem>
class QToolButton;

class DrawNavigator;
namespace ActorDraw
{

class DrawModule;

class DrawView : public QGraphicsView
{
	Q_OBJECT
public:
	DrawView(QWidget *parent = 0) : QGraphicsView(parent)
	{
		c_scale = 1;
		pressed = false;
		press_pos = QPoint();
		firstResize = true;
		net = true;
		smallNetLabel = new QLabel(this);
		smallNetLabel->hide();
		smallNetLabel->setText(trUtf8("Слишком мелкая сетка"));
	};
	void setDraw(DrawModule *draw, QMutex *mutex)
	{
		DRAW = draw;
		dr_mutex = mutex;
	};
	double zoom()const
	{
		return c_scale;
	};
	void setZoom(double zoom);
	void setNet();//RESIZE NET
	bool isNet() const
	{
		return net;

	}
	void forceRedraw()
	{
		// horizontalScrollBar()->setValue(horizontalScrollBar()->value() +1);
		//  horizontalScrollBar()->setValue(horizontalScrollBar()->value()-1);
		qDebug() << "ForceREDDR";
		QGraphicsView::resetCachedContent();
		QGraphicsView::update();
		QGraphicsView::repaint();


		// verticalScrollBar()->setValue(horizontalScrollBar()->value() +1);
		//  verticalScrollBar()->setValue(horizontalScrollBar()->value()-1);

	}

protected:
	// void scrollContentsBy ( int dx, int dy );
	void resizeEvent(QResizeEvent *event);
	void wheelEvent(QWheelEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	//    void paintEvent(QPaintEvent *event);

private:
	DrawModule *DRAW;
	double c_scale;
	bool pressed;
	bool net;
	QPoint press_pos;
	bool firstResize;
	double lastStep;
	QMutex *dr_mutex;
	QLabel *smallNetLabel;

};

class DrawScene : public QGraphicsScene
{
	Q_OBJECT
public:
	DrawScene(QObject *parent = 0): QGraphicsScene(parent)
	{
		///  installEventFilter(this);
	};
	void drawNet(double startx, double endx, double starty, double endy, QColor color, const double step, const double stepY, bool net, qreal nw, qreal aw);
	void setDraw(DrawModule *draw, QMutex *mutex)
	{
		DRAW = draw;
		dr_mutex = mutex;
	};

	void addDrawLine(QLineF lineF, QColor color, qreal width);
	void reset()
	{
		for (int i = 0; i < lines.count(); i++) {
			removeItem(lines.at(i));
		}
		lines.clear();
		for (int i = 0; i < texts.count(); i++) {
			removeItem(texts.at(i));
		}
		texts.clear();
		clearBuffer();
	}
	void upd()
	{
		QGraphicsScene::update();
	}
	void DestroyNet();
	void drawOnlyAxis(double startx, double endx, double starty, double endy, qreal aw);
	bool isLineAt(const QPointF &pos, qreal radius);
	qreal drawText(const QString &Text, qreal widthChar, QPointF from, QColor color); //Returns offset of pen.
	QRectF getRect();
	int saveToFile(const QString &p_FileName);
	int loadFromFile(const QString &p_FileName);
	void fromBufferToScene()
	{
		QGraphicsItemGroup *buff = createItemGroup(itemsBuffer);
		buff->setZValue(90);
		addItem(buff);
		clearBuffer();
	}
	void clearBuffer()
	{
		itemsBuffer.clear();
	}
	int buffSize()
	{
		return itemsBuffer.count();
	}
protected:
	// void resizeEvent ( QResizeEvent * event );
	//  bool eventFilter(QObject *object, QEvent *event);
	// bool event(QEvent * event);
private:
	bool isUserLine(QGraphicsItem *); //Return true if item is user item;
	QList<QGraphicsLineItem *> lines;
	QList<QGraphicsLineItem *> Netlines;
	QList<QGraphicsLineItem *> linesDubl; //Базовый чертеж
	QList<QGraphicsSimpleTextItem *> texts;
	DrawModule *DRAW;
	QList<QGraphicsItem *> itemsBuffer;
	QMutex *dr_mutex;
};

class DrawModule : public DrawModuleBase
{
	Q_OBJECT
public /* methods */:
	DrawModule(ExtensionSystem::KPlugin *parent);
	inline void terminateEvaluation() {}
	static QList<ExtensionSystem::CommandLineParameter> acceptableCommandLineParameters();
	QWidget *mainWidget() const;
	QWidget *pultWidget() const;
	void handleGuiReady();

	bool isAutoNet() const
	{
		return autoNet;
	}
	void setAutoNet(bool state)
	{
		autoNet = state;
	}
	double NetStepX() const
	{
		return netStepX;
	}
	void setNetStepX(double step)
	{
		netStepX = step;
	}
	double NetStepY() const
	{
		return netStepY;
	}
	void setNetStepY(double step)
	{
		netStepY = step;
	}
	double zoom()
	{
		return CurView->zoom();
	}

	QGraphicsPolygonItem *Pen() const
	{
		return mPen;
	}
	void scalePen(double factor)
	{
		mutex.lock();
		mPen->setScale(factor);
		qDebug() << "PenScale" << factor << "mPen->scale" << mPen->scale();
		mutex.unlock();
	}
	DrawView *getCurView() const
	{
		return CurView;
	}
	static ExtensionSystem::SettingsPtr DrawSettings();
	QColor axisColor() const
	{
		return QColor(DrawSettings()->value("AxisColor", "#999900").toString());
	}
	void redrawPicture()
	{
		// CurScene->resetCashedContent();
		CurScene->upd();

		CurView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
		CurView->forceRedraw();

		CurScene->update(CurScene->sceneRect());

		CurView->viewport()->update();
		CurView->setZoom(CurView->zoom() * 2);
		CurView->setZoom(CurView->zoom() * 0.5);
		//  CurView->setViewportUpdateMode (QGraphicsView::NoViewportUpdate);

	}
	QString initialize(const QStringList &configurationParameters, const ExtensionSystem::CommandLine &runtimeParameters);
	QMutex mutex;
public slots:
	void changeGlobalState(ExtensionSystem::GlobalState old, ExtensionSystem::GlobalState current);
	void loadActorData(QIODevice *source);
	void reloadSettings(ExtensionSystem::SettingsPtr settings, const QStringList &keys);
	void reset();
	void setAnimationEnabled(bool enabled);
	void runSetupPen();
	void runReleasePen();
	void runSetPenColor(const Color &color);
	void runMoveTo(const qreal x, const qreal y);
	void runMoveBy(const qreal dX, const qreal dY);
	void runAddCaption(const qreal width, const QString &text);
	bool runIsLineAtCircle(const qreal x, const qreal y, const qreal radius);
	void zoomFullDraw();


	void drawNet();
	void autoNetChange(bool value);
	void netStepChange(double value);
	void zoomIn();
	void zoomOut();
	void zoomNorm();

	void showNavigator(bool state);
	void openFile();
	void saveFile();
	void redraw();
	void updateDraw();

	/* ========= CLASS PRIVATE ========= */
private:
	void createGui();
	void CreatePen(void);

	DrawScene *CurScene;
	DrawView *CurView;
	QGraphicsPolygonItem *mPen;
	double netStepX, netStepY;
	QColor netColor;
	bool autoNet;
	bool penIsDrawing;
	bool firstShow;
	Color penColor;
	ExtensionSystem::GlobalState currentState;
	DrawNavigator *navigator;
	QToolButton *showToolsBut;
	QDir curDir;
	bool animate;
	QTimer *redrawTimer;
	qreal curAngle;
	qreal AncX, AncY;
	QPointF curPos;
};


} // namespace ActorDraw

#endif // DRAWMODULE_H
