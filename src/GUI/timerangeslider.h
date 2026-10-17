#ifndef TIMERANGESLIDER_H
#define TIMERANGESLIDER_H

#include <QWidget>

/* Horizontal slider with two draggable handles selecting a [low, high]
   sub-range of [minimum, maximum]. Values are milliseconds since epoch. */
class TimeRangeSlider : public QWidget
{
	Q_OBJECT

public:
	TimeRangeSlider(QWidget *parent = 0);

	void setRange(qint64 min, qint64 max);
	void setValues(qint64 low, qint64 high);

	qint64 minimum() const {return _min;}
	qint64 maximum() const {return _max;}
	qint64 low() const {return _low;}
	qint64 high() const {return _high;}

	QSize sizeHint() const;
	QSize minimumSizeHint() const;

signals:
	/* Emitted continuously while a handle is being dragged. */
	void valuesChanged(qint64 low, qint64 high);
	/* Emitted when the user releases a handle. */
	void dragFinished();

protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

private:
	enum Handle {None, Low, High};

	int left() const;
	int right() const;
	int valueToPos(qint64 value) const;
	qint64 posToValue(int pos) const;
	void moveHandle(int pos);

	qint64 _min, _max;
	qint64 _low, _high;
	Handle _drag;
};

#endif // TIMERANGESLIDER_H
