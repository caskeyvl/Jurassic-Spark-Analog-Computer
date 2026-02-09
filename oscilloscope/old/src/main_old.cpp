#include <QApplication>
#include <QVBoxLayout>
#include <QScreen>
#include <QMainWindow>
#include <QPushButton> 

int main(int argc, char *argv[]){
	QApplication app(argc, argv); 
	
	// Set mouse to act like touch
	app.setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents, true); 

	// Create main window
	QMainWindow w;

	// Central widget and layout
	auto *central = new QWidget(&w); 
	auto *layout = new QVBoxLayout(central); 
	layout->setContentsMargins(10, 10, 10, 10); 
	layout->setSpacing(20); 

	// Touch button
	auto *btn = new QPushButton("TAP", central); 
	btn->setMinimumSize(40,20); 
	btn->setStyleSheet(
		"QPushButton { font-size: 48px; padding: 24px; }"
		"QPushButton:pressed { opacity: 0.7; }"
	); 

	layout->addStretch(1); 
	layout->addWidget(btn, 0, Qt::AlignCenter);
	layout->addStretch(1);

	w.setCentralWidget(central); 
	
	// Toggle text on tap
	QObject::connect(btn, &QPushButton::clicked, [&](){
		btn->setText(btn->text() == "Tap" ? "tapped!" : "Tap");
	}); 

	// Check if kiosk mode or running on laptop for testing
	const bool kiosk =
		app.arguments().contains("--kiosk") || 
		qEnvironmentVariableIsSet("KIOSK");

	if(kiosk) { 
		w.showFullScreen(); 
	} else { 
		w.resize(1920, 1080);
		w.show(); 
	}
	return app.exec();
}
