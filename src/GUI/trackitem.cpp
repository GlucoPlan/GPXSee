#include <cmath>
#include <QLocale>
#include <QCoreApplication>
#include "common/util.h"
#include "data/track.h"
#include "format.h"
#include "tooltip.h"
#include "trackitem.h"

ToolTip TrackItem::info(bool extended) const
{
	ToolTip tt;
	QLocale l;

	if (!_name.isEmpty())
		tt.insert(tr("Name"), _name);
	if (!_desc.isEmpty())
		tt.insert(tr("Description"), _desc);
	if (!_comment.isEmpty() && _comment != _desc)
		tt.insert(tr("Comment"), _comment);
	tt.insert(tr("Distance"), Format::distance(path().last().last().distance(),
	  _units));
	if  (_time > 0)
		tt.insert(tr("Total time"), Format::timeSpan(_time));
	if  (_movingTime > 0)
		tt.insert(tr("Moving time"), Format::timeSpan(_movingTime));
	if (!_date.isNull()) {
		QDateTime date(_date.toTimeZone(_timeZone));
		tt.insert(tr("Date"), l.toString(date.date(), QLocale::ShortFormat)
		  + " " + date.time().toString("h:mm:ss"));
	}
	if (!_links.isEmpty()) {
		QString links;
		for (int i = 0; i < _links.size(); i++) {
			const Link &link = _links.at(i);
			links.append(QString("<a href=\"%0\">%1</a>").arg(link.URL(),
			  link.text().isEmpty() ? link.URL() : link.text()));
			if (i != _links.size() - 1)
				links.append("<br/>");
		}
		tt.insert(tr("Links"), links);
	}
#ifdef Q_OS_ANDROID
	Q_UNUSED(extended);
#else // Q_OS_ANDROID
	if ((extended || _video) && !_file.isEmpty())
		tt.insert(tr("File"), QString("<a href=\"file:%1\">%2</a>")
		  .arg(_file, QFileInfo(_file).fileName()));
#endif // Q_OS_ANDROID

	return tt;
}

TrackItem::TrackItem(const Track &track, Map *map, QGraphicsItem *parent)
  : PathItem(track.path(), map, parent)
{
	_name = track.name();
	_desc = track.description();
	_comment = track.comment();
	_links = track.links();
	_date = track.date();
	_time = track.time();
	_movingTime = track.movingTime();
	_file = track.file();
	_video = track.isVideo();
	_points = track.pointInfo();
}

static QString speedString(qreal speed, Units units, const QLocale &l)
{
	if (units == Nautical)
		return l.toString(MS2KN * speed, 'f', 1) + UNIT_SPACE
		  + QCoreApplication::translate("MotionInfoItem", "kn");
	else if (units == Imperial)
		return l.toString(MS2MIH * speed, 'f', 1) + UNIT_SPACE
		  + QCoreApplication::translate("MotionInfoItem", "mi/h");
	else
		return l.toString(MS2KMH * speed, 'f', 1) + UNIT_SPACE
		  + QCoreApplication::translate("MotionInfoItem", "km/h");
}

bool TrackItem::pointInfo(int seg, int idx, ToolTip &tt) const
{
	if (seg < 0 || seg >= _points.size() || idx < 0
	  || idx >= _points.at(seg).size())
		return false;

	const TrackPointInfo &pi = _points.at(seg).at(idx);
	const Trackpoint &p = pi.point;
	QLocale l;

	if (p.hasTimestamp()) {
		QDateTime date(p.timestamp().toTimeZone(_timeZone));
		tt.insert(tr("Point time"), l.toString(date.date(),
		  QLocale::ShortFormat) + " " + date.time().toString("h:mm:ss"));
	}
	tt.insert(tr("Coordinates"), Format::coordinates(p.coordinates(),
	  _coordinatesFormat));
	if (p.hasElevation())
		tt.insert(tr("Elevation"), Format::elevation(p.elevation(), _units));
	if (!std::isnan(pi.speed))
		tt.insert(tr("Speed"), speedString(pi.speed, _units, l));
	tt.insert(tr("Distance from start"), Format::distance(pi.distance,
	  _units));
	if (!std::isnan(pi.time))
		tt.insert(tr("Time from start"), Format::timeSpan(pi.time));
	if (p.hasHeartRate())
		tt.insert(tr("Heart rate"), l.toString(qRound(p.heartRate()))
		  + UNIT_SPACE + QCoreApplication::translate("HeartRateGraph", "bpm"));
	if (p.hasCadence())
		tt.insert(tr("Cadence"), l.toString(qRound(p.cadence()))
		  + UNIT_SPACE + QCoreApplication::translate("CadenceGraph", "rpm"));
	if (p.hasPower())
		tt.insert(tr("Power"), l.toString(qRound(p.power()))
		  + UNIT_SPACE + QCoreApplication::translate("PowerGraph", "W"));
	if (p.hasTemperature()) {
		if (_units == Metric)
			tt.insert(tr("Temperature"), l.toString(p.temperature(), 'f', 1)
			  + UNIT_SPACE + QChar(0x00B0)
			  + QCoreApplication::translate("TemperatureGraph", "C"));
		else
			tt.insert(tr("Temperature"), l.toString(p.temperature() * C2FS
			  + C2FO, 'f', 1) + UNIT_SPACE + QChar(0x00B0)
			  + QCoreApplication::translate("TemperatureGraph", "F"));
	}

	return true;
}
