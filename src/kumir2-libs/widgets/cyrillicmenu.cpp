#include "cyrillicmenu.h"

#ifdef Q_OS_UNIX
#include <stdlib.h>
#endif

#include <QPainter>
#include <QPaintEvent>
#include <QStyle>
#include <QStyleOption>

namespace Widgets
{

CyrillicMenu::CyrillicMenu(QWidget *parent): QMenu(parent) {}

CyrillicMenu::CyrillicMenu(const QString &title, QWidget *parent): QMenu(title, parent) {}

void CyrillicMenu::paintEvent(QPaintEvent *event)
{
	QPainter p(this);
	QStyleOptionMenuItem menuOpt;
	QRegion emptyArea = QRegion(rect());
	menuOpt.initFrom(this);
	menuOpt.state = QStyle::State_None;
	menuOpt.checkType = QStyleOptionMenuItem::NotCheckable;
	menuOpt.maxIconWidth = 0;
	menuOpt.tabWidth = 0;
	style()->drawPrimitive(QStyle::PE_PanelMenu, &menuOpt, &p, this);
	for (int i = 0; i < actions().count(); ++i) {
		QAction *action = actions().at(i);
		QStyleOptionMenuItem opt;
		initStyleOption(&opt, action);
		opt.rect = actionGeometry(action);
		if (!action->isSeparator()) {
			opt.text = action->text() + "\t";
			if (action->property("fakeShortcut").toString().length() > 0) {
				opt.text += action->property("fakeShortcut").toString();
			} else {
				opt.text += shortcutToText(action->shortcut());
			}
		}
		QRegion adjustedActionReg(opt.rect);
		emptyArea -= adjustedActionReg;
		style()->drawControl(QStyle::CE_MenuItem, &opt, &p, this);

	}

	int fw = style()->pixelMetric(QStyle::PM_MenuPanelWidth, 0, this);
	QRegion borderReg;
	borderReg += QRect(0, 0, fw, height()); //left
	borderReg += QRect(width() - fw, 0, fw, height()); //right
	borderReg += QRect(0, 0, width(), fw); //top
	borderReg += QRect(0, height() - fw, width(), fw); //bottom
	p.setClipRegion(borderReg);
	emptyArea -= borderReg;
	QStyleOptionFrame frame;
	frame.rect = rect();
	frame.palette = palette();
	frame.state = QStyle::State_None;
	frame.lineWidth = style()->pixelMetric(QStyle::PM_MenuPanelWidth);
	frame.midLineWidth = 0;
	style()->drawPrimitive(QStyle::PE_FrameMenu, &frame, &p, this);
	event->accept();
}

/**
 * @brief CyrillicMenu::updateUbuntuShortcutTexts
 * Ubuntu's desktop environment uses global application menu, so
 * customized QMenu event handlers are completely ignored.
 *
 * Solution: add shortcut text as visible action name text for each entry
 */
void CyrillicMenu::updateUbuntuShortcutTexts()
{
	bool isUbuntuSession = false;
#ifdef Q_WS_X11
	const char *sessionName = ::getenv("SESSION");
	isUbuntuSession =
		sessionName && "ubuntu" == QString::fromLatin1(sessionName).toLower();
#endif
	if (!isUbuntuSession) {
		return;
	}
	static const char *FixedPropName = "UbuntuFixed";
	for (int i = 0; i < actions().count(); ++i) {
		QAction *const action = actions().at(i);
		if (!action->isSeparator()) {
			bool ubuntuFixed = action->property(FixedPropName).toBool();
			if (!ubuntuFixed) {
				QString shortcutText;
				if (action->property("fakeShortcut").toString().length() > 0) {
					shortcutText = action->property("fakeShortcut").toString();
				} else {
					shortcutText = shortcutToText(action->shortcut());
				}
				if (shortcutText.length() > 0) {
					action->setText(QString("%1\t(%2)")
						.arg(action->text())
						.arg(shortcutText)
					);
				}
				action->setProperty(FixedPropName, true);
			}
		}
	}
}

QString CyrillicMenu::shortcutToText(const QKeySequence &shortcut)
{
	QStringList keys = shortcut.toString().split(",");
	while (keys.size() > 2) {
		QString a = keys.last();
		keys.pop_back();
		keys.last() += "," + a;
	}
	QStringList newKeys;
	for (int i = 0; i < keys.size(); i++) {
		QString key = keys[i].trimmed();
		if (key.toLower() == "space") {
			newKeys << QString::fromUtf8("Пробел");
		} else if (key.toLower() == "up") {
			newKeys << QString::fromUtf8("Вверх");
		} else if (key.toLower() == "down") {
			newKeys << QString::fromUtf8("Вниз");
		} else if (key.toLower() == "left") {
			newKeys << QString::fromUtf8("Влево");
		} else if (key.toLower() == "right") {
			newKeys << QString::fromUtf8("Вправо");
		} else if (key.length() > 1) {
			newKeys << key;
		} else {
			key = key.toUpper();
			if (key == "Q") {
				key = QString::fromUtf8("Й");
			} else if (key == "W") {
				key = QString::fromUtf8("Ц");
			} else if (key == "E") {
				key = QString::fromUtf8("У");
			} else if (key == "R") {
				key = QString::fromUtf8("К");
			} else if (key == "T") {
				key = QString::fromUtf8("Е");
			} else if (key == "Y") {
				key = QString::fromUtf8("Н");
			} else if (key == "U") {
				key = QString::fromUtf8("Г");
			} else if (key == "I") {
				key = QString::fromUtf8("Ш");
			} else if (key == "O") {
				key = QString::fromUtf8("Щ");
			} else if (key == "P") {
				key = QString::fromUtf8("З");
			} else if (key == "A") {
				key = QString::fromUtf8("Ф");
			} else if (key == "S") {
				key = QString::fromUtf8("Ы");
			} else if (key == "D") {
				key = QString::fromUtf8("В");
			} else if (key == "F") {
				key = QString::fromUtf8("А");
			} else if (key == "G") {
				key = QString::fromUtf8("П");
			} else if (key == "H") {
				key = QString::fromUtf8("Р");
			} else if (key == "J") {
				key = QString::fromUtf8("О");
			} else if (key == "K") {
				key = QString::fromUtf8("Л");
			} else if (key == "L") {
				key = QString::fromUtf8("Д");
			} else if (key == "Z") {
				key = QString::fromUtf8("Я");
			} else if (key == "X") {
				key = QString::fromUtf8("Ч");
			} else if (key == "C") {
				key = QString::fromUtf8("С");
			} else if (key == "V") {
				key = QString::fromUtf8("М");
			} else if (key == "B") {
				key = QString::fromUtf8("И");
			} else if (key == "N") {
				key = QString::fromUtf8("Т");
			} else if (key == "M") {
				key = QString::fromUtf8("Ь");
			} else if (key == "," || key == "<") {
				key = QString::fromUtf8("Б");
			} else if (key == "." || key == ">") {
				key = QString::fromUtf8("Ю");
			} else if (key == "`" || key == "~") {
				key = QString::fromUtf8("Ё");
			} else if (key == "[" || key == "{") {
				key = QString::fromUtf8("Х");
			} else if (key == "]" || key == "}") {
				key = QString::fromUtf8("Ъ");
			}
			newKeys << key;
		}
	}
	return newKeys.join(", ");
}

}
