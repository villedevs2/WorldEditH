#include "WorldEditH.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	WorldEditH w;
	w.show();
	return a.exec();
}
