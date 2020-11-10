/*
This file is generated, but you can safely change it
until you run "gen_actor_source.py" with "--project" flag.

Generated file is just a skeleton for module contents.
You should change it corresponding to functionality.
*/

#ifndef KEYBOARDMODULE_H
#define KEYBOARDMODULE_H

// Base class include
#include "keyboardmodulebase.h"

// Kumir includes
#include <kumir2-libs/extensionsystem/kplugin.h>
#include <kumir2-libs/utils/lockedqueue.hpp>

// Qt includes
#include <QDateTime>

namespace ActorKeyboard
{


class KeyboardModule
	: public KeyboardModuleBase
{
	Q_OBJECT
public /* methods */:
	KeyboardModule(ExtensionSystem::KPlugin *parent);
	static QList<ExtensionSystem::CommandLineParameter> acceptableCommandLineParameters();
	QWidget *mainWidget() const;
	QWidget *pultWidget() const;
public slots:
	void changeGlobalState(ExtensionSystem::GlobalState old, ExtensionSystem::GlobalState current);
	void loadActorData(QIODevice *source);
	void reloadSettings(ExtensionSystem::SettingsPtr settings, const QStringList &keys);
	void reset();
	void terminateEvaluation();
	void finalizeRun();
	void setAnimationEnabled(bool enabled);
	bool runKeyHit();
	int runKeyCode();
	void runClearKeyBuffer();
	int runKEY_UP();
	int runKEY_DOWN();
	int runKEY_LEFT();
	int runKEY_RIGHT();
	Keycode runOperatorINPUT(const QString &s, bool &ok);
	QString runOperatorOUTPUT(const Keycode &x);

	Keycode runOperatorASSIGN(const int x);
	int runOperatorASSIGN(const ActorKeyboard::Keycode &x);

	bool runOperatorEQUAL(const ActorKeyboard::Keycode &self, const ActorKeyboard::Keycode &other);
	bool runOperatorEQUAL(const ActorKeyboard::Keycode &self, const int other);
	bool runOperatorEQUAL(const int other, const ActorKeyboard::Keycode &self);

	bool runOperatorNOTEQUAL(const ActorKeyboard::Keycode &self, const ActorKeyboard::Keycode &other);
	bool runOperatorNOTEQUAL(const ActorKeyboard::Keycode &self, const int other);
	bool runOperatorNOTEQUAL(const int other, const ActorKeyboard::Keycode &self);

	bool runOperatorLESS(const ActorKeyboard::Keycode &self, const ActorKeyboard::Keycode &other);
	bool runOperatorLESS(const ActorKeyboard::Keycode &self, const int other);
	bool runOperatorLESS(const int self, const ActorKeyboard::Keycode &other);

	bool runOperatorGREATER(const ActorKeyboard::Keycode &self, const ActorKeyboard::Keycode &other);
	bool runOperatorGREATER(const ActorKeyboard::Keycode &self, const int other);
	bool runOperatorGREATER(const int self, const ActorKeyboard::Keycode &other);

	int runOperatorPLUS(const ActorKeyboard::Keycode &self, const ActorKeyboard::Keycode &other);
	int runOperatorPLUS(const ActorKeyboard::Keycode &self, const int other);
	int runOperatorPLUS(const int self, const ActorKeyboard::Keycode &other);

	int runOperatorMINUS(const ActorKeyboard::Keycode &self, const ActorKeyboard::Keycode &other);
	int runOperatorMINUS(const ActorKeyboard::Keycode &self, const int other);
	int runOperatorMINUS(const int self, const ActorKeyboard::Keycode &other);

	int runOperatorASTERISK(const ActorKeyboard::Keycode &self, const ActorKeyboard::Keycode &other);
	int runOperatorASTERISK(const ActorKeyboard::Keycode &self, const int other);
	int runOperatorASTERISK(const int self, const ActorKeyboard::Keycode &other);

protected:
	struct KeyEvent {
		static const qint64 MAX_DELTA = 10;
		int kumirCode;
		qint64 timestamp;

		inline explicit KeyEvent(int kumirCodee) : kumirCode(kumirCodee)
		{
			timestamp = QDateTime::currentMSecsSinceEpoch();
		}
		inline explicit KeyEvent(): kumirCode(0), timestamp(0) {}
	};

	bool eventFilter(QObject *obj, QEvent *event);
	static int polyakovCodeOfKey(int qtCode, const QString &text);

	kumir2::LockedQueue<KeyEvent> buffer_;
	KeyEvent lastPressed_;
	QMutex lastPressedLock_;




};


} // namespace ActorKeyboard

#endif // KEYBOARDMODULE_H
