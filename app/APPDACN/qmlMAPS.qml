import QtQuick 2.0
import QtLocation 5.11
import QtPositioning 5.11

Rectangle {
    id:window

    property double latitude_1:11.537543
    property double longitude_1:106.901618

    property double latitude_2:11.537543
    property double longitude_2:106.901618
    property double distanceMeters: 0
    property var marker2Obj: null
    property Component locationmarker_1: locmarker_1
    property Component locationmarker_2: locmarker_2
    Plugin
    {
        id:googlemapview
        name:"osm"
    }
    Map
    {
        id:mapview
        anchors.fill: parent
        plugin: googlemapview
        center: QtPositioning.coordinate(latitude_1,longitude_1)
        zoomLevel: 10
        minimumZoomLevel: 8
        maximumZoomLevel: 15
//        center: QtPositioning.coordinate(latitude_1, longitude_1)
        MapPolyline
        {
            line.width: 3
            line.color: "red"
            path: [
                QtPositioning.coordinate(latitude_1, longitude_1),
                QtPositioning.coordinate(latitude_2, longitude_2)
            ]
        }
        MapQuickItem {
            id: distanceLabel
            coordinate: QtPositioning.coordinate(
                (latitude_1 + latitude_2) / 2,
                (longitude_1 + longitude_2) / 2
            )

            anchorPoint.x: labelRect.width / 2
            anchorPoint.y: labelRect.height / 2

            sourceItem: Rectangle {
                id: labelRect
                radius: 6
                color: "#AA000000"

                width: textItem.width + 12
                height: textItem.height + 12

                Text {
                    id: textItem
                    anchors.centerIn: parent
                    color: "white"
                    font.bold: true
                    text: distanceMeters > 1000
                          ? (distanceMeters / 1000).toFixed(2) + " km"
                          : Math.round(distanceMeters) + " m"
                }
            }
        }
    }
    function setDistance(meters) {
        distanceMeters = meters
    }
    function setCenterPosition(lati, longi)
    {
        latitude_1 = lati
        longitude_1 = longi
        mapview.center = QtPositioning.coordinate(lati, longi)
    }

    function setLocationMarking_1(lati, longi)
    {
        var item = locationmarker_1.createObject(window, {
                                      coordinate:QtPositioning.coordinate(lati, longi)
                                      })
        mapview.addMapItem(item)
    }

    function setLocationMarking_2(lati, longi)
    {
        latitude_2 = lati
        longitude_2 = longi

        if (marker2Obj !== null)
            marker2Obj.destroy()

        marker2Obj = locationmarker_2.createObject(window, {
            coordinate: QtPositioning.coordinate(lati, longi)
        })
        mapview.addMapItem(marker2Obj)

        // Auto fit map
        mapview.fitViewportToMapItems()
    }

    Component
    {
        id: locmarker_1
        MapQuickItem
        {
            id: markerImg
            anchorPoint.x: image.width/4
            anchorPoint.y: image.height
            coordinate: position
            sourceItem: Image {
                id: image
                width: 20
                height: 20
                source: "observer.png"
            }
        }
    }

    Component
    {
        id: locmarker_2
        MapQuickItem
        {
            id: markerImg
            anchorPoint.x: image.width/4
            anchorPoint.y: image.height
            coordinate: QtPositioning.coordinate(latitude_2, longitude_2)
            sourceItem: Image {
                id: image
                width: 20
                height: 20
                source: "target.png"
            }
        }
    }
}
