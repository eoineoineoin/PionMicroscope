#include <ViewerWindow.h>
#include <QApplication>

int main(int argc, char** argv)
{
	QApplication a(argc, argv);
	ViewerWindow window;
	window.show();
	a.exec();
}
