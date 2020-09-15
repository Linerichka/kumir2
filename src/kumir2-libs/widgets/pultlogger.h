//
//  pultlogger.h
//  Kumir2
//
//  Created by Denis Khachko on 22.05.14.
//  Copyright (c) 2014 NIISI. All rights reserved.
//

#ifndef Kumir2_pultlogger_h
#define Kumir2_pultlogger_h

#include <QWidget>
class QLabel;
class QFrame;
class QDir;

#ifdef WIDGETS_LIBRARY
#define WIDGETS_EXPORT Q_DECL_EXPORT
#else
#define WIDGETS_EXPORT Q_DECL_IMPORT
#endif


class  loggerButton : public QWidget
{
	Q_OBJECT
public:
	/**
	 * Конструктор
	 * @param parent ссыка на объект-владелец
	 *
	 */
	loggerButton(const QDir &dir, QWidget *parent = 0);
	~loggerButton() {};

	void upArrowType(bool b)
	{
		isUpArrow = b;
	}

	void loadButtons(const QDir &dir);

signals:
	void pressed();

protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

private:
	int posX, posY;
	bool isUpArrow;
	QImage buttonImageUp, buttonImageDown;
	bool downFlag;
	QWidget *Parent;
	QVector<QLine> upArrow, downArrow;
};


class logLine
{
public:
	logLine(
		QString KumCommand,
		QString LogCommand,
		QString React, QFrame *frame, QFrame *respFrame, uint pos
	);

	void moveUp();
	void moveDown();

	int pos() const;
	void removeLabels();

	inline QString KumCommand() const
	{
		return kumCommand;
	}

private:
	QString kumCommand;
	QString logCommand;
	QString react;
	QLabel *textLabel;
	QLabel *respLabel;
};

class WIDGETS_EXPORT pultLogger : public QWidget
{
	Q_OBJECT
public:
	/**
	 * Конструктор
	 * @param parent ссыка на объект-владелец
	 * @param fl флаги окна
	 */
	pultLogger(const QDir &dir, QWidget *parent = 0);
	~pultLogger();

	void setSizes(uint w, uint h);
	void Move(uint x, uint y);

	void Show();

	void appendText(QString kumCommand, QString text, QString replay);
	QString log();

public slots:
	void upBtnPressed();
	void downBtnPressed();
	void ClearLog();
	void CopyLog();

private:
	QFrame *mainFrame;
	QFrame *dummyFrame;
	QFrame *respFrame;
	int W, H;
	int pos;
	QList<logLine> lines;
	int buttonSize;
	loggerButton *downButton;
	loggerButton *upButton;
};

class  WIDGETS_EXPORT linkLight: public QWidget
{
	Q_OBJECT
public:
	/**
	     * Конструктор
	     * @param parent ссыка на объект-владелец
	     *
	     */
	linkLight(QWidget *parent = 0);
	~linkLight() {};

	void setLink(bool b)
	{
		onLine = b;
	}

	bool link() const
	{
		return onLine;
	}

	QString text;

signals:

protected:
	void paintEvent(QPaintEvent *event);

private:
	int posX, posY;
	bool onLine;
};


class WIDGETS_EXPORT MainButton : public QWidget
{
	Q_OBJECT
public:
	/**
	     * Конструктор
	     * @param parent ссыка на объект-владелец
	     *
	     */
	MainButton(const QDir &dir, QWidget *parent = 0);
	/**
	     * Деструктор
	     */
	~MainButton() {};

	void setDirection(int d)
	{
		direction = d;
	}

	void setText(QString t);

	bool isChecked() const
	{
		return checked;
	}

	void setCheckable(bool flag)
	{
		Q_UNUSED(flag);
		checkable = true;
	}

	void setChecked(bool flag)
	{
		checked = flag;
		downFlag = flag;
		repaint();
	}

	bool loadIcon(QString icon);

	void setIconOffset(int value)
	{
		iconoffs = value;
	}

	void setQmode(bool mode)
	{
		qmode = mode;
		repaint();
	}

	bool Qmode() const
	{
		return  qmode;
	}

	void setQu(bool qu)
	{
		SetQu = qu;
	}

	void setQPos(QPoint pos)
	{
		posQ = pos;
		SetQu = true;
	}

signals:
	void pressed();
	void clicked();

protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);

private:
	void drawAddons(QPainter *painter);
	int posX, posY, iconoffs;
	uint direction;
	QImage buttonImageUp, buttonImageDown, buttonIcon;
	bool downFlag, checked, checkable, mouseOver, icon, qmode, SetQu;
	QWidget *Parent;
	QVector<QLine> upArrow, downArrow, leftArrow, rightArrow;
	QString text;
	QPoint posQ;
};

#endif
