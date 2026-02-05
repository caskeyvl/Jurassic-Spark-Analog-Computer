#include "mainwindow.h" 
#include "ui_mainwindow.h"

#include <QtCharts/qlineseries.h>
#include <QFile>
#include <QDebug>
#include <QQuickWidget> 
#include <QVBoxLayout> 
#include <QMetaObject>
#include <QVariant> 
#include <QPushButton> 
#include <QMetaObject>
#include <QString>
#include <QQuickItem>
#include <QTimer>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent),
	ui(new Ui::MainWindow) {

	ui->setupUi(this); 
	
	// Timer connections
	connect(&sampleTimer, &QTimer::timeout, this, &MainWindow::onSampleTick); 
	sampleTimer.setTimerType(Qt::PreciseTimer); 
	sampleTimer.setInterval(16); 

	screen = ui->quickWaveform;
	initQml(); 
	initConnections(); 

	updateButtonStyles(); 
	updateScopeView(); 
}

MainWindow::~MainWindow() {
	delete ui;
}

void MainWindow::initQml() { 
	screen->setResizeMode(QQuickWidget::SizeRootObjectToView);

	connect(screen, &QQuickWidget::statusChanged, this, [this](QQuickWidget::Status s){ 
		if(s != QQuickWidget::Ready) return;

		QObject* root = screen->rootObject();
		qWarning() << "root: " << root;
		QObject* obj = root ? root->findChild<QObject*>("ch1Series") : nullptr;

		qWarning() << "obj" << obj
			 << "class" << (obj ? obj->metaObject()->className() : "null")
			 << "name" << (obj ? obj->objectName() : ""); 

		if(!obj) return;

		series[0] = qobject_cast<QLineSeries*>(obj); 
		qWarning() << "cast series[0]:" << series[0];
		if(!series[0]) {
			qInfo() << "Not QLineSeries. Got: " << obj->metaObject()->className();
			return; 
		}

		points[0].resize(N);
		for (int i = 0; i < N; ++i) points[0][i] = QPointF(i, 0.0);
		series[0]->replace(points[0]); 


		qWarning() << "QML Ready, starting timer";
		startStreaming(); 
	});

	screen->setSource(QUrl("qrc:/qml/scopeScreen.qml")); 
	if(screen->status() == QQuickWidget::Ready) {
		QMetaObject::invokeMethod(screen, "statusChanged", Qt::QueuedConnection,
				Q_ARG(QQuickWidget::Status, QQuickWidget::Ready));
	}
}

void MainWindow::initConnections() { 
	auto toggle = [this](int i) { 
		chOn[i] = !chOn[i]; 
		updateButtonStyles(); 
		updateScopeView(); 
	};

	connect(ui->btnCh1, &QPushButton::clicked, this, [=]{ toggle(0); });
	connect(ui->btnCh2, &QPushButton::clicked, this, [=]{ toggle(1); });
	connect(ui->btnCh3, &QPushButton::clicked, this, [=]{ toggle(2); });
	connect(ui->btnCh4, &QPushButton::clicked, this, [=]{ toggle(3); });
}

void MainWindow::updateButtonStyles() {
	auto style = [](QPushButton *b, bool on, const char *color) {
		b->setCheckable(true); 
		b->setChecked(on); 
		b->setStyleSheet(on ? QString("background:%1;").arg(color) : ""); 
	};

	style(ui->btnCh1, chOn[0], "#00ff00"); 		// set screen green
	style(ui->btnCh2, chOn[1], "#00aaff"); 		// set screen blue
	style(ui->btnCh3, chOn[2], "#ffaa00"); 		// set screen orange
	style(ui->btnCh4, chOn[3], "#ff00ff"); 		// set screen magenta
}

void MainWindow::updateScopeView() {
	if (!series[0]) return; 
	
	bool anyOn = false;
	for(bool on : chOn) anyOn |= on; 
	if(!anyOn) { 
	series[0]->clear();
	}
}
void MainWindow::callQml(const char *method, const QVariant &arg) { 
	auto *w = ui->quickWaveform; 
	if(!w) return; 

	QObject *root = w->rootObject();
	if (!root) return; 
	if(arg.isValid())
		QMetaObject::invokeMethod(root, method, Q_ARG(QVariant, arg)); 
	else
		QMetaObject::invokeMethod(root, method); 
}
void MainWindow::startStreaming() { 
	
	sampleTimer.start(); 
	qInfo() << "timer active?" << sampleTimer.isActive();
}
void MainWindow::stopStreaming() { 
	sampleTimer.stop(); 
}

void MainWindow::onSampleTick() { 
	if (!series[0]) return; 

	bool anyOn = true; 
	for(bool on : chOn) anyOn |= on; 
	if(!anyOn) { 
		series[0]->clear(); 
		return; 
	}

	// Mock Generator: sine + bit of drift (replace with ADC)
	// We'll "scroll" by writing into a ring buffer and then remapping X
	const int N = points[0].size(); 
	const double freq = 3.0; 		// cyclesch1.clear() across buffer-ish (tweak)
	const double step = 0.08;		// "sample rate" knob
	phase += step; 

	double y = std::sin(phase * freq);

	points[0][writeIndex].setY(y);
	writeIndex = (writeIndex + 1) % N;

	// Rebuild an ordered view for replace() so line is continuous
	// Needing optimization later (most likely)
	
	QVector<QPointF> ordered; 
	ordered.reserve(N); 
	
	int idx = writeIndex;
	for (int x = 0; x < N; ++x) { 
	ordered.push_back(QPointF(x, points[0][idx].y()));
	idx = (idx + 1) % N;
	}

	series[0]->replace(ordered);
}
