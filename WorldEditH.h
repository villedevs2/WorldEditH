#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_WorldEditH.h"

class WorldEditH : public QMainWindow
{
	Q_OBJECT

public:
	WorldEditH(QWidget *parent = Q_NULLPTR);

private:
	Ui::WorldEditHClass ui;
};
