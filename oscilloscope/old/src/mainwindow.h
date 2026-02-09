#pragma once

#include <QMainWindow> 
#include <array> 
#include <QTimer>
#include <QVector>
#include <QPointF>

class QLineSeries;
class QQuickWidget;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; } 
QT_END_NAMESPACE

class MainWindow : public QMainWindow { 
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow(); 

private:
	Ui::MainWindow *ui; 
	QQuickWidget *screen = nullptr; 

	std::array<bool, 4> chOn{false, false, false, false}; 

	// Streaming / plotting
	QTimer sampleTimer;
	QLineSeries* series[4] = {nullptr, nullptr, nullptr, nullptr};

	QVector<QPointF> points[4];
	int writeIndex = 0; 
	double phase = 0.0; 

	int N = 1000; 

	void initQml();
	void initConnections(); 
	void updateScopeView(); 
	void updateButtonStyles(); 
	void callQml(const char *method, const QVariant &arg = {}); 
	void startStreaming(); 
	void stopStreaming(); 

	private slots:
		void onSampleTick(); 
};
