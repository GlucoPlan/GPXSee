#ifndef TIMEFILTERBAR_H
#define TIMEFILTERBAR_H

#include <QToolBar>
#include <QDateTime>
#include <QTimeZone>

class QCheckBox;
class QDateTimeEdit;
class QTimer;
class QAction;
class TimeRangeSlider;

/* Toolbar for the display-only track time filter: an enable checkbox,
   "from"/"to" date-time fields, a two-handle time slider and a reset button.
   The fields and the slider are kept in sync both ways. */
class TimeFilterBar : public QToolBar
{
	Q_OBJECT

public:
	TimeFilterBar(QWidget *parent = 0);

	/* Full time span of the loaded data. Null values disable the filter. */
	void setBounds(const QDateTime &min, const QDateTime &max);
	void setTimeZone(const QTimeZone &zone);
	/* Switch the filter off without emitting filterChanged(). */
	void deactivate();

	bool isActive() const;
	/* Effective filter bounds. Null when the user did not narrow the range,
	   meaning "unbounded" on that side. */
	QDateTime from() const;
	QDateTime to() const;

signals:
	void filterChanged();

private slots:
	void enableToggled(bool checked);
	void fromEdited(const QDateTime &dt);
	void toEdited(const QDateTime &dt);
	void sliderMoved(qint64 low, qint64 high);
	void sliderReleased();
	void reset();
	void apply();

private:
	void updateWidgets();
	void updateEnabled();
	void scheduleApply();
	QDateTime toWallClock(const QDateTime &dt) const;
	QDateTime fromWallClock(const QDateTime &dt) const;

	QCheckBox *_enable;
	QDateTimeEdit *_fromEdit;
	QDateTimeEdit *_toEdit;
	TimeRangeSlider *_slider;
	QAction *_resetAction;
	QTimer *_timer;

	QDateTime _min, _max;
	QDateTime _from, _to;
	bool _modified;
	QTimeZone _zone;
};

#endif // TIMEFILTERBAR_H
