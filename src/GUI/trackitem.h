#ifndef TRACKITEM_H
#define TRACKITEM_H

#include <QDateTime>
#include "data/track.h"
#include "pathitem.h"

class Map;
class Track;

class TrackItem : public PathItem
{
	Q_OBJECT

public:
	TrackItem(const Track &track, Map *map, QGraphicsItem *parent = 0);

	ToolTip info(bool extended) const;
	bool isVideo() const {return _video;}

protected:
	bool pointInfo(int seg, int idx, ToolTip &tt) const;

private:
	QList<TrackPointInfoSegment> _points;

	qreal _time;
	qreal _movingTime;
	bool _video;
};

#endif // TRACKITEM_H
