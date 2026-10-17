#ifndef DATA_H
#define DATA_H

#include <QList>
#include <QMultiMap>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QPair>
#include "waypoint.h"
#include "track.h"
#include "route.h"
#include "parser.h"

class Data
{
public:
	Data(const QString &fileName, bool tryUnknown = true);
	Data(const QUrl &url);

	bool isValid() const {return _valid;}
	const QString &errorString() const {return _errorString;}
	int errorLine() const {return _errorLine;}

	const QList<Track> &tracks() const {return _tracks;}
	const QList<Route> &routes() const {return _routes;}
	const QVector<Waypoint> &waypoints() const {return _waypoints;}
	const QList<Area> &areas() const {return _polygons;}

	/* Time span of all track points that have a timestamp, computed from the
	   unfiltered data. Both values are null when no track point has time. */
	const QPair<QDateTime, QDateTime> &timeRange() const {return _timeRange;}

	static QString formats();
	static QStringList filter();

	/* Display-only time filter applied to the track points of all Data
	   objects created afterwards. A null from/to means "unbounded". Tracks
	   without any timestamps are never filtered. */
	static void setTimeFilter(bool enabled, const QDateTime &from,
	  const QDateTime &to);

private:
	void processData(const QList<TrackData> &trackData,
	  const QList<RouteData> &routeData);
	void updateTimeRange(const TrackData &data);
	static TrackData filterTrack(const TrackData &data);

	bool _valid;
	QString _errorString;
	int _errorLine;

	QList<Track> _tracks;
	QList<Route> _routes;
	QList<Area> _polygons;
	QVector<Waypoint> _waypoints;
	QPair<QDateTime, QDateTime> _timeRange;

	static QMultiMap<QString, Parser*> _parsers;
	static bool _timeFilter;
	static QDateTime _timeFrom;
	static QDateTime _timeTo;
#ifdef Q_OS_ANDROID
	static bool _permChecked;
#endif // Q_OS_ANDROID
};

#endif // DATA_H
