#include "terminal_plane.h"
#include "terminal.h"
#include "terminal_onesession.h"

#include <algorithm>
#include <QScrollBar>
#include <QTimerEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QPainter>
#include <QTextCodec>
#include <QMimeData>

namespace Terminal
{

static uint SessionMargin = 4u;

Plane::Plane(Term *parent) :
	QWidget(parent),
	terminal_(parent),
	inputMode_(false),
	inputPosition_(0),
	mousePressSession_(nullptr),
	actionCopyToClipboard_(new QAction(this)),
	actionPasteFromClipboard_(new QAction(this)),
	autoScrollStateX_(0),
	autoScrollStateY_(0),
	autoScrollTimerId_(-1)
{
	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	actionCopyToClipboard_->setText(tr("Copy to clipboard"));
	connect(actionCopyToClipboard_, SIGNAL(triggered()),
		this, SLOT(copyToClipboard()));

	actionPasteFromClipboard_->setText(tr("Paste from clipboard"));
	connect(actionPasteFromClipboard_, SIGNAL(triggered()),
		this, SLOT(pasteFromClipboard()));

	connect(this, SIGNAL(requestAutoScrollX(char)),
		this, SLOT(handleAutoscrollXChange(char)));
	connect(this, SIGNAL(requestAutoScrollY(char)),
		this, SLOT(handleAutoscrollYChange(char)));

	autoScrollTimerId_ = startTimer(100);

}

void Plane::keyPressEvent(QKeyEvent *e)
{
	if (e->matches(QKeySequence::Copy)) {
		copyToClipboard();
		e->accept();
		return;
	}
	if (e->matches(QKeySequence::SelectAll)) {
		selectAll();
		e->accept();
		return;
	}
	if (!inputMode_) {
		e->ignore();
		return;
	}
	if (e->matches(QKeySequence::Paste)) {
		pasteFromClipboard();
		e->accept();
	} else if (e->matches(QKeySequence::MoveToNextChar)) {
		inputPosition_++;
		emit inputCursorPositionChanged(inputPosition_);
		e->accept();
	} else if (e->matches(QKeySequence::MoveToPreviousChar)) {
		if (inputPosition_ > 0) {
			inputPosition_ --;
			emit inputCursorPositionChanged(inputPosition_);
			e->accept();
		}
	} else if (e->matches(QKeySequence::MoveToStartOfLine)
		|| e->matches(QKeySequence::MoveToStartOfDocument)
		|| e->matches(QKeySequence::MoveToPreviousLine)
	) {
		inputPosition_ = 0;
		emit inputCursorPositionChanged(inputPosition_);
		e->accept();
	} else if (e->matches(QKeySequence::MoveToEndOfLine)
		|| e->matches(QKeySequence::MoveToEndOfDocument)
		|| e->matches(QKeySequence::MoveToNextLine)
	) {
		inputPosition_ = inputText_.length();
		emit inputCursorPositionChanged(inputPosition_);
		e->accept();
	} else if (e->key() == Qt::Key_Backspace) {
		if (inputPosition_ > 0) {
			if (inputPosition_ > inputText_.length()) {
				inputPosition_ = inputText_.length();
				emit inputCursorPositionChanged(inputPosition_);
			} else {
				inputText_.remove(inputPosition_ - 1, 1);
				inputPosition_ --;
				emit inputCursorPositionChanged(inputPosition_);
				emit inputTextChanged(inputText_);
			}
			e->accept();
		}
	} else if (e->key() == Qt::Key_Delete) {
		if (inputPosition_ < inputText_.length()) {
			inputText_.remove(inputPosition_, 1);
			emit inputCursorPositionChanged(inputPosition_);
			emit inputTextChanged(inputText_);
			e->accept();
		}
	} else if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
		emit inputFinishRequest();
		e->accept();
	} else if (!e->text().isEmpty()) {
		while (inputPosition_ > inputText_.length()) {
			inputText_ += " ";
		}
		inputText_.insert(inputPosition_, e->text());
		inputPosition_ += e->text().length();
		emit inputTextChanged(inputText_);
		emit inputCursorPositionChanged(inputPosition_);
		e->accept();
	}
}

OneSession *Plane::sessionByPos(const QPoint &pos) const
{
	OneSession *result = nullptr;
	foreach (OneSession *session, terminal_->sessions_) {
		if (sessionRect(session).contains(pos)) {
			result = session;
			break;
		}
	}
	return result;
}

QRect Plane::sessionRect(const OneSession *session) const
{
	QRect result(offset() + QPoint(0, SessionMargin), QSize(0, 0));
	foreach (const OneSession *s, terminal_->sessions_) {
		result.setSize(s->visibleSize());
		if (session == s) {
			break;
		}
		result.translate(0, result.height() + SessionMargin);
	}
	result.setWidth(qMax(result.width(), width()));
	return result;
}

void Plane::mousePressEvent(QMouseEvent *e)
{
	emit requestAutoScrollX(0);
	emit requestAutoScrollY(0);
	setFocus();
	e->accept();
	mousePressSession_ = sessionByPos(e->pos());
	mousePressPosition_ = e->pos();
	const QPoint scrollOffset = QPoint(terminal_->sb_horizontal->value(),
			terminal_->sb_vertical->value());
	mousePressPosition_ += scrollOffset;

	if (e->button() != Qt::RightButton) {
		for (int i = 0; i < terminal_->sessions_.size(); i++) {
			terminal_->sessions_.at(i)->clearSelection();
		}
	}
	update();
}

void Plane::mouseMoveEvent(QMouseEvent *e)
{
	e->accept();
	const QPoint scrollOffset = QPoint(terminal_->sb_horizontal->value(),
			terminal_->sb_vertical->value());
	QPoint pos = e->pos() + scrollOffset;

	if (e->pos().y() < 0) {
		emit requestAutoScrollY(-1);
	} else if (e->pos().y() > height()) {
		emit requestAutoScrollY(+1);
	} else {
		emit requestAutoScrollY(0);
	}

	if (e->pos().x() < 0) {
		emit requestAutoScrollX(-1);
	} else if (e->pos().x() > width()) {
		emit requestAutoScrollX(+1);
	} else {
		emit requestAutoScrollX(0);
	}

	if (e->button() != Qt::RightButton) {
		for (int i = 0; i < terminal_->sessions_.size(); i++) {
			terminal_->sessions_.at(i)->clearSelection();
		}
	}
	QPoint from, to;
	if (pos.y() > mousePressPosition_.y()) {
		from = mousePressPosition_;
		to = pos;
	} else if (pos.y() < mousePressPosition_.y()) {
		from = pos;
		to = mousePressPosition_;
	} else if (/* y equal and */ pos.x() < mousePressPosition_.x()) {
		from = pos;
		to = mousePressPosition_;
	} else {
		from = mousePressPosition_;
		to = pos;
	}
	for (int i = 0; i < terminal_->sessions_.size(); i++) {
		OneSession *session = terminal_->sessions_.at(i);
		const QRect rect = sessionRect(session).translated(scrollOffset);
		const QPoint sessionOffset = rect.topLeft();
		session->clearSelection();
		if ((from.y() <= rect.bottom()) && (to.y() >= rect.top())) {
			// Selection affects this session
			QPoint fromPos = QPoint(0, 0);
			QPoint toPos = rect.bottomRight() - sessionOffset;
			if (from.y() >= rect.top()) {
				fromPos = from - sessionOffset;
			}
			if (to.y() <= rect.bottom()) {
				toPos = to - sessionOffset;
			}
			session->triggerTextSelection(fromPos, toPos);
		}
	}


	update();
}

void Plane::mouseReleaseEvent(QMouseEvent *e)
{
	emit requestAutoScrollX(0);
	emit requestAutoScrollY(0);
	e->accept();
}

QPoint Plane::offset() const
{
	QPoint result(0, 0);
	if (terminal_->sb_horizontal->isEnabled()) {
		int valX = terminal_->sb_horizontal->value();
		result.setX(-valX);
	}
	if (terminal_->sb_vertical->isEnabled()) {
		int valY = terminal_->sb_vertical->value();
		result.setY(-valY);
	}
	return result;
}

void Plane::contextMenuEvent(QContextMenuEvent *event)
{
	event->accept();
	bool canCopyToClipboard = false;
	foreach (const OneSession *s, terminal_->sessions_) {
		canCopyToClipboard = canCopyToClipboard || s->hasSelectedText();
	}
	const QClipboard *clipboard = QApplication::clipboard();
	bool canPasteFromClipboard = inputMode_ &&
		clipboard->text().length() > 0;
	bool hasAnyAction =
		canCopyToClipboard ||
		canPasteFromClipboard;

	if (hasAnyAction) {
		QMenu *menu = new QMenu(this);
		if (canCopyToClipboard) {
			menu->addAction(actionCopyToClipboard_);
		}
		if (canPasteFromClipboard) {
			menu->addAction(actionPasteFromClipboard_);
		}
		menu->exec(mapToGlobal(event->pos()));
	}
}

void Plane::timerEvent(QTimerEvent *e)
{
	e->accept();
	if (e->timerId() == autoScrollTimerId_) {
		QScrollBar *h = terminal_->sb_horizontal;
		QScrollBar *v = terminal_->sb_vertical;
		if (-1 == autoScrollStateY_) {
			if (v->value() > 0) {
				v->setValue(v->value() - v->singleStep());
			}
		} else if (1 == autoScrollStateY_) {
			if (v->value() < v->maximum()) {
				v->setValue(v->value() + v->singleStep());
			}
		}
		if (-1 == autoScrollStateX_) {
			if (h->value() > 0) {
				h->setValue(h->value() - h->singleStep());
			}
		} else if (1 == autoScrollStateX_) {
			if (h->value() < h->maximum()) {
				h->setValue(h->value() + h->singleStep());
			}
		}
	}
}

void Plane::selectAll()
{
	foreach (OneSession *s, terminal_->sessions_) {
		s->selectAll();
	}
	update();
}

void Plane::copyToClipboard()
{
	QClipboard *clipboard = QApplication::clipboard();
	QString text;
	QString rtfBody;

	QString rtfHeader = "{\\rtf1\\ansicpg1251\r\n"
		"{\\fonttbl{\\f0\\fmodern\\fcharset204 Courier New;}}\r\n"
		"{\\colortbl;"
		"\\red0\\green0\\blue0;"  // Main text
		"\\red255\\green0\\blue0;"  // Error stream
		"\\red0\\green0\\blue255;"  // Input data
		"\\red128\\green128\\blue128;" // Headings
		"}\r\n"
		"{\\f0\\lang1024\r\n"
		;

	QString rtfFooter = "}}\r\n";


	foreach (const OneSession *s, terminal_->sessions_) {
		if (s->hasSelectedText()) {
			if (text.length() > 0) {
				text += "\n";
			}
			if (rtfBody.length() > 0) {
				rtfBody += "\\par\r\n";
			}
			text += s->selectedText();
			rtfBody += s->selectedRtf();
		}
	}
	QMimeData *richData = new QMimeData();
	QByteArray rtfData = QTextCodec::codecForName("CP1251")->fromUnicode(rtfHeader + rtfBody + rtfFooter);
	QFile debug("/home/victor/test.rtf");
	if (debug.open(QIODevice::WriteOnly | QIODevice::Text)) {
		debug.write(rtfData);
		debug.close();
	}
	richData->setText(text);
	richData->setData("text/rtf", rtfData);
	clipboard->setMimeData(richData);
}

void Plane::pasteFromClipboard()
{
	const QClipboard *clipboard = QApplication::clipboard();
	const QString text = clipboard->text().replace("\n", " ");
	inputText_ += text;
	inputPosition_ += text.length();
	emit inputTextChanged(inputText_);
	emit inputCursorPositionChanged(inputPosition_);
}

void Plane::handleAutoscrollXChange(char directionSign)
{
	autoScrollStateX_ = directionSign;
}

void Plane::handleAutoscrollYChange(char directionSign)
{
	autoScrollStateY_ = directionSign;
}

void Plane::updateScrollBars()
{
	const int myHeight = height();
	QPoint prevOffset = offset();
	int w = 2 * SessionMargin;
	int h = 0;
	for (int i = 0; i < terminal_->sessions_.size(); i++) {
		if (i == 0) {
			h += SessionMargin;
		}
		OneSession *s = terminal_->sessions_[i];
		QSize ss = s->visibleSize();
		w = qMax(w, ss.width());
		h += ss.height();
		if (i < terminal_->sessions_.size() - 1) {
			h += 2 * SessionMargin;
		}
		if (i == terminal_->sessions_.size() - 1) {
			int lastSessionHeight = ss.height() + SessionMargin;
			int fillSpace = myHeight - lastSessionHeight;
			if (fillSpace > 0) {
				h += fillSpace;
			}
		}
	}

	QScrollBar *hb = terminal_->sb_horizontal;
	QScrollBar *vb = terminal_->sb_vertical;

	if (w <= width()) {
		hb->setEnabled(false);
		hb->setVisible(false);
	} else {
		hb->setEnabled(true);
		hb->setVisible(true);
		hb->setRange(0, w - width());
		hb->setSingleStep(width() / 3);
		hb->setPageStep(width());
	}

	if (h <= myHeight) {
		vb->setEnabled(false);
		vb->setVisible(false);
	} else {
		QFontMetrics fm(font());
		int dH = fm.height();
		vb->setEnabled(true);
		vb->setVisible(true);
		vb->setRange(0, h - myHeight);
		vb->setSingleStep(dH);
		vb->setPageStep(myHeight);
	}
	if (prevOffset != offset()) {
		update();
	}

}

void Plane::resizeEvent(QResizeEvent *e)
{
	foreach (OneSession *session, terminal_->sessions_) {
		session->relayout(e->size().width() - 2 * SessionMargin, 0, true);
	}

	QWidget::resizeEvent(e);
	updateScrollBars();
}

QSize Plane::minimumSizeHint() const
{
	if (terminal_->sessions_.isEmpty()) {
		return QSize(0, 0);
	} else {
		OneSession *last = terminal_->sessions_.last();
		return last->minimumSizeHint() + QSize(2 * SessionMargin, 2 * SessionMargin);
	}
}

void Plane::wheelEvent(QWheelEvent *e)
{
	if (!terminal_->sb_vertical->isEnabled() && e->orientation() == Qt::Vertical) {
		e->ignore();
		return;
	}
	if (!terminal_->sb_horizontal->isEnabled() && e->orientation() == Qt::Horizontal) {
		e->ignore();
		return;
	}
	int degrees = e->delta() / 8;
	int steps = degrees / 15;
	QScrollBar *sb = e->orientation() == Qt::Vertical ? terminal_->sb_vertical : terminal_->sb_horizontal;
	sb->setValue(sb->value() - steps * sb->singleStep() * 3);
}

void Plane::paintEvent(QPaintEvent *e)
{
	QPainter p(this);
	p.setRenderHint(QPainter::TextAntialiasing, true);

	QPoint off = offset();
	const QRect dirtyRect = e->rect();
	int y = SessionMargin;
	for (int i = 0; i < terminal_->sessions_.size(); i++) {
		OneSession *session = terminal_->sessions_[i];
		const QSize sessionSize = session->visibleSize();
		const QRect sessionRect(off + QPoint(0, y), sessionSize);
		bool drawThisSession = dirtyRect.intersects(sessionRect);
		if (drawThisSession) {
			p.save();
			p.translate(off.x(), y + off.y());
			const QRect sessionDirtyRect = QRect(
					-off.x(), dirtyRect.top() - sessionRect.top(),
					width(), dirtyRect.height()
				);
			session->draw(p, sessionDirtyRect);
			p.restore();
		}
		y += sessionSize.height() + 2 * SessionMargin;
		if (i < terminal_->sessions_.size() - 1) {
			p.save();
			p.setPen(QColor(Qt::lightGray));
			int lineY = y - SessionMargin + off.y();
			if (lineY != 0) {
				p.drawLine(0, lineY, width(), lineY);
			}
			p.restore();
		}
	}
	p.setBrush(Qt::NoBrush);
	QBrush br = hasFocus()
		? palette().brush(QPalette::Highlight)
		: palette().brush(QPalette::Mid);
	p.setPen(QPen(br, 3));
	p.drawLine(0, 0, width(), 0);
	p.drawLine(0, height() - 1, width(), height() - 1);
	p.drawLine(0, 0, 0, height() - 1);
	p.drawLine(width() - 1, 0, width() - 1, height());
	e->accept();
}

} // namespace Terminal
