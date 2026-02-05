import QtQuick 
import QtCharts

Rectangle { 
	id: root
	anchors.fill: parent
	color: "black"
	
	property color traceColor: "red"
	property bool live: false

	ChartView {
		id: chart
		anchors.fill: parent
		antialiasing: true
		backgroundColor: "black"
		legend.visible: false

		ValueAxis{ id: axX; min: 0; max: 999 } 
		ValueAxis{ id: axY; min: -1.2; max: 1.2} 

		LineSeries { 
			id: ch1  
			objectName: "ch1Series"
			axisX: axX
			axisY: axY
			color: "#00ff00"
			width: 2
		}
		LineSeries { 
			id: ch2  
			objectName: "ch2Series"
			axisX: axX
			axisY: axY
			color: "#00aaff"
			width: 2
		}
		LineSeries { 
			id: ch3
			objectName: "ch3Series"
			axisX: axX
			axisY: axY
			color: "#ffaa00"
			width: 2
		}
		LineSeries { 
			id: ch4
			objectName: "ch4Series"
			axisX: axX
			axisY: axY
			color: "#ff00ff"
			width: 2
		}
	}

	function setTraceColor(c) {
		root.traceColor = c
	}


	function clearAllTraces(channel) {
		ch1.clear()
		ch2.clear()
		ch3.clear()
		ch4.clear()
	}
}
