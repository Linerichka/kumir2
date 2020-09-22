#ifndef WIDGETS_DECLARATIVESETTINGSPAGE_IMPL_H
#define WIDGETS_DECLARATIVESETTINGSPAGE_IMPL_H

#include "declarativesettingspage.h"
#include <QColor>

class QToolButton;

namespace Widgets
{

class DeclarativeSettingsPage;

class DeclarativeSettingsPageImpl : public QObject
{
	friend DeclarativeSettingsPage;
	Q_OBJECT

private /*methods*/:
	explicit DeclarativeSettingsPageImpl(class DeclarativeSettingsPage *parent);

	static QColor buttonColor(const QToolButton *b);
	static void setButtonColor(QToolButton *b, const QColor &color);

	void init();
	void accept();
	void resetToDefaults();
	void addIntegerField(const QString &key, const DeclarativeSettingsPage::Entry &entry);
	void addColorField(const QString &key, const DeclarativeSettingsPage::Entry &entry);
	void addRealField(const QString &key, const DeclarativeSettingsPage::Entry &entry);
	void addBoolField(const QString &key, const DeclarativeSettingsPage::Entry &entry);
	void addStringField(const QString &key, const DeclarativeSettingsPage::Entry &entry);
	void addChoiceField(const QString &key, const DeclarativeSettingsPage::Entry &entry);
	void addField(const QString &labelText, QWidget *controlWidget);

private slots:
	void showColorDialog();

private /*fields*/:
	DeclarativeSettingsPage *pClass_;
	ExtensionSystem::SettingsPtr settings_;
	QMap<QString, DeclarativeSettingsPage::Entry> entries_;
	QMap<QString, QWidget *> widgets_;
};

} // namespace Widgets

#endif // WIDGETS_DECLARATIVESETTINGSPAGE_IMPL_H
