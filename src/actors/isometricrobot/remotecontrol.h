#ifndef ROBOT25D_REMOTECONTROL_H
#define ROBOT25D_REMOTECONTROL_H

//#include <kumir2-libs/extensionsystem/kplugin.h>

#include <QSvgWidget>
#include <QString>
#include <QMap>
//#include <QMouseEvent>
//#include <QPaintEvent>
#include <QRect>
#include <QRectF>

namespace ExtensionSystem
{
	class KPlugin;
}



inline bool operator<(const QRectF &a, const QRectF &b)
{
	const QString hashA = QString::fromLatin1("%1:%2:%3:%4")
		.arg(a.left()).arg(a.top()).arg(a.width()).arg(a.height());
	const QString hashB = QString::fromLatin1("%1:%2:%3:%4")
		.arg(b.left()).arg(b.top()).arg(b.width()).arg(b.height());
	return hashA < hashB;
}

namespace ActorIsometricRobot
{

class IsometricRobotModule;

class SvgRemoteControl : public QSvgWidget
{
	Q_OBJECT
public:
	explicit SvgRemoteControl(
		ExtensionSystem::KPlugin *plugin,
		IsometricRobotModule *module,
		const QString &rcFileName,
		QWidget *parent
	);

signals:

public slots:
	inline void setLinkEnabled(bool on)
	{
		_linkEnabled = on;
	}

private slots:
	void handleSvgButtonPressed(const QString &svgId);

private:
	ExtensionSystem::KPlugin *plugin_;
	IsometricRobotModule *module_;

	typedef QPair<QString, QString> TextLine;
	typedef QList<TextLine> LineList;

	void appendCommandToLog(const QString &svgId);
	void appendStatusToLog();
	void appendResultToLog(bool result);
	QString commandNameBySvgId(const QString &svgId) const;
	void setupButtons();
	void setupLabels();
	QRect scaleToPixels(const QRectF &points) const;

	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void paintEvent(QPaintEvent *event);
	void paintLogger(QPainter *painter, const QRect &rect);
	int visibleLoggerLinesCount() const;

	bool btnScrollUpEnabled() const;
	bool btnScrollDownEnabled() const;

	QMap<QRectF, QString> _buttons;
	QString _buttonHoverId;
	QString _buttonPressId;
	bool _linkEnabled;
	QRectF _linkOnRect;
	QRectF _linkOffRect;

	LineList _loggerText;
	int _loggerOffset;

signals:
	void buttonPressed(const QString &svgId);
};

} // namespace Robot25D

#endif // ROBOT25D_REMOTECONTROL_H
