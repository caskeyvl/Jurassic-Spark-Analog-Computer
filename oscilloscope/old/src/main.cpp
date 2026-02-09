#include <QApplication>
#include <QPalette>
#include "mainwindow.h"

static void setDarkPalette(QApplication &app) { 
    QPalette p;
    p.setColor(QPalette::Window, QColor(30,30,30));
    p.setColor(QPalette::WindowText, Qt::white);
    p.setColor(QPalette::Base, QColor(20,20,20));
    p.setColor(QPalette::AlternateBase, QColor(30,30,30));
    p.setColor(QPalette::ToolTipBase, Qt::white);
    p.setColor(QPalette::ToolTipText, Qt::white);
    p.setColor(QPalette::Text, Qt::white);
    p.setColor(QPalette::Button, QColor(45,45,45));
    p.setColor(QPalette::ButtonText, Qt::white);
    p.setColor(QPalette::BrightText, Qt::red);
    p.setColor(QPalette::Highlight, QColor(90,140,255));
    p.setColor(QPalette::HighlightedText, Qt::black);

    app.setPalette(p);
}

int main(int argc, char *argv[]) {
	QApplication app(argc, argv); 
	
	qInfo() << "MAIN STARTED"; 

	MainWindow w; 
	
	// Check if in kiosk mode or running on laptop for testing
	const bool kiosk = 
		app.arguments().contains("--kiosk") || 
		qEnvironmentVariableIsSet("KIOSK"); 

	if(kiosk) {
		w.showFullScreen();
	} else {
		w.show();
	}

	setDarkPalette(app);

	return app.exec(); 
}
