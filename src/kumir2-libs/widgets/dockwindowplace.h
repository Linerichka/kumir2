#ifndef WIDGETS_DOCKWINDOWPLACE_H
#define WIDGETS_DOCKWINDOWPLACE_H

#include <QtGlobal>
#include <QTabWidget>

namespace ExtensionSystem
{
	class Settings;
	typedef QSharedPointer<Settings> SettingsPtr;
}


#ifdef WIDGETS_LIBRARY
#define WIDGETS_EXPORT Q_DECL_EXPORT
#else
#define WIDGETS_EXPORT Q_DECL_IMPORT
#endif


namespace Widgets
{

class WIDGETS_EXPORT DockWindowPlace : public QTabWidget
{
	friend class DockWindowPlaceImpl;
	friend class SecondaryWindow;
	Q_OBJECT
public:
	explicit DockWindowPlace(QWidget *parent,
		const QString settingsKey);
	QSize minimumSizeHint() const;
	QSize sizeHint() const;
	void updateSettings(ExtensionSystem::SettingsPtr settings);
	void addPersistentWidget(QWidget *widget, const QString &title);
	void saveState();
	void restoreState();
	void setPreferredItemSize(const QSize &size);
public slots:
	void activate(const QSize &prefSize);
	void processResize(const QSize &sz);
signals:
	void visiblityRequest(bool visible, const QSize &size);
	void resizeRequest(const QSize &size);
protected:
	void registerWindowHere(class SecondaryWindow *window);
	void tabInserted(int index);
	void tabRemoved(int);
	void resizeEvent(QResizeEvent *e);
private:
	class DockWindowPlaceImpl *pImpl_;
};

} // namespace Widgets

#endif // WIDGETS_DOCKWINDOWPLACE_H
