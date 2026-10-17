#include <QPainter>
#include <QMouseEvent>
#include <QPalette>
#include "timerangeslider.h"

#define HANDLE_WIDTH  10
#define HANDLE_HEIGHT 18
#define GROOVE_HEIGHT 4

static inline int eventX(QMouseEvent *event)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	return event->pos().x();
#else // QT 6
	return qRound(event->position().x());
#endif // QT 6
}

TimeRangeSlider::TimeRangeSlider(QWidget *parent)
  : QWidget(parent), _min(0), _max(0), _low(0), _high(0), _drag(None)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setMouseTracking(false);
	setCursor(Qt::SizeHorCursor);
}

QSize TimeRangeSlider::sizeHint() const
{
	return QSize(240, HANDLE_HEIGHT + 4);
}

QSize TimeRangeSlider::minimumSizeHint() const
{
	return QSize(100, HANDLE_HEIGHT + 4);
}

void TimeRangeSlider::setRange(qint64 min, qint64 max)
{
	if (max < min)
		qSwap(min, max);
	_min = min;
	_max = max;
	_low = qBound(_min, _low, _max);
	_high = qBound(_min, _high, _max);
	update();
}

void TimeRangeSlider::setValues(qint64 low, qint64 high)
{
	if (high < low)
		qSwap(low, high);
	_low = qBound(_min, low, _max);
	_high = qBound(_min, high, _max);
	update();
}

int TimeRangeSlider::left() const
{
	return HANDLE_WIDTH / 2 + 1;
}

int TimeRangeSlider::right() const
{
	return width() - HANDLE_WIDTH / 2 - 2;
}

int TimeRangeSlider::valueToPos(qint64 value) const
{
	if (_max <= _min)
		return left();
	double r = (double)(value - _min) / (double)(_max - _min);
	return left() + qRound(r * (right() - left()));
}

qint64 TimeRangeSlider::posToValue(int pos) const
{
	int span = right() - left();
	if (span <= 0 || _max <= _min)
		return _min;
	double r = (double)(qBound(left(), pos, right()) - left()) / span;
	return _min + (qint64)(r * (double)(_max - _min) + 0.5);
}

void TimeRangeSlider::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing, true);

	const QPalette &pal = palette();
	QPalette::ColorGroup cg = isEnabled() ? QPalette::Active
	  : QPalette::Disabled;
	int cy = height() / 2;
	int lx = valueToPos(_low);
	int hx = valueToPos(_high);

	// groove
	QRect groove(left(), cy - GROOVE_HEIGHT / 2, right() - left(),
	  GROOVE_HEIGHT);
	p.setPen(Qt::NoPen);
	p.setBrush(pal.color(cg, QPalette::Mid));
	p.drawRoundedRect(groove, 2, 2);

	// selected span
	QRect span(lx, cy - GROOVE_HEIGHT / 2, hx - lx, GROOVE_HEIGHT);
	p.setBrush(pal.color(cg, QPalette::Highlight));
	p.drawRect(span);

	// handles
	p.setPen(QPen(pal.color(cg, QPalette::Dark), 1));
	p.setBrush(pal.color(cg, QPalette::Button));
	QRect h(0, cy - HANDLE_HEIGHT / 2, HANDLE_WIDTH, HANDLE_HEIGHT);
	h.moveLeft(lx - HANDLE_WIDTH / 2);
	p.drawRoundedRect(h, 2, 2);
	h.moveLeft(hx - HANDLE_WIDTH / 2);
	p.drawRoundedRect(h, 2, 2);
}

void TimeRangeSlider::mousePressEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton || _max <= _min) {
		QWidget::mousePressEvent(event);
		return;
	}

	int x = eventX(event);
	int dl = qAbs(x - valueToPos(_low));
	int dh = qAbs(x - valueToPos(_high));

	if (dl < dh)
		_drag = Low;
	else if (dh < dl)
		_drag = High;
	else // both handles at the same place
		_drag = (x < valueToPos(_low)) ? Low : High;

	moveHandle(x);
}

void TimeRangeSlider::mouseMoveEvent(QMouseEvent *event)
{
	if (_drag == None) {
		QWidget::mouseMoveEvent(event);
		return;
	}

	moveHandle(eventX(event));
}

void TimeRangeSlider::mouseReleaseEvent(QMouseEvent *event)
{
	if (_drag == None || event->button() != Qt::LeftButton) {
		QWidget::mouseReleaseEvent(event);
		return;
	}

	moveHandle(eventX(event));
	_drag = None;
	emit dragFinished();
}

void TimeRangeSlider::moveHandle(int pos)
{
	qint64 value = posToValue(pos);

	if (_drag == Low)
		_low = qMin(value, _high);
	else if (_drag == High)
		_high = qMax(value, _low);
	else
		return;

	update();
	emit valuesChanged(_low, _high);
}
