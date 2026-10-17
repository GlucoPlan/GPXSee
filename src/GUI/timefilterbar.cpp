#include <QCheckBox>
#include <QDateTimeEdit>
#include <QLabel>
#include <QTimer>
#include <QAction>
#include <QSignalBlocker>
#include "timerangeslider.h"
#include "timefilterbar.h"

#define DATETIME_FORMAT "dd.MM.yyyy HH:mm:ss"
#define APPLY_DELAY     500 /* ms, debounce of field edits */

static QDateTimeEdit *createEdit(QWidget *parent)
{
	QDateTimeEdit *edit = new QDateTimeEdit(parent);
	edit->setDisplayFormat(DATETIME_FORMAT);
	edit->setCalendarPopup(true);
	/* The fields show the wall-clock time of the application time zone.
	   They are kept in UTC to avoid any local DST adjustments of the shown
	   values; the conversion is done in toWallClock()/fromWallClock(). */
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	edit->setTimeZone(QTimeZone::utc());
#else // QT < 6.7
	edit->setTimeSpec(Qt::UTC);
#endif // QT < 6.7
	return edit;
}

TimeFilterBar::TimeFilterBar(QWidget *parent)
  : QToolBar(tr("Time filter"), parent), _modified(false),
  _zone(QTimeZone::utc())
{
	_enable = new QCheckBox(tr("Time filter"), this);
	_enable->setToolTip(tr("Show only the track points within the selected"
	  " time range"));
	_fromEdit = createEdit(this);
	_fromEdit->setToolTip(tr("Start of the time range"));
	_toEdit = createEdit(this);
	_toEdit->setToolTip(tr("End of the time range"));
	_slider = new TimeRangeSlider(this);
	_slider->setToolTip(tr("Drag the handles to select the time range"));

	_timer = new QTimer(this);
	_timer->setSingleShot(true);
	_timer->setInterval(APPLY_DELAY);

	addWidget(_enable);
	addSeparator();
	addWidget(new QLabel(tr("From:"), this));
	addWidget(_fromEdit);
	addWidget(new QLabel(tr("To:"), this));
	addWidget(_toEdit);
	addSeparator();
	addWidget(_slider);
	_resetAction = addAction(tr("Reset"));
	_resetAction->setToolTip(tr("Reset to the full time range"));

	connect(_enable, &QCheckBox::toggled, this, &TimeFilterBar::enableToggled);
	connect(_fromEdit, &QDateTimeEdit::dateTimeChanged, this,
	  &TimeFilterBar::fromEdited);
	connect(_toEdit, &QDateTimeEdit::dateTimeChanged, this,
	  &TimeFilterBar::toEdited);
	connect(_slider, &TimeRangeSlider::valuesChanged, this,
	  &TimeFilterBar::sliderMoved);
	connect(_slider, &TimeRangeSlider::dragFinished, this,
	  &TimeFilterBar::sliderReleased);
	connect(_resetAction, &QAction::triggered, this, &TimeFilterBar::reset);
	connect(_timer, &QTimer::timeout, this, &TimeFilterBar::apply);

	updateEnabled();
}

QDateTime TimeFilterBar::toWallClock(const QDateTime &dt) const
{
	QDateTime local(dt.toTimeZone(_zone));
	return QDateTime(local.date(), local.time(), QTimeZone::utc());
}

QDateTime TimeFilterBar::fromWallClock(const QDateTime &dt) const
{
	return QDateTime(dt.date(), dt.time(), _zone);
}

bool TimeFilterBar::isActive() const
{
	return _enable->isChecked() && _min.isValid() && _max.isValid();
}

QDateTime TimeFilterBar::from() const
{
	return _modified ? _from : QDateTime();
}

QDateTime TimeFilterBar::to() const
{
	return _modified ? _to : QDateTime();
}

void TimeFilterBar::setBounds(const QDateTime &min, const QDateTime &max)
{
	_min = min;
	_max = max;

	if (!_min.isValid() || !_max.isValid()) {
		_modified = false;
		_from = QDateTime();
		_to = QDateTime();
	} else if (!_modified) {
		_from = _min;
		_to = _max;
	}

	updateWidgets();
	updateEnabled();
}

void TimeFilterBar::setTimeZone(const QTimeZone &zone)
{
	_zone = zone;
	updateWidgets();
}

void TimeFilterBar::deactivate()
{
	_timer->stop();
	QSignalBlocker blocker(_enable);
	_enable->setChecked(false);
	updateEnabled();
}

void TimeFilterBar::updateEnabled()
{
	bool data = _min.isValid() && _max.isValid();
	bool on = data && _enable->isChecked();

	_enable->setEnabled(data);
	_fromEdit->setEnabled(on);
	_toEdit->setEnabled(on);
	_slider->setEnabled(on);
	_resetAction->setEnabled(on && _modified);
}

void TimeFilterBar::updateWidgets()
{
	QSignalBlocker b1(_fromEdit);
	QSignalBlocker b2(_toEdit);

	if (!_min.isValid() || !_max.isValid()) {
		_slider->setRange(0, 0);
		_slider->setValues(0, 0);
		return;
	}

	_slider->setRange(_min.toMSecsSinceEpoch(), _max.toMSecsSinceEpoch());
	_slider->setValues(_from.toMSecsSinceEpoch(), _to.toMSecsSinceEpoch());
	_fromEdit->setDateTime(toWallClock(_from));
	_toEdit->setDateTime(toWallClock(_to));
}

void TimeFilterBar::enableToggled(bool checked)
{
	Q_UNUSED(checked);

	_timer->stop();
	updateEnabled();
	emit filterChanged();
}

void TimeFilterBar::fromEdited(const QDateTime &dt)
{
	// the field has a 1s resolution, the start is the beginning of the second
	_from = fromWallClock(dt);
	if (_from > _to)
		_to = _from.addMSecs(999);
	_modified = true;

	updateWidgets();
	updateEnabled();
	scheduleApply();
}

void TimeFilterBar::toEdited(const QDateTime &dt)
{
	// the field has a 1s resolution, the end is the end of the second
	_to = fromWallClock(dt).addMSecs(999);
	if (_to < _from)
		_from = fromWallClock(dt);
	_modified = true;

	updateWidgets();
	updateEnabled();
	scheduleApply();
}

void TimeFilterBar::sliderMoved(qint64 low, qint64 high)
{
	_from = QDateTime::fromMSecsSinceEpoch(low, QTimeZone::utc());
	_to = QDateTime::fromMSecsSinceEpoch(high, QTimeZone::utc());
	_modified = true;

	QSignalBlocker b1(_fromEdit);
	QSignalBlocker b2(_toEdit);
	_fromEdit->setDateTime(toWallClock(_from));
	_toEdit->setDateTime(toWallClock(_to));
	updateEnabled();
}

void TimeFilterBar::sliderReleased()
{
	_timer->stop();
	apply();
}

void TimeFilterBar::reset()
{
	_modified = false;
	_from = _min;
	_to = _max;

	updateWidgets();
	updateEnabled();
	_timer->stop();
	apply();
}

void TimeFilterBar::scheduleApply()
{
	if (isActive())
		_timer->start();
}

void TimeFilterBar::apply()
{
	if (isActive())
		emit filterChanged();
}
