/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtLocation module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#pragma once
//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
#include <QElapsedTimer>
#include <QTouchEvent>
#include <QtCore/QPointer>
#include <QtGui/QVector2D>
#include <QtLocation/private/qdeclarativegeomap_p.h>
#include <QtLocation/private/qlocationglobal_p.h>
#include <QtPositioning/qgeocoordinate.h>
#include <QtQuick/QQuickItem>
class QGraphicsSceneMouseEvent;
class QQuickGeoCoordinateAnimation;
class QWheelEvent;
class QGeoMap;
class QGeoMapPinchEvent : public QObject
{
    Q_OBJECT
public:
    QGeoMapPinchEvent(const QPointF& center,
                      qreal angle,
                      const QPointF& point1,
                      const QPointF& point2,
                      int pointCount = 0,
                      bool accepted = true)
          : QObject()
          , m_center(center)
          , m_angle(angle)
          , m_point1(point1)
          , m_point2(point2)
          , m_pointCount(pointCount)
          , m_accepted(accepted)
    {
    }
    QGeoMapPinchEvent()
          : QObject()
          , m_angle(0.0)
          , m_pointCount(0)
          , m_accepted(true)
    {
    }
    QPointF center() const
    {
        return m_center;
    }
    void setCenter(const QPointF& center)
    {
        m_center = center;
    }
    qreal angle() const
    {
        return m_angle;
    }
    void setAngle(qreal angle)
    {
        m_angle = angle;
    }
    QPointF point1() const
    {
        return m_point1;
    }
    void setPoint1(const QPointF& p)
    {
        m_point1 = p;
    }
    QPointF point2() const
    {
        return m_point2;
    }
    void setPoint2(const QPointF& p)
    {
        m_point2 = p;
    }
    int pointCount() const
    {
        return m_pointCount;
    }
    void setPointCount(int count)
    {
        m_pointCount = count;
    }
    bool accepted() const
    {
        return m_accepted;
    }
    void setAccepted(bool a)
    {
        m_accepted = a;
    }

private:
    QPointF m_center;
    qreal m_angle;
    QPointF m_point1;
    QPointF m_point2;
    int m_pointCount;
    bool m_accepted;
};
class QQuickGeoMapGestureArea : public QQuickItem
{
    Q_OBJECT
    Q_FLAGS(AcceptedGestures)
public:
    QQuickGeoMapGestureArea(QDeclarativeGeoMap* map, QQuickItem* parent = nullptr);
    ~QQuickGeoMapGestureArea();
    enum GeoMapGesture
    {
        NoGesture = 0x0000,
        PinchGesture = 0x0001,
        PanGesture = 0x0002,
        FlickGesture = 0x0004,
        RotationGesture = 0x0008,
        TiltGesture = 0x0010
    };
    Q_DECLARE_FLAGS(AcceptedGestures, GeoMapGesture)
    AcceptedGestures acceptedGestures() const;
    void setAcceptedGestures(AcceptedGestures acceptedGestures);
    bool isPinchActive() const;
    bool isRotationActive() const;
    bool isTiltActive() const;
    bool isPanActive() const;
    bool isActive() const;
    bool enabled() const;
    void setEnabled(bool enabled);

    bool hoverEnabled() const;
    void setHoverEnabled(bool value);

    qreal maximumZoomLevelChange() const;
    void setMaximumZoomLevelChange(qreal maxChange);
    qreal flickDeceleration() const;
    void setFlickDeceleration(qreal deceleration);
    void handleTouchEvent(QTouchEvent* event);
#if QT_CONFIG(wheelevent)
    Q_INVOKABLE void handleWheelEvent(QWheelEvent* event);
#endif
    Q_INVOKABLE void handleMousePressEvent(QMouseEvent* event);
    Q_INVOKABLE void handleMouseMoveEvent(QMouseEvent* event);
    Q_INVOKABLE void handleMouseReleaseEvent(QMouseEvent* event);
    Q_INVOKABLE void handleMouseUngrabEvent();
    Q_INVOKABLE void handleTouchUngrabEvent();
    void setMinimumZoomLevel(qreal min);
    qreal minimumZoomLevel() const;
    void setMaximumZoomLevel(qreal max);
    qreal maximumZoomLevel() const;
    void setMap(QGeoMap* map);
    bool preventStealing() const;
    void setPreventStealing(bool prevent);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseUngrabEvent() override;
    void touchUngrabEvent() override;
    void touchEvent(QTouchEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
#if QT_CONFIG(wheelevent)
    void wheelEvent(QWheelEvent* event) override;
#endif
signals:
    void panActiveChanged();
    void pinchActiveChanged();
    void rotationActiveChanged();
    void tiltActiveChanged();
    void enabledChanged();
    void maximumZoomLevelChangeChanged();
    void acceptedGesturesChanged();
    void flickDecelerationChanged();
    void pinchStarted(QGeoMapPinchEvent* pinch);
    void pinchUpdated(QGeoMapPinchEvent* pinch);
    void pinchFinished(QGeoMapPinchEvent* pinch);
    void panStarted();
    void panFinished();
    void flickStarted();
    void flickFinished();
    void rotationStarted(QGeoMapPinchEvent* pinch);
    void rotationUpdated(QGeoMapPinchEvent* pinch);
    void rotationFinished(QGeoMapPinchEvent* pinch);
    void tiltStarted(QGeoMapPinchEvent* pinch);
    void tiltUpdated(QGeoMapPinchEvent* pinch);
    void tiltFinished(QGeoMapPinchEvent* pinch);
    void preventStealingChanged();
    void clicked(QPoint position, Qt::MouseButton button);
    void doubleClicked(QPoint position, Qt::MouseButton button);
    void positionChanged(QPointF position);
    void hoverEnabledChanged();

private:
    void update();
    // Create general data relating to the touch points
    void touchPointStateMachine();
    void startOneTouchPoint();
    void updateOneTouchPoint();
    void startTwoTouchPoints();
    void updateTwoTouchPoints();
    // All two fingers vertical parallel panning related code, which encompasses tilting
    void tiltStateMachine();
    bool canStartTilt();
    void startTilt();
    void updateTilt();
    void endTilt();
    // All two fingers rotation related code, which encompasses rotation
    void rotationStateMachine();
    bool canStartRotation();
    void startRotation();
    void updateRotation();
    void endRotation();
    // All pinch related code, which encompasses zoom
    void pinchStateMachine();
    bool canStartPinch();
    void startPinch();
    void updatePinch();
    void endPinch();
    // Pan related code (regardles of number of touch points),
    // includes the flick based panning after letting go
    void panStateMachine();
    bool canStartPan();
    void updatePan();
    bool tryStartFlick();
    void startFlick(int dx, int dy, int timeMs = 0);
    void stopFlick();
    bool pinchEnabled() const;
    void setPinchEnabled(bool enabled);
    bool rotationEnabled() const;
    void setRotationEnabled(bool enabled);
    bool tiltEnabled() const;
    void setTiltEnabled(bool enabled);
    bool panEnabled() const;
    void setPanEnabled(bool enabled);
    bool flickEnabled() const;
    void setFlickEnabled(bool enabled);
public slots:
    void handleFlickAnimationStopped();

private:
    void stopPan();
    void clearTouchData();
    void updateFlickParameters(const QPointF& pos);

private:
    QGeoMap* m_map;
    QDeclarativeGeoMap* m_declarativeMap;
    bool m_enabled;
    // This should be intended as a "two fingers gesture" struct
    struct Pinch
    {
        Pinch()
              : m_pinchEnabled(true)
              , m_rotationEnabled(true)
              , m_tiltEnabled(true)
              , m_startDist(0)
              , m_lastAngle(0.0)
        {
        }
        QGeoMapPinchEvent m_event;
        bool m_pinchEnabled;
        bool m_rotationEnabled;
        bool m_tiltEnabled;
        struct Zoom
        {
            Zoom()
                  : m_minimum(0.0)
                  , m_maximum(30.0)
                  , m_start(0.0)
                  , m_previous(0.0)
                  , maximumChange(4.0)
            {
            }
            qreal m_minimum;
            qreal m_maximum;
            qreal m_start;
            qreal m_previous;
            qreal maximumChange;
        } m_zoom;
        struct Rotation
        {
            Rotation()
                  : m_startBearing(0.0)
                  , m_previousTouchAngle(0.0)
                  , m_totalAngle(0.0)
            {
            }
            qreal m_startBearing;
            qreal m_previousTouchAngle;  // needed for detecting crossing +- 180 in a safer way
            qreal m_totalAngle;
        } m_rotation;
        struct Tilt
        {
            Tilt()
            {
            }
            QPointF m_startTouchCentroid;
            qreal m_startTilt;
        } m_tilt;
        QPointF m_lastPoint1;
        QPointF m_lastPoint2;
        qreal m_startDist;
        qreal m_lastAngle;
    } m_pinch;
    AcceptedGestures m_acceptedGestures;
    struct Pan
    {
        Pan()
              : m_maxVelocity(2500)
              , m_deceleration(2500)
              , m_animation(0)
              , m_flickEnabled(true)
              , m_panEnabled(true)
        {
        }
        qreal m_maxVelocity;
        qreal m_deceleration;
        QQuickGeoCoordinateAnimation* m_animation;
        bool m_flickEnabled;
        bool m_panEnabled;
    } m_flick;
    // these are calculated regardless of gesture or number of touch points
    QVector2D m_flickVector;
    QElapsedTimer m_lastPosTime;
    QPointF m_lastPos;
    QVector<QTouchEvent::TouchPoint> m_allPoints;
    QVector<QTouchEvent::TouchPoint> m_touchPoints;
    QScopedPointer<QTouchEvent::TouchPoint> m_mousePoint;
    QPointF m_sceneStartPoint1;
    // only set when two points in contact
    QPointF m_sceneStartPoint2;
    QGeoCoordinate m_startCoord;
    QGeoCoordinate m_touchCenterCoord;
    qreal m_twoTouchAngle;
    qreal m_twoTouchAngleStart;
    qreal m_distanceBetweenTouchPoints;
    qreal m_distanceBetweenTouchPointsStart;
    QPointF m_twoTouchPointsCentroidStart;
    QPointF m_touchPointsCentroid;
    bool m_preventStealing;
    bool m_panEnabled;

private:
    // prototype state machine...
    enum TouchPointState
    {
        touchPoints0,
        touchPoints1,
        touchPoints2
    } m_touchPointState;
    enum PinchState
    {
        pinchInactive,
        pinchInactiveTwoPoints,
        pinchActive
    } m_pinchState;
    enum RotationState
    {
        rotationInactive,
        rotationInactiveTwoPoints,
        rotationActive
    } m_rotationState;
    enum TiltState
    {
        tiltInactive,
        tiltInactiveTwoPoints,
        tiltActive
    } m_tiltState;
    enum FlickState
    {
        flickInactive,
        panActive,
        flickActive
    } m_flickState;
    inline void setTouchPointState(const TouchPointState state);
    inline void setFlickState(const FlickState state);
    inline void setTiltState(const TiltState state);
    inline void setRotationState(const RotationState state);
    inline void setPinchState(const PinchState state);
};
