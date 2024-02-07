#include "tabwidgetelement.h"
#include "kumirprogram.h"
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include <kumir2-libs/widgets/actionproxy.h>
#include "toolbarcontextmenu.h"

#include <kumir2/actorinterface.h>
#include <kumir2/editor_instanceinterface.h>
#include "kumir2/analizer_instanceinterface.h"
#include <kumir2/startpage_widget_interface.h>
#include <kumir2-libs/extensionsystem/pluginmanager.h>

namespace CoreGUI
{

TabWidgetElement::TabWidgetElement(
	QWidget *w
	, ExtensionSystem::SettingsPtr settings
	, bool enableToolBar
	, QList<QAction *> toolbarActions
	, QList<QMenu *> ms
//                                     , QList<QWidget*> sws
	, MainWindow::DocumentType t
	, QActionGroup *gr_fileActions
	, QActionGroup *gr_otherActions
	, class KumirProgram *kumir
)
	: QWidget()
	, component(w)
	, menus(ms)
//        , statusbarWidgets(sws)
	, type(t)
	, editorInstance_(nullptr)
	, startPageInstance_(nullptr)
	, kumirProgram_(kumir)
	, courseManagerTab_(false)
	, documentHasChanges_(false)
	, actionSave_(nullptr)
	, toolbarContextMenu_(nullptr)
{
	kumirProgram_ = nullptr;
	Q_CHECK_PTR(w);
	Q_ASSERT(!QString::fromLatin1(w->metaObject()->className()).isEmpty());
	setProperty("uncloseable", w->property("uncloseable"));
	if (type == MainWindow::StartPage) {
		connect(w, SIGNAL(titleChanged(QString)), this, SIGNAL(changeTitle(QString)));
	} else {
		connect(w, SIGNAL(documentCleanChanged(bool)), this, SIGNAL(documentCleanChanged(bool)));
		connect(w, SIGNAL(documentCleanChanged(bool)), this, SLOT(setDocumentChangesClean(bool)));
	}
	QVBoxLayout *l = new QVBoxLayout;
	l->setContentsMargins(0, 0, 0, 0);
	l->setSpacing(0);
	setLayout(l);
	if (enableToolBar) {
		if (!toolbarActions.isEmpty()) {
			using namespace Widgets;
			QToolBar *tb = new QToolBar(this);
			toolbarContextMenu_ = new ToolbarContextMenu(this);
			toolbarContextMenu_->setSettingsObject(settings, "MainToolBar");
			tb->installEventFilter(this);
			tb->setIconSize(QSize(22, 22));
#ifdef Q_OS_MAC
			static const char *css = ""
				"QToolBar {"
				"   border: 0px;"
				"   background-color: $windowColor;"
				"   padding: 8px;"
				"}"
				"QToolButton {"
				"   border: 0px;"
				"}"
				"";
#else
			static const char *css = ""
				"QToolBar { border: 0px }";
#endif
			tb->setStyleSheet(QString::fromLatin1(css).replace("$windowColor", palette().brush(QPalette::Window).color().name()));
			l->addWidget(tb);
			if (type != MainWindow::StartPage) {
				Q_FOREACH (QAction *a, gr_fileActions->actions()) {
					ActionProxy *proxy = new ActionProxy(a, this);
					if ("file-save" == a->objectName()) {
						actionSave_ = proxy;
					}
					tb->addAction(proxy);
					if (proxy->isSeparator()) {
						toolbarContextMenu_->addSeparator();
					} else {
						toolbarContextMenu_->addProxy(proxy);
					}
				}
			}
			tb->addSeparator();
			toolbarContextMenu_->addSeparator();
			foreach (QAction *a, toolbarActions) {
				ActionProxy *proxy = new ActionProxy(a, this);
				tb->addAction(proxy);
				if (proxy->isSeparator()) {
					toolbarContextMenu_->addSeparator();
				} else {
					toolbarContextMenu_->addProxy(proxy);
				}
			}
			if (type == MainWindow::Program) {
				tb->addSeparator();
				toolbarContextMenu_->addSeparator();
				QList<QAction *> acts = kumir->actions()->actions();
				for (int i = 0; i < acts.size(); i++) {
					if (acts[i]->isSeparator() || !acts[i]->icon().isNull()) {
						ActionProxy *proxy = new ActionProxy(acts[i], this);
						tb->addAction(proxy);
						if (proxy->isSeparator()) {
							toolbarContextMenu_->addSeparator();
						} else {
							toolbarContextMenu_->addProxy(proxy);
						}
					}
				}
			}
			if (!gr_otherActions->actions().isEmpty()) {
				tb->addSeparator();
				toolbarContextMenu_->addSeparator();
				Q_FOREACH (QAction *a, gr_otherActions->actions()) {
					ActionProxy *proxy = new ActionProxy(a, this);
					tb->addAction(proxy);
					if (proxy->isSeparator()) {
						toolbarContextMenu_->addSeparator();
					} else {
						toolbarContextMenu_->addProxy(proxy);
					}
				}
			}
			toolbarContextMenu_->finalize();
			tb->addAction(toolbarContextMenu_->showAction());
		}

	}
	l->addWidget(w);
}

void TabWidgetElement::setEditor(Shared::Editor::InstanceInterface *editor)
{
	editorInstance_ = editor;
	if (editor && editor->analizer() && editor->analizer()->helper()) {
		editor->analizer()->helper()->connectSignalImportsChanged(
			this,
			SLOT(updateCompilerImportsList(QStringList))
		);
	}
}

void TabWidgetElement::setStartPage(Shared::StartpageWidgetInterface *sp)
{
	startPageInstance_ = sp;
}

void TabWidgetElement::updateCompilerImportsList(const QStringList &localizedNames)
{
	using ExtensionSystem::PluginManager;
	using Shared::ActorInterface;
	using Shared::actorCanonicalName;

	QSet<QString> availableWindowNames;
	availableMenuNames_.clear();

	QList<ActorInterface *> actors = PluginManager::instance()->findPlugins<ActorInterface>();
	Q_FOREACH (ActorInterface *actor, actors) {
		const QString actorName = actorCanonicalName(actor->localizedModuleName(QLocale::Russian));
		if (localizedNames.contains(actorName)) {
			const QByteArray actorObjectName =
				actorCanonicalName(actor->asciiModuleName()).replace(" ", "-").toLower();
			if (actor->mainWidget()) {
				availableWindowNames.insert(QString("window-actor-%1").arg(QString(actorObjectName)));
			}
			if (actor->pultWidget()) {
				availableWindowNames.insert(QString("window-control-%1").arg(QString(actorObjectName)));
			}
			if (actor->moduleMenus().size() > 0) {
				availableMenuNames_.insert("menu-Actor" + actorCanonicalName(actor->asciiModuleName()));
			}
		}
	}
	if (toolbarContextMenu_) {
		toolbarContextMenu_->setExplicitImportNames(availableWindowNames);
	}
	Q_EMIT explicitImportNamesRequest();
}

void TabWidgetElement::updateSettingsObject(SettingsPtr settings)
{
	if (toolbarContextMenu_) {
		toolbarContextMenu_->setSettingsObject(settings, "MainToolBar");
	}
}

bool TabWidgetElement::eventFilter(QObject *obj, QEvent *evt)
{
//    if (QEvent::ContextMenu==evt->type()) {
//        QContextMenuEvent * event = static_cast<QContextMenuEvent*>(evt);
//        const QPoint position = event->globalPos();
//        QRect contextRect(position, toolbarContextMenu_->size());
//        QDesktopWidget* screen = qApp->desktop();
//        const QRect screenRect = screen->availableGeometry(this);
//        if (contextRect.right() > screenRect.right()) {
//            contextRect.moveRight(screenRect.right());
//        }
//        if (contextRect.left() < screenRect.left()) {
//            contextRect.moveLeft(screenRect.left());
//        }

//        toolbarContextMenu_->move(contextRect.topLeft());
//        toolbarContextMenu_->show();
//        return true;
//    }
	return false;
}

void TabWidgetElement::setDocumentChangesClean(bool clean)
{
	bool oldDocumentHasChanges = documentHasChanges_;
	documentHasChanges_ = ! clean;
	if (editorInstance_ &&
		!isCourseManagerTab() &&
		documentHasChanges_ != oldDocumentHasChanges) {
		emit titleChanged(title());
	}
	if (actionSave_) {
		foreach (QWidget *w, actionSave_->associatedWidgets()) {
			if (QString(w->metaObject()->className()) == "QToolButton") {
				QToolButton *btn = qobject_cast<QToolButton *>(w);
				btn->setAutoRaise(clean);
			}
		}
	}
}

QString TabWidgetElement::title() const
{
	if (editorInstance_) {
		const Shared::Analizer::SourceFileInterface::Data data = editorInstance_->documentContents();
		const QUrl url = data.sourceUrl;
		if (isCourseManagerTab()) {
			return tr("%1 (Course)").arg(courseTitle_).trimmed();
		} else if (url.isValid()) {
			const QString fullPath = editorInstance_->documentContents().sourceUrl.toLocalFile();
			const QString shortPath = QFileInfo(fullPath).fileName();
			QString title;
			if (documentHasChanges_ && !isCourseManagerTab()) {
				title = shortPath + "*";
			} else {
				title = shortPath;
			}
			return title;
		} else {
			QString title;
			if (MainWindow::Program == type) {
				title = tr("New Program");
			} else if (MainWindow::Text == type) {
				title = tr("New Text");
			}
			if (title.length() > 0 && documentHasChanges_) {
				title += "*";
			}
			return title;
		}
	} else if (startPageInstance_) {
		return startPageInstance_->startPageTitle();
	} else {
		return "";
	}
}

}

