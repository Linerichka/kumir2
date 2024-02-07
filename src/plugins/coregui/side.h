#ifndef COREGUI_ROW_H
#define COREGUI_ROW_H

#include <kumir2-libs/extensionsystem/settings.h>

#include <QSplitter>

namespace CoreGUI
{

class Side : public QSplitter
{
	Q_OBJECT
public:
	explicit Side(QWidget *parent,
		const QString &settingsKey
	);
	void addComponent(QWidget *widget, bool autoResizable);
	void updateSettings(ExtensionSystem::SettingsPtr settings, const QStringList &keys);
	void save();
	void restore();
	QSize sizeHint() const;
	QSize minimumSizeHint() const;

signals:
	void visiblityRequest();

private slots:
	void handleVisiblityRequest(bool visible, const QSize &size);
	void forceResizeItem(const QSize &sz);

private /*methods*/:
	void resizeEvent(QResizeEvent *event);
	void ensureEnoughtSpaceForComponent(QWidget *component, const QSize &size);
	void releaseSpaceUsesByComponent(QWidget *component);
	void increaseSize(int diff, QList<int> &szs);
	void decreaseSize(int diff, QList<int> &szs);

private /*fields*/:
	ExtensionSystem::SettingsPtr settings_;
	QString settingsKey_;
	QList<bool> autoResizable_;
};

} // namespace CoreGUI

#endif // COREGUI_ROW_H
