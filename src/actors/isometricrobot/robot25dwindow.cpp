#include "robot25dwindow.h"

#include <QSettings>
#include <QScrollBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QMouseEvent>
#include <QScriptEngine>

Robot25DWindow::Robot25DWindow(
	Robot25D::RobotModel *model,
	const QDir &imagesDir,
	QWidget *parent
) :
	QGraphicsView(parent)
{
	_model = model;
	setAttribute(Qt::WA_Hover);
	setMouseTracking(true);
	_mousePressPosition = QPoint(-1, -1);
	setScene(new QGraphicsScene);
	const QString resPath = imagesDir.absolutePath();
	QImage bgImg = QImage(resPath + "/grass_0.png");
	QBrush bgBrush = QBrush(bgImg);
	setBackgroundBrush(bgBrush);
	setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	_robotView = new Robot25D::RobotView(_model, imagesDir);
	scene()->addItem(_robotView);

	loadGame(resPath + "/default.pm.json");
	setWindowTitle(tr("Isometric Robot"));
}


QSize Robot25DWindow::sizeHint() const
{
	return QGraphicsView::sizeHint() + QSize(30, 40);
}

void Robot25DWindow::mousePressEvent(QMouseEvent *event)
{
	const QScrollBar *vert = verticalScrollBar();
	const QScrollBar *horiz = horizontalScrollBar();
	bool scrollable = vert->maximum() + horiz->maximum() > 0;
	if (scrollable) {
		if (event->button() == Qt::LeftButton) {
			setCursor(Qt::ClosedHandCursor);
			_mousePressPosition = event->pos();
		} else {
			setCursor(Qt::OpenHandCursor);
		}
		event->accept();
	} else {
		setCursor(Qt::ArrowCursor);
		event->ignore();
	}
}

void Robot25DWindow::mouseReleaseEvent(QMouseEvent *event)
{
	const QScrollBar *vert = verticalScrollBar();
	const QScrollBar *horiz = horizontalScrollBar();
	bool scrollable = vert->maximum() + horiz->maximum() > 0;
	if (scrollable) {
		_mousePressPosition = QPoint(-1, -1);
		setCursor(Qt::OpenHandCursor);
		event->accept();
	} else {
		setCursor(Qt::ArrowCursor);
		event->ignore();
	}
}

void Robot25DWindow::mouseMoveEvent(QMouseEvent *event)
{
	QScrollBar *vert = verticalScrollBar();
	QScrollBar *horiz = horizontalScrollBar();
	bool scrollable = vert->maximum() + horiz->maximum() > 0;
	if (scrollable) {
		if (_mousePressPosition != QPoint(-1, -1)) {
			setCursor(Qt::ClosedHandCursor);
			const QPoint diff = event->pos() - _mousePressPosition;
			const int dx = diff.x();
			const int dy = diff.y();
			if (dx != 0) {
				horiz->setValue(horiz->value() - dx);
			}
			if (dy != 0) {
				vert->setValue(vert->value() - dy);
			}
			_mousePressPosition = event->pos();
		}
		event->accept();
	} else {
		setCursor(Qt::ArrowCursor);
		event->ignore();
	}
}

void Robot25DWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
	QScrollBar *vert = verticalScrollBar();
	QScrollBar *horiz = horizontalScrollBar();
	bool scrollable = vert->maximum() + horiz->maximum() > 0;
	if (scrollable) {
		emit resizeRequest(sizeHint());
		event->accept();
	} else {
		event->ignore();
	}
}

void Robot25DWindow::wheelEvent(QWheelEvent *event)
{
	event->ignore();
}

Robot25DWindow::~Robot25DWindow()
{

}

void Robot25DWindow::loadGame(const QString &fileName)
{
	QFile f(fileName);
	if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QString script = "a = ";
		script += QString::fromUtf8(f.readAll());
		f.close();
		Schema::Game g;
		QScriptEngine engine;
		QScriptValue v = engine.evaluate(script);
		if (Schema::parceJSON(v, g)) {
			_game = g;
			setTaskIndex(g.index);
		}
	}
}


void Robot25DWindow::loadEnvironment(const QString &fileName)
{
	QFile f(fileName);
	if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
		const QString content = QString::fromUtf8(f.readAll());
		f.close();
		Schema::Game g;
		Schema::Task oneTask;
		if (Schema::parceJSON(content, oneTask.environment)) {
			g.tasks.append(oneTask);
			_game = g;
			setTaskIndex(0);
		} else {
			const QString message =
				QString::fromUtf8("Невозможно загрузить %1: это не обстановка Вертуна").arg(QFileInfo(fileName).fileName());
			QMessageBox::critical(this, tr("Can't load environment"), message);
		}
	} else {
		const QString message =
			QString::fromUtf8("Невозможно загрузить %1: файл не читается").arg(QFileInfo(fileName).fileName());
		QMessageBox::critical(this, tr("Can't load environment"), message);
	}
}

void Robot25DWindow::handleLoadAction()
{
	QSettings s;
	QString lastDir = s.value("Robot25D/LastDir", QDir::currentPath()).toString();
	const QString fileName = QFileDialog::getOpenFileName(this
			, tr("Load environment")
			, lastDir
			, tr("Environments") + QString::fromLatin1(" (*.env.json)")
		);
	if (!fileName.isEmpty()) {
		s.setValue("Robot25D/LastDir", fileName);
		loadEnvironment(fileName);
	}
}



void Robot25DWindow::handleNextAction()
{
	setTaskIndex(_game.index + 1);
}

void Robot25DWindow::handlePrevAction()
{
	setTaskIndex(_game.index - 1);
}

void Robot25DWindow::setTaskIndex(int index)
{
	index = qMax(0, qMin(_game.tasks.size() - 1, index));
	_model->loadEnvironment(_game.tasks[index].environment);
	_game.index = index;
}
