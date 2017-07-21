#include "MainWindow.h"

#include "QApplication"
#include "QtGui"

MainWindow::MainWindow()
{
	m_texture = new QImage();

	m_level = new Level();
	readConfigFile(tr("config.xml"));

	QWidget *central_widget = new QWidget();

	// main components
	// ---------------------------------------------------------------------------

	m_glwidget = new GLWidget(this, m_level);
	m_objbrowser = new ObjectBrowser(this, m_level);
	
	m_texedit = new TextureEdit(this, m_level);

	m_objedit = new ObjectEdit(this, m_level);
	
	m_tiledesigner = new TileDesigner(this, m_level);

	m_preview = new PreviewWindow(this, m_level);
	

	m_visbox_conf = new VisboxConf(this);

	m_tileset_window = new TilesetWindow(this, m_level, m_texture);

	m_level_conf = new LevelConf(this);

	// side widget
	// ---------------------------------------------------------------------------
	QWidget *side_widget = new QWidget();
	
	QBoxLayout *side_layout = new QBoxLayout(QBoxLayout::TopToBottom);
	side_layout->setSpacing(3);
	side_layout->setMargin(0);

	side_widget->setLayout(side_layout);

	m_objbrowser->setMinimumSize(300, 410);
	m_objbrowser->setMaximumWidth(400);

	
	// main layout
	// ---------------------------------------------------------------------------
	QGridLayout *layout = new QGridLayout();
	layout->setSpacing(3);
	layout->setMargin(3);

	layout->addWidget(m_glwidget, 0, 0);
	layout->addWidget(m_objbrowser, 0, 1);


	central_widget->setLayout(layout);
	setCentralWidget(central_widget);


	// connect signals and slots
	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), m_glwidget, SLOT(animate()));
	timer->start(50);


	connect(m_glwidget, SIGNAL(onAddObject(int)), m_objbrowser, SLOT(add(int)));

	connect(m_glwidget, SIGNAL(onRemoveObject(int)), m_objbrowser, SLOT(remove(int)));
	connect(m_glwidget, SIGNAL(onRemoveObject(int)), m_texedit, SLOT(remove(int)));
	connect(m_glwidget, SIGNAL(onRemoveObject(int)), m_objedit, SLOT(remove(int)));
	
	connect(m_glwidget, SIGNAL(onSelectObject(int)), m_objbrowser, SLOT(select(int)));
	connect(m_glwidget, SIGNAL(onSelectObject(int)), m_texedit, SLOT(select(int)));
	connect(m_glwidget, SIGNAL(onSelectObject(int)), m_objedit, SLOT(select(int)));
	
	connect(m_glwidget, SIGNAL(onDeselectObject()), m_objbrowser, SLOT(deselect()));
	connect(m_glwidget, SIGNAL(onDeselectObject()), m_texedit, SLOT(deselect()));
	connect(m_glwidget, SIGNAL(onDeselectObject()), m_objedit, SLOT(deselect()));
	
	connect(m_glwidget, SIGNAL(onSetMode(GLWidget::OperationMode)), m_objedit, SLOT(setMode(GLWidget::OperationMode)));
	
	connect(m_objbrowser, SIGNAL(onSelectObject(int)), m_glwidget, SLOT(select(int)));
	connect(m_objbrowser, SIGNAL(onSelectObject(int)), m_texedit, SLOT(select(int)));
	connect(m_objbrowser, SIGNAL(onSelectObject(int)), m_objedit, SLOT(select(int)));

	connect(m_objbrowser, SIGNAL(onObjectDataChanged(int)), m_objedit, SLOT(objectDataChanged(int)));
	connect(m_objedit, SIGNAL(onObjectDataChanged(int)), m_objbrowser, SLOT(objectDataChanged(int)));
	
	connect(m_texedit, SIGNAL(onClose()), this, SLOT(texEditClosed()));
	connect(m_objedit, SIGNAL(onClose()), this, SLOT(objEditClosed()));
	connect(m_tiledesigner, SIGNAL(onClose()),this, SLOT(tileDesignerClosed()));
	connect(m_tileset_window, SIGNAL(onClose()), this, SLOT(tilesetWindowClosed()));
	connect(m_preview, SIGNAL(onClose()), this, SLOT(previewClosed()));

	connect(m_objedit, SIGNAL(onSetCreateType(Level::ObjectType)), m_glwidget, SLOT(setCreateType(Level::ObjectType)));
	connect(m_objedit, SIGNAL(onSetCreateTriggerType(int)), m_glwidget, SLOT(setCreateTriggerType(int)));

	connect(m_tiledesigner, SIGNAL(onInsertTile(int)), m_tileset_window, SLOT(add(int)));
	connect(m_tiledesigner, SIGNAL(onReplaceTile(int)), m_tileset_window, SLOT(replace(int)));

	connect(m_tileset_window, SIGNAL(onSelectTile(int)), m_glwidget, SLOT(setTileBrush(int)));
	connect(m_tileset_window, SIGNAL(onSelectTile(int)), m_tiledesigner, SLOT(tileSelected(int)));
	connect(m_glwidget, SIGNAL(onTileUpdate(int, int)), m_preview, SLOT(tileUpdated(int, int)));

	// zoom shortcuts
	m_zoomin_shortcut = new QShortcut(QKeySequence(Qt::Key_Plus), this);
	m_zoomout_shortcut = new QShortcut(QKeySequence(Qt::Key_Minus), this);
	connect(m_zoomin_shortcut, SIGNAL(activated()), this, SLOT(zoomIn()));
	connect(m_zoomout_shortcut, SIGNAL(activated()), this, SLOT(zoomOut()));



	// create various stuff
	createActions();
	createMenus();
	createToolbars();
	createStatusbar();

	setCurrentFile(tr(""));
	m_texture_file = "";
	
	// go to selection mode by default
	m_select_action->setChecked(true);


	// object editor positioning
	m_objedit->move(QPoint(width() + 120, 100));

	// object designer positioning
	m_tiledesigner->move(QPoint(width() - 600, 230));

	// tileset window positioning
	m_tileset_window->move(QPoint(width() + 850, 600));

	// preview window positioning
	m_preview->move(QPoint(width() + 250, 100));


	// tex edit hidden by default
	m_texedit_open = false;
	m_toggle_texedit->setChecked(false);
	m_texedit->setHidden(true);

	// obj edit shown by default
	m_objedit_open = true;
	m_toggle_objedit->setChecked(true);
	m_objedit->setHidden(false);

	// tile designer hidden by default
	m_tiledesigner_open = false;
	m_toggle_tiledesigner->setChecked(false);
	m_tiledesigner->setHidden(true);

	// tileset shown by default
	m_tileset_window_open = false;
	m_toggle_tileset_window->setChecked(false);
	m_tileset_window->setHidden(true);

	// preview shown by default
	m_preview_open = true;
	m_toggle_preview->setChecked(true);
	m_preview->setHidden(false);

	
	// grid settings
	// TODO: load settings
	m_gridSizeCombo->setCurrentIndex(3);
	emit m_glwidget->setGridSize(3);

	m_enable_grid = false;
	m_toggleGridAction->setChecked(false);
	m_snap_grid = false;
	m_snapGridAction->setChecked(false);

	m_glwidget->enableGrid(m_enable_grid);
	m_glwidget->setSnapGrid(m_snap_grid);


	// visbox
	m_toggle_visbox->setChecked(false);
	m_enable_visbox = false;
	m_glwidget->enableVisbox(m_enable_visbox);


	// zoom settings
	m_zoomLevelCombo->setCurrentIndex(3);
	emit m_glwidget->setZoomLevel(3);


	// default type
	m_defTypeCombo->setCurrentIndex(0);
	emit m_objedit->setDefaultType(Level::OBJECT_TYPE_TRIGGER);

	// tilemap settings
	int xs = 0;
	int ys = 0;
	int xe = 50;
	int ye = 50;
	float tw = 1.0f;
	float th = 1.2f;
}

MainWindow::~MainWindow(void)
{
	delete m_level;
}

bool MainWindow::handleUnsaved()
{
	QMessageBox box;
	box.setText("The document has been modified.");
	box.setInformativeText("Do you want to save the changes?");
	box.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
	box.setDefaultButton(QMessageBox::Save);
	int ret = box.exec();

	switch (ret)
	{
		case QMessageBox::Save:
		{
			emit saveFile();
			return true;
		}
		case QMessageBox::Discard:
		{
			return true;
		}
		case QMessageBox::Cancel:
		{
			return false;
		}
		default:
			break;
	}

	return false;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (m_level->isModified())
	{
		bool ret = handleUnsaved();

		if (ret)
			event->accept();
		else
			event->ignore();
	}
}


QString MainWindow::getLevelDir()
{
	const QString LEVEL_DIR("level_dir");
	QSettings settings;

	return settings.value(LEVEL_DIR).toString();
}

void MainWindow::setLevelDir(QString path)
{
	const QString LEVEL_DIR("level_dir");
	QSettings settings;

	QDir dir(path);
	QString level_path = dir.absolutePath();
	QString level_dir = level_path.section("/", 0, -2);

	settings.setValue(LEVEL_DIR, level_dir);
}

QString MainWindow::getTextureDir()
{
	const QString TEXTURE_DIR("texture_dir");
	QSettings settings;

	return settings.value(TEXTURE_DIR).toString();
}

void MainWindow::setTextureDir(QString path)
{
	const QString TEXTURE_DIR("texture_dir");
	QSettings settings;

	QDir dir(path);
	QString tex_path = dir.absolutePath();
	QString tex_dir = tex_path.section("/", 0, -2);

	settings.setValue(TEXTURE_DIR, tex_dir);
}

QString MainWindow::getExportDir()
{
	const QString EXPORT_DIR("export_dir");
	QSettings settings;

	return settings.value(EXPORT_DIR).toString();
}

void MainWindow::setExportDir(QString path)
{
	const QString EXPORT_DIR("export_dir");
	QSettings settings;

	QDir dir(path);
	QString exp_path = dir.absolutePath();
	QString exp_dir = exp_path.section("/", 0, -2);

	settings.setValue(EXPORT_DIR, exp_dir);
}


void MainWindow::newFile()
{
	if (m_level->isModified())
	{
		bool ret = handleUnsaved();

		if (!ret)
		{
			return;
		}
	}
	newDocument();
	resetControls();
}

void MainWindow::openFile()
{
	if (m_level->isModified())
	{
		bool ret = handleUnsaved();

		if (!ret)
		{
			return;
		}
	}


	QString filename = QFileDialog::getOpenFileName(this,
													tr("Open Level File"),
													getLevelDir(),
													tr("Level File (*.hspf);;All Files (*.*)"));

	if (!filename.isEmpty())
	{
		if (readBinaryProjectFile(filename))
		{
			setCurrentFile(filename);
			m_level->resetModify();

			// set current directory
			setLevelDir(filename);
		}
	}
}

bool MainWindow::saveFile()
{
	if (m_open_file.length() <= 0)	// save to a new file
	{
		QString filename = QFileDialog::getSaveFileName(this,
														tr("Save Level File"),
														getLevelDir(),
														tr("Level File (*.hspf);;All Files (*.*)"));
		if (!filename.isEmpty())
		{
			//writeLevelFile(filename);
			writeBinaryProjectFile(filename);
			setCurrentFile(filename);
			m_level->resetModify();

			// set current directory
			setLevelDir(filename);
			return true;
		}
		else
		{
			return false;
		}
	}
	else						// save to existing file
	{
		//writeLevelFile(m_open_file);
		writeBinaryProjectFile(m_open_file);
		m_level->resetModify();
		return true;
	}

	return false;
}

bool MainWindow::saveAsFile()
{
	QString filename = QFileDialog::getSaveFileName(this,
													tr("Save Level File"),
													getLevelDir(),
													tr("Level File (*.hspf);;All Files (*.*)"));

	if (!filename.isEmpty())
	{
		//writeLevelFile(filename);
		writeBinaryProjectFile(filename);
		setCurrentFile(filename);
		m_level->resetModify();

		// set current directory
		setLevelDir(filename);
		return true;
	}
	else
	{
		return false;
	}
}

void MainWindow::exportLevel()
{
	QString filename = QFileDialog::getSaveFileName(this,
													tr("Export Level File"),
													getExportDir(),
													tr("Level Binary (*.level);;All Files (*.*)"));

	if (!filename.isEmpty())
	{
		writeLevelFile(filename);

		// set current directory
		setExportDir(filename);
	}
}

void MainWindow::exitProgram()
{

}


void MainWindow::selectionMode()
{
	emit m_glwidget->setMode(GLWidget::MODE_SELECT);
	m_select_action->setChecked(true);
}

void MainWindow::moveMode()
{
	emit m_glwidget->setMode(GLWidget::MODE_MOVE);
	m_move_action->setChecked(true);
}

void MainWindow::rotateMode()
{
	emit m_glwidget->setMode(GLWidget::MODE_ROTATE);
	m_rotate_action->setChecked(true);
}

void MainWindow::scaleMode()
{
	emit m_glwidget->setMode(GLWidget::MODE_SCALE);
	m_scale_action->setChecked(true);
}

void MainWindow::drawPolyMode()
{
	emit m_glwidget->setMode(GLWidget::MODE_DRAW_POLY);
	m_draw_poly_action->setChecked(true);
}

void MainWindow::drawRectMode()
{
	emit m_glwidget->setMode(GLWidget::MODE_DRAW_RECT);
	m_draw_rect_action->setChecked(true);
}

void MainWindow::tilemapMode()
{
	emit m_glwidget->setMode(GLWidget::MODE_TILEMAP);
	m_tilemap_action->setChecked(true);
}

void MainWindow::tileZEditMode()
{
	emit m_glwidget->setMode(GLWidget::MODE_TILE_ZEDIT);
	m_tile_zedit_action->setChecked(true);
}


void MainWindow::toggleTexEdit()
{
	if (m_texedit_open)
	{
		emit m_texedit->setHidden(true);
		m_texedit_open = false;
		m_toggle_texedit->setChecked(false);
	}
	else
	{
		emit m_texedit->setHidden(false);
		m_texedit_open = true;
		m_toggle_texedit->setChecked(true);
	}
}

void MainWindow::texEditClosed()
{
	m_texedit_open = false;
	m_toggle_texedit->setChecked(false);
}

void MainWindow::toggleObjEdit()
{
	if (m_objedit_open)
	{
		emit m_objedit->setHidden(true);
		m_objedit_open = false;
		m_toggle_objedit->setChecked(false);
	}
	else
	{
		emit m_objedit->setHidden(false);
		m_objedit_open = true;
		m_toggle_objedit->setChecked(true);
	}
}

void MainWindow::objEditClosed()
{
	m_objedit_open = false;
	m_toggle_objedit->setChecked(false);
}

void MainWindow::toggleTileDesigner()
{
	if (m_tiledesigner_open)
	{
		emit m_tiledesigner->setHidden(true);
		m_tiledesigner_open = false;
		m_toggle_tiledesigner->setChecked(false);
	}
	else
	{
		emit m_tiledesigner->setHidden(false);
		m_tiledesigner_open = true;
		m_toggle_tiledesigner->setChecked(true);
	}
}

void MainWindow::tileDesignerClosed()
{
	m_tiledesigner_open = false;
	m_toggle_tiledesigner->setChecked(false);
}

void MainWindow::toggleTilesetWindow()
{
	if (m_tileset_window_open)
	{
		emit m_tileset_window->setHidden(true);
		m_tileset_window_open = false;
		m_toggle_tileset_window->setChecked(false);
	}
	else
	{
		emit m_tileset_window->setHidden(false);
		m_tileset_window_open = true;
		m_toggle_tileset_window->setChecked(true);
	}
}

void MainWindow::tilesetWindowClosed()
{
	m_tileset_window_open = false;
	m_toggle_tileset_window->setChecked(false);
}

void MainWindow::togglePreview()
{
	if (m_preview_open)
	{
		emit m_preview->setHidden(true);
		m_preview_open = false;
		m_toggle_preview->setChecked(false);
	}
	else
	{
		emit m_preview->setHidden(false);
		m_preview_open = true;
		m_toggle_preview->setChecked(true);
	}
}

void MainWindow::previewClosed()
{
	m_preview_open = false;
	m_toggle_preview->setChecked(false);
}

void MainWindow::toggleVisbox()
{
	if (m_enable_visbox)
		m_enable_visbox = false;
	else
		m_enable_visbox = true;
	
	m_glwidget->enableVisbox(m_enable_visbox);
}

void MainWindow::visboxConfig()
{
	if (m_visbox_conf->exec() == QDialog::Accepted)
	{
		float width = m_visbox_conf->getWidth();
		float height = m_visbox_conf->getHeight();

		m_glwidget->configVisbox(width, height);
	}
}

void MainWindow::levelConfig()
{
	if (m_level_conf->exec() == QDialog::Accepted)
	{
		// TODO: set level stuff
	}
}

void MainWindow::toggleGrid()
{
	if (m_enable_grid)
		m_enable_grid = false;
	else
		m_enable_grid = true;

	m_glwidget->enableGrid(m_enable_grid);
}

void MainWindow::snapGrid()
{
	if (m_snap_grid)
		m_snap_grid = false;
	else
		m_snap_grid = true;

	m_glwidget->setSnapGrid(m_snap_grid);
}

void MainWindow::setDefType(int type)
{
	int t = m_defTypeCombo->itemData(type).toInt();
	emit m_objedit->setDefaultType((Level::ObjectType)t);
}

void MainWindow::changeTexture(QString path)
{
	m_texture->load(path);
	m_texture_file = path;

	m_glwidget->loadTexture(m_texture);
	m_texedit->setTexture(m_texture);
	m_tiledesigner->setTexture(m_texture);
	m_tileset_window->setTexture(m_texture);
	m_preview->setTexture(m_texture);
}


void MainWindow::setBGColor()
{
	QColor result = QColorDialog::getColor(m_bgcolor, this, tr("Select background color"));
	if (result.isValid())
	{
		m_bgcolor = result;
		emit m_glwidget->setBGColor(m_bgcolor);
	}
}

void MainWindow::setColor()
{
	QColor result = QColorDialog::getColor(m_object_color, this, tr("Select object color"));
	if (result.isValid())
	{
		m_colorButton->setStyleSheet(tr("background-color: #%1%2%3").arg(result.red(), 2, 16, QChar('0')).arg(result.green(), 2, 16, QChar('0')).arg(result.blue(), 2, 16, QChar('0')));

		m_object_color = result;

		emit m_glwidget->setObjectColor(m_object_color);
	}
}

void MainWindow::zbaseChanged(int value)
{
	emit m_glwidget->setTileBaseZ(value);
}


void MainWindow::edgify_fill_point(FILE* fout, coord_point* points, int p1, int p2)
{
	if (points[p1].con1 >= 0)
	{
		if (points[p1].con2 >= 0)
		{
			fprintf(fout, "Point %d has too many connections!", p1);
		}
		else
		{
			points[p1].con2 = p2;
		}
	}
	else
	{
		points[p1].con1 = p2;
	}

	if (points[p2].con1 >= 0)
	{
		if (points[p2].con2 >= 0)
		{
			fprintf(fout, "Point %d has too many connections!", p2);
		}
		else
		{
			points[p2].con2 = p1;
		}
	}
	else
	{
		points[p2].con1 = p1;
	}
}

bool MainWindow::edgify(std::vector<std::vector<int>>& ptlist)
{
	struct coord_pair
	{
		int x;
		int y;
	};

	QProgressDialog progress("Edgifying...", "Cancel", 0, 5, this);
	progress.setWindowModality(Qt::WindowModal);
	progress.setValue(0);
	progress.setMinimumDuration(0);

	FILE *fout = fopen("edgifylog.txt", "wt");

	int tm_width = Tilemap::AREA_WIDTH;
	int tm_height = Tilemap::AREA_HEIGHT;

	int xpoints = (tm_width * 2) + 2;
	int ypoints = tm_height + 2;

	coord_point* points = new coord_point[xpoints * ypoints];
	for (int i = 0; i < xpoints*ypoints; i++)
	{
		points[i].con1 = -1;
		points[i].con2 = -1;
	}

	// step 1
	progress.setValue(1);

	int num_buckets = (Tilemap::AREA_WIDTH / Tilemap::BUCKET_WIDTH) * (Tilemap::AREA_HEIGHT / Tilemap::BUCKET_HEIGHT);

	for (int b = 0; b < num_buckets; b++)
	{
		Tilemap::Bucket* bucket = m_level->getTileBucket(b);
		if (bucket != nullptr)
		{

			for (int y = 0; y < Tilemap::BUCKET_HEIGHT; y++)
			{
				for (int x = 0; x < Tilemap::BUCKET_WIDTH; x++)
				{
					int ix = (bucket->x * Tilemap::BUCKET_WIDTH) + x;
					int iy = (bucket->y * Tilemap::BUCKET_HEIGHT) + y;

					int ti = m_level->readTilemapTile(ix, iy);
					const Tilemap::Tile* tile = nullptr;
					if (ti != Tilemap::TILE_EMPTY)
					{
						tile = m_level->getTile(ti);

						bool odd = (iy & 1) != 0;

						/*
						coord_pair topleft;
						coord_pair topright;
						coord_pair left;
						coord_pair right;
						coord_pair botleft;
						coord_pair botright;

						left.x = x - 1;
						left.y = y;
						right.x = x + 1;
						right.y = y;

						if (odd)
						{
							topleft.x = x;
							topleft.y = y - 1;
							topright.x = x + 1;
							topright.y = y - 1;
							botleft.x = x;
							botleft.y = y + 1;
							botright.x = x + 1;
							botright.y = y + 1;
						}
						else
						{
							topleft.x = x - 1;
							topleft.y = y - 1;
							topright.x = x;
							topright.y = y - 1;
							botleft.x = x - 1;
							botleft.y = y + 1;
							botright.x = x;
							botright.y = y + 1;
						}
						*/

						int p[6];

						coord_pair connection[6];

						connection[0].x = ix - 1;
						connection[0].y = iy;
						connection[3].x = ix + 1;
						connection[3].y = iy;

						if (odd)
						{
							connection[1].x = ix;		// top left
							connection[1].y = iy - 1;
							connection[2].x = ix + 1;	// top right
							connection[2].y = iy - 1;
							connection[5].x = ix;		// bottom left
							connection[5].y = iy + 1;
							connection[4].x = ix + 1;	// bottom right
							connection[4].y = iy + 1;

							p[0] = (iy * tm_width) + (ix * 2) + 1;
							p[1] = (iy * tm_width) + (ix * 2) + 2;
							p[2] = (iy * tm_width) + (ix * 2) + 3;
							p[3] = ((iy + 1) * tm_width) + (ix * 2) + 3;
							p[4] = ((iy + 1) * tm_width) + (ix * 2) + 2;
							p[5] = ((iy + 1) * tm_width) + (ix * 2) + 1;
						}
						else
						{
							connection[1].x = ix - 1;	// top left
							connection[1].y = iy - 1;
							connection[2].x = ix;		// top right
							connection[2].y = iy - 1;
							connection[5].x = ix - 1;	// bottom left
							connection[5].y = iy + 1;
							connection[4].x = ix;		// bottom right
							connection[4].y = iy + 1;

							p[0] = (iy * tm_width) + (ix * 2) + 0;
							p[1] = (iy * tm_width) + (ix * 2) + 1;
							p[2] = (iy * tm_width) + (ix * 2) + 2;
							p[3] = ((iy + 1) * tm_width) + (ix * 2) + 2;
							p[4] = ((iy + 1) * tm_width) + (ix * 2) + 1;
							p[5] = ((iy + 1) * tm_width) + (ix * 2) + 0;
						}

						int tilecon[6] = { 0, 0, 0, 0, 0, 0 };

						for (int i = 0; i < 6; i++)
						{
							int tx = connection[i].x;
							int ty = connection[i].y;

							tilecon[i] = 0;

							if (tx >= 0 && ty >= 0 && tx < Tilemap::AREA_WIDTH && ty < Tilemap::AREA_HEIGHT)
							{
								int ctile = m_level->readTilemapTile(tx, ty);
								if (ctile != Tilemap::TILE_EMPTY)
								{
									const Tilemap::Tile* tt = m_level->getTile(ctile);

									switch (i)
									{
										case 0:		// LEFT
										{
											if ((tt->side_bits & Tilemap::SIDE_RIGHT) && (tile->side_bits & Tilemap::SIDE_LEFT))
												tilecon[i] = 1;
											break;
										}
										case 1:		// TOPLEFT
										{
											if ((tt->side_bits & Tilemap::SIDE_BOT_RIGHT) && (tile->side_bits & Tilemap::SIDE_TOP_LEFT))
												tilecon[i] = 1;
											break;
										}
										case 2:		// TOPRIGHT
										{
											if ((tt->side_bits & Tilemap::SIDE_BOT_LEFT) && (tile->side_bits & Tilemap::SIDE_TOP_RIGHT))
												tilecon[i] = 1;
											break;
										}
										case 3:		// RIGHT
										{
											if ((tt->side_bits & Tilemap::SIDE_LEFT) && (tile->side_bits & Tilemap::SIDE_RIGHT))
												tilecon[i] = 1;
											break;
										}
										case 4:		// BOTTOM RIGHT
										{
											if ((tt->side_bits & Tilemap::SIDE_TOP_LEFT) && (tile->side_bits & Tilemap::SIDE_BOT_RIGHT))
												tilecon[i] = 1;
											break;
										}
										case 5:		// BOTTOM LEFT
										{
											if ((tt->side_bits & Tilemap::SIDE_TOP_RIGHT) && (tile->side_bits & Tilemap::SIDE_BOT_LEFT))
												tilecon[i] = 1;
											break;
										}
									}
								}
							}
						}

						fprintf(fout, "Tile %d, %d: L %d, TL %d, TR %d, R: %d, BR: %d, BL: %d\n", x, y, tilecon[0], tilecon[1], tilecon[2], tilecon[3], tilecon[4], tilecon[5]);

						/*		p1
							p0		p2
							p5		p3
								p4
						*/

						switch (tile->type)
						{
							case Tilemap::TILE_FULL:
							{
								// LEFT
								if (tilecon[0] == 0)
								{
									edgify_fill_point(fout, points, p[0], p[5]);
								}
								// TOPLEFT
								if (tilecon[1] == 0)
								{
									edgify_fill_point(fout, points, p[0], p[1]);
								}
								// TOPRIGHT
								if (tilecon[2] == 0)
								{
									edgify_fill_point(fout, points, p[1], p[2]);
								}
								// RIGHT
								if (tilecon[3] == 0)
								{
									edgify_fill_point(fout, points, p[2], p[3]);
								}
								// BOTTOM RIGHT
								if (tilecon[4] == 0)
								{
									edgify_fill_point(fout, points, p[3], p[4]);
								}
								// BOTTOM LEFT
								if (tilecon[5] == 0)
								{
									edgify_fill_point(fout, points, p[4], p[5]);
								}
								break;
							}
							case Tilemap::TILE_LEFT:
							{
								// LEFT
								if (tilecon[0] == 0)
								{
									edgify_fill_point(fout, points, p[0], p[5]);
								}
								// TOPLEFT
								if (tilecon[1] == 0)
								{
									edgify_fill_point(fout, points, p[0], p[1]);
								}
								// BOTTOM LEFT
								if (tilecon[5] == 0)
								{
									edgify_fill_point(fout, points, p[4], p[5]);
								}
								// always fill mid
								edgify_fill_point(fout, points, p[4], p[1]);
								break;
							}
							case Tilemap::TILE_RIGHT:
							{
								// TOPRIGHT
								if (tilecon[2] == 0)
								{
									edgify_fill_point(fout, points, p[1], p[2]);
								}
								// RIGHT
								if (tilecon[3] == 0)
								{
									edgify_fill_point(fout, points, p[2], p[3]);
								}
								// BOTTOM RIGHT
								if (tilecon[4] == 0)
								{
									edgify_fill_point(fout, points, p[3], p[4]);
								}
								// always fill mid
								edgify_fill_point(fout, points, p[4], p[1]);
								break;
							}
							case Tilemap::TILE_TOP:
							{
								// TOPLEFT
								if (tilecon[1] == 0)
								{
									edgify_fill_point(fout, points, p[0], p[1]);
								}
								// TOPRIGHT
								if (tilecon[2] == 0)
								{
									edgify_fill_point(fout, points, p[1], p[2]);
								}
								// always fill mid
								edgify_fill_point(fout, points, p[0], p[2]);
								break;
							}
							case Tilemap::TILE_BOTTOM:
							{
								// BOTTOM RIGHT
								if (tilecon[4] == 0)
								{
									edgify_fill_point(fout, points, p[3], p[4]);
								}
								// BOTTOM LEFT
								if (tilecon[5] == 0)
								{
									edgify_fill_point(fout, points, p[4], p[5]);
								}
								// always fill mid
								edgify_fill_point(fout, points, p[5], p[3]);
								break;
							}
							case Tilemap::TILE_MID:
							{
								// LEFT
								if (tilecon[0] == 0)
								{
									edgify_fill_point(fout, points, p[0], p[5]);
								}
								// RIGHT
								if (tilecon[3] == 0)
								{
									edgify_fill_point(fout, points, p[2], p[3]);
								}
								// always fill mid
								edgify_fill_point(fout, points, p[0], p[2]);
								edgify_fill_point(fout, points, p[5], p[3]);
								break;
							}
							case Tilemap::TILE_CORNER_TL:
							{
								// LEFT
								if (tilecon[0] == 0)
								{
									edgify_fill_point(fout, points, p[0], p[5]);
								}
								// TOPLEFT
								if (tilecon[1] == 0)
								{
									edgify_fill_point(fout, points, p[0], p[1]);
								}
								// always fill mid
								edgify_fill_point(fout, points, p[5], p[1]);
								break;
							}
							case Tilemap::TILE_CORNER_TR:
							{
								// RIGHT
								if (tilecon[3] == 0)
								{
									edgify_fill_point(fout, points, p[2], p[3]);
								}
								// TOPRIGHT
								if (tilecon[2] == 0)
								{
									edgify_fill_point(fout, points, p[1], p[2]);
								}
								// always fill mid
								edgify_fill_point(fout, points, p[1], p[3]);
								break;
							}
							case Tilemap::TILE_CORNER_BL:
							{
								// LEFT
								if (tilecon[0] == 0)
								{
									edgify_fill_point(fout, points, p[0], p[5]);
								}
								// BOTTOM LEFT
								if (tilecon[5] == 0)
								{
									edgify_fill_point(fout, points, p[4], p[5]);
								}
								// always fill mid
								edgify_fill_point(fout, points, p[0], p[4]);
								break;
							}
							case Tilemap::TILE_CORNER_BR:
							{
								// RIGHT
								if (tilecon[3] == 0)
								{
									edgify_fill_point(fout, points, p[2], p[3]);
								}
								// BOTTOM RIGHT
								if (tilecon[4] == 0)
								{
									edgify_fill_point(fout, points, p[3], p[4]);
								}
								// always fill mid
								edgify_fill_point(fout, points, p[4], p[2]);
								break;
							}
						}

						//fprintf(fout, "Tile %d, %d: tl %d, %d, tr %d, %d, bl %d, %d, br %d, %d\n", x, y, topleft.x, topleft.y, topright.x, topright.y, botleft.x, botleft.y, botright.x, botright.y);
					}

				}
			}
		}
	}

	// step 2
	progress.setValue(2);

	/*
	for (int j = 0; j < tm_height; j++)
	{
		for (int i = 0; i < tm_width; i++)
		{
			int index = (j * tm_width) + i;
			if (points[index].con1 >= 0 || points[index].con2 >= 0)
			{
				fprintf(fout, "Point %d,%d [%d] connects to %d, %d\n", i, j, index, points[index].con1, points[index].con2);
			}
		}
	}
	*/

	// step 3
	progress.setValue(3);
	

	ptlist.clear();

	bool start_found;
	do
	{
		start_found = false;
		int start_point = -1;

		// find a start point
		for (int i = 0; i < tm_width*tm_height; i++)
		{
			if (points[i].con1 >= 0 && points[i].con2 >= 0)
			{
				start_found = true;
				start_point = i;
				fprintf(fout, "Found a start point at %d (%d, %d)\n", i, i % tm_width, i / tm_width);
				break;
			}
		}
		/*
		for (int b = 0; b < num_buckets; b++)
		{
			Tilemap::Bucket* bucket = m_level->getTileBucket(b);
			if (bucket != nullptr)
			{
				int ix = (bucket->x * Tilemap::BUCKET_WIDTH);
				int iy = (bucket->y * Tilemap::BUCKET_HEIGHT);

				for (int y = iy; y < iy + Tilemap::BUCKET_HEIGHT; y++)
				{
					for (int x = ix; x < ix + Tilemap::BUCKET_WIDTH; x++)
					{
						int pi = y * Tilemap::AREA_WIDTH + x;
						if (points[pi].con1 >= 0 && points[pi].con2 >= 0)
						{
							start_found = true;
							start_point = pi;
							fprintf(fout, "Found a start point at %d (%d, %d)\n", pi, pi % tm_width, pi / tm_width);
							goto end_find;
						}
					}
				}
			}
		}

		end_find:
		*/



		if (start_found)
		{
			std::vector<int> newlist;

			int current_point = start_point;
			int prev_point = start_point;

			newlist.push_back(start_point);
			current_point = points[start_point].con1;

			points[start_point].con1 = -1;
			points[start_point].con2 = -1;

			bool end = false;
			bool closed = false;
			while (!end)
			{
				if (current_point == start_point)
				{
					closed = true;
					end = true;
				}
				else
				{
					newlist.push_back(current_point);

					if (points[current_point].con1 >= 0 && points[current_point].con1 != prev_point)
					{
						prev_point = current_point;
						current_point = points[current_point].con1;
						points[prev_point].con1 = -1;
						points[prev_point].con2 = -1;
					}
					else if (points[current_point].con2 >= 0 && points[current_point].con2 != prev_point)
					{
						prev_point = current_point;
						current_point = points[current_point].con2;
						points[prev_point].con1 = -1;
						points[prev_point].con2 = -1;
					}
					else
					{
						end = true;
					}
				}
			}



			if (!closed)
			{
				fprintf(fout, "Loop not closed!\n");
				delete[] points;
				fclose(fout);
				return false;
			}

			ptlist.push_back(newlist);
		}
	} while (start_found);

	// step 4
	progress.setValue(4);

	for (int loop = 0; loop < ptlist.size(); loop++)
	{
		fprintf(fout, "Loop %d: Points in loop: ", loop);
		for (int i = 0; i < ptlist[loop].size(); i++)
		{
			fprintf(fout, "%d ", ptlist[loop].at(i));
		}
		fprintf(fout, "\n");
	}

	// step 5
	progress.setValue(5);



	fclose(fout);

	delete[] points;

	return true;
}



void MainWindow::doEdgify()
{
	std::vector<std::vector<int>> ptlist;

	if (edgify(ptlist))
	{
		m_glwidget->setEdgeData(ptlist);
	}
}

void MainWindow::clearEdgify()
{
	std::vector<std::vector<int>> ptlist;

	m_glwidget->setEdgeData(ptlist);
}


void MainWindow::createActions()
{
	// file menu
	m_newAction = new QAction(QIcon(":/images/new.png"), tr("&New"), this);
	m_newAction->setShortcuts(QKeySequence::New);
	m_newAction->setStatusTip(tr("Create a new file"));
	connect(m_newAction, SIGNAL(triggered()), this, SLOT(newFile()));

	m_openAction = new QAction(QIcon(":/images/open.png"), tr("&Open"), this);
	m_openAction->setShortcuts(QKeySequence::Open);
	m_openAction->setStatusTip(tr("Open an existing file"));
	connect(m_openAction, SIGNAL(triggered()), this, SLOT(openFile()));

	m_saveAction = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
	m_saveAction->setShortcuts(QKeySequence::Save);
	m_saveAction->setStatusTip(tr("Save file"));
	connect(m_saveAction, SIGNAL(triggered()), this, SLOT(saveFile()));

	m_saveAsAction = new QAction(QIcon(":/images/saveas.png"), tr("Save &as"), this);
	m_saveAsAction->setShortcuts(QKeySequence::SaveAs);
	m_saveAsAction->setStatusTip(tr("Save as new file"));
	connect(m_saveAsAction, SIGNAL(triggered()), this, SLOT(saveAsFile()));

	m_exportAction = new QAction(tr("Export Level"), this);
	m_exportAction->setStatusTip(tr("Export Level"));
	connect(m_exportAction, SIGNAL(triggered()), this, SLOT(exportLevel()));

	m_exitAction = new QAction(QIcon(":/images/exit.png"), tr("&Exit"), this);
	m_exitAction->setShortcuts(QKeySequence::Quit);
	m_exitAction->setStatusTip(tr("Exit the application"));
	connect(m_exitAction, SIGNAL(triggered()), this, SLOT(close()));


	// edit menu
	m_copyAction = new QAction(tr("Copy"), this);
	m_copyAction->setShortcut(Qt::Key_C + Qt::CTRL);
	connect(m_copyAction, SIGNAL(triggered()), m_glwidget, SLOT(copy()));

	m_pasteAction = new QAction(tr("Paste"), this);
	m_pasteAction->setShortcut(Qt::Key_V + Qt::CTRL);
	connect(m_pasteAction, SIGNAL(triggered()), m_glwidget, SLOT(paste()));

	m_toggleGridAction = new QAction(QIcon("grid.png"), tr("Enable grid"), this);
	m_toggleGridAction->setCheckable(true);
	m_toggleGridAction->setShortcut(Qt::Key_G);
	connect(m_toggleGridAction, SIGNAL(triggered()), this, SLOT(toggleGrid()));

	m_snapGridAction = new QAction(QIcon("snapgrid.png"), tr("Snap to grid"), this);
	m_snapGridAction->setCheckable(true);
	m_snapGridAction->setShortcut(Qt::Key_H);
	connect(m_snapGridAction, SIGNAL(triggered()), this, SLOT(snapGrid()));

	m_bgColorAction = new QAction(tr("Select background color..."), this);
	connect(m_bgColorAction, SIGNAL(triggered()), this, SLOT(setBGColor()));

	m_levelConfAction = new QAction(tr("Level settings..."), this);
	connect(m_levelConfAction, SIGNAL(triggered()), this, SLOT(levelConfig()));

	m_zbaseLabel = new QLabel(tr("Z Base:"));
	m_zbaseSpin = new QSpinBox();
	m_zbaseSpin->setRange(0, Tilemap::Z_MAX);
	m_zbaseSpin->setSingleStep(1);
	QBoxLayout* zbase_layout = new QBoxLayout(QBoxLayout::LeftToRight);
	zbase_layout->setSpacing(2);
	zbase_layout->setMargin(1);
	zbase_layout->addWidget(m_zbaseLabel);
	zbase_layout->addWidget(m_zbaseSpin);
	connect(m_zbaseSpin, SIGNAL(valueChanged(int)), this, SLOT(zbaseChanged(int)));

	m_zbaseWidget = new QWidget;
	m_zbaseWidget->setLayout(zbase_layout);

	m_gridSizeCombo = new QComboBox();
	for (int i=0; i < GLWidget::NUM_GRID_SIZES; i++)
	{
		m_gridSizeCombo->addItem(tr("%1").arg(GLWidget::GRID_SIZE[i], 0, 'f', 2));
	}
	connect(m_gridSizeCombo, SIGNAL(activated(int)), this, SLOT(setGridSize(int)));
	m_gridSizeLabel = new QLabel("Grid:");
	m_gridSizeLabel->setMinimumWidth(30);
	QBoxLayout* gridsize_layout = new QBoxLayout(QBoxLayout::LeftToRight);
	gridsize_layout->setSpacing(2);
	gridsize_layout->setMargin(1);
	gridsize_layout->addWidget(m_gridSizeLabel);
	gridsize_layout->addWidget(m_gridSizeCombo);
	m_gridSizeWidget = new QWidget;
	m_gridSizeWidget->setMaximumHeight(30);
	m_gridSizeWidget->setLayout(gridsize_layout);


	// texture menu
	m_loadTexAction = new QAction(tr("Load Texture..."), this);
	m_loadTexAction->setStatusTip(tr("Load new texture"));
	connect(m_loadTexAction, SIGNAL(triggered()), this, SLOT(loadTexture()));


	// prefab menu
	m_loadPrefabsAction = new QAction(tr("Load Prefabs..."), this);
	m_loadPrefabsAction->setStatusTip(tr("Load prefabs"));
	connect(m_loadPrefabsAction, SIGNAL(triggered()), this, SLOT(loadPrefabs()));

	m_savePrefabsAction = new QAction(tr("Save Prefabs..."), this);
	m_savePrefabsAction->setStatusTip(tr("Save prefabs"));
	connect(m_savePrefabsAction, SIGNAL(triggered()), this, SLOT(savePrefabs()));



	// operation tools
	m_opgroup = new QActionGroup(this);

	m_select_action = new QAction(QIcon("select.png"), tr("Select and edit object"), this);
	m_select_action->setCheckable(true);
	m_select_action->setShortcut(Qt::Key_S);
	connect(m_select_action, SIGNAL(triggered()), this, SLOT(selectionMode()));

	m_move_action = new QAction(QIcon("move.png"), tr("Move object"), this);
	m_move_action->setCheckable(true);
	m_move_action->setShortcut(Qt::Key_M);
	connect(m_move_action, SIGNAL(triggered()), this, SLOT(moveMode()));

	m_rotate_action = new QAction(QIcon("rotate.png"), tr("Rotate object"), this);
	m_rotate_action->setCheckable(true);
	connect(m_rotate_action, SIGNAL(triggered()), this, SLOT(rotateMode()));

	m_scale_action = new QAction(QIcon("scale.png"), tr("Scale object"), this);
	m_scale_action->setCheckable(true);
	connect(m_scale_action, SIGNAL(triggered()), this, SLOT(scaleMode()));

	m_draw_poly_action = new QAction(QIcon("polygon.png"), tr("Draw new polygon object"), this);
	m_draw_poly_action->setCheckable(true);
	m_draw_poly_action->setShortcut(Qt::Key_T);
	connect(m_draw_poly_action, SIGNAL(triggered()), this, SLOT(drawPolyMode()));

	m_draw_rect_action = new QAction(QIcon("rect.png"), tr("Draw new rectangle object"), this);
	m_draw_rect_action->setCheckable(true);
	m_draw_rect_action->setShortcut(Qt::Key_R);
	connect(m_draw_rect_action, SIGNAL(triggered()), this, SLOT(drawRectMode()));

	m_tilemap_action = new QAction(QIcon("tilemap.png"), tr("Draw on tilemap"), this);
	m_tilemap_action->setCheckable(true);
	connect(m_tilemap_action, SIGNAL(triggered()), this, SLOT(tilemapMode()));

	m_tile_zedit_action = new QAction(QIcon("tilezedit.png"), tr("Edit Z on tilemap"), this);
	m_tile_zedit_action->setCheckable(true);
	connect(m_tile_zedit_action, SIGNAL(triggered()), this, SLOT(tileZEditMode()));

	m_opgroup->addAction(m_select_action);
	m_opgroup->addAction(m_move_action);
	m_opgroup->addAction(m_rotate_action);
	m_opgroup->addAction(m_scale_action);
	m_opgroup->addAction(m_draw_poly_action);
	m_opgroup->addAction(m_draw_rect_action);
	m_opgroup->addAction(m_tilemap_action);
	m_opgroup->addAction(m_tile_zedit_action);


	// editor tools
	m_toggle_texedit = new QAction(QIcon("texture.png"), tr("Toggle Texture Editor"), this);
	m_toggle_texedit->setCheckable(true);
	connect(m_toggle_texedit, SIGNAL(triggered()), this, SLOT(toggleTexEdit()));

	m_toggle_objedit = new QAction(QIcon("objedit.png"), tr("Toggle Object Editor"), this);
	m_toggle_objedit->setCheckable(true);
	connect(m_toggle_objedit, SIGNAL(triggered()), this, SLOT(toggleObjEdit()));

	m_toggle_tiledesigner = new QAction(QIcon("objfilter.png"), tr("Toggle Tile Designer"), this);
	m_toggle_tiledesigner->setCheckable(true);
	connect(m_toggle_tiledesigner, SIGNAL(triggered()), this, SLOT(toggleTileDesigner()));

	m_toggle_tileset_window = new QAction(QIcon("tilemap.png"), tr("Toggle Tileset"), this);
	m_toggle_tileset_window->setCheckable(true);
	connect(m_toggle_tileset_window, SIGNAL(triggered()), this, SLOT(toggleTilesetWindow()));

	m_toggle_preview = new QAction(QIcon("3d.png"), tr("Toggle Preview"), this);
	m_toggle_preview->setCheckable(true);
	connect(m_toggle_preview, SIGNAL(triggered()), this, SLOT(togglePreview()));


	// visbox
	m_toggle_visbox = new QAction(QIcon("visbox.png"), tr("Toggle Visualization Box"), this);
	m_toggle_visbox->setCheckable(true);
	m_toggle_visbox->setShortcut(Qt::Key_Period);
	connect(m_toggle_visbox, SIGNAL(triggered()), this, SLOT(toggleVisbox()));

	m_visbox_conf_action = new QAction(QIcon("visboxconf.png"), tr("Visualization Box Configuration"), this);
	connect(m_visbox_conf_action, SIGNAL(triggered()), this, SLOT(visboxConfig()));


	// edgify
	m_edgify_action = new QAction(tr("Edgify..."), this);
	connect(m_edgify_action, SIGNAL(triggered()), this, SLOT(doEdgify()));

	// clear edgify
	m_clear_edgify_action = new QAction(tr("Clear Edgify"), this);
	connect(m_clear_edgify_action, SIGNAL(triggered()), this, SLOT(clearEdgify()));


	// zoom
	m_zoomLevelCombo = new QComboBox();
	for (int i=0; i < GLWidget::NUM_ZOOM_LEVELS; i++)
	{
		m_zoomLevelCombo->addItem(tr("%1%").arg((int)(GLWidget::ZOOM_LEVELS[i] * 100.0f)));
	}
	connect(m_zoomLevelCombo, SIGNAL(activated(int)), this, SLOT(setZoomLevel(int)));
	m_zoomLevelLabel = new QLabel("Zoom:");
	m_zoomLevelLabel->setMinimumWidth(30);
	QBoxLayout* zoomlevel_layout = new QBoxLayout(QBoxLayout::LeftToRight);
	zoomlevel_layout->setSpacing(2);
	zoomlevel_layout->setMargin(1);
	zoomlevel_layout->addWidget(m_zoomLevelLabel);
	zoomlevel_layout->addWidget(m_zoomLevelCombo);
	m_zoomLevelWidget = new QWidget;
	m_zoomLevelWidget->setMaximumHeight(30);
	m_zoomLevelWidget->setLayout(zoomlevel_layout);


	// default type
	m_defTypeCombo = new QComboBox();
	m_defTypeCombo->addItem("Trigger", QVariant(Level::OBJECT_TYPE_TRIGGER));
	m_defTypeCombo->addItem("Destructible", QVariant(Level::OBJECT_TYPE_DESTRUCTIBLE));
	m_defTypeCombo->addItem("Mover", QVariant(Level::OBJECT_TYPE_MOVER));
	m_defTypeCombo->addItem("Enemy", QVariant(Level::OBJECT_TYPE_ENEMY));
	connect(m_defTypeCombo, SIGNAL(activated(int)), this, SLOT(setDefType(int)));
	m_defTypeLabel = new QLabel("Default Type:");
	m_defTypeLabel->setMinimumWidth(50);
	QBoxLayout* deftype_layout = new QBoxLayout(QBoxLayout::LeftToRight);
	deftype_layout->setSpacing(2);
	deftype_layout->setMargin(1);
	deftype_layout->addWidget(m_defTypeLabel);
	deftype_layout->addWidget(m_defTypeCombo);
	m_defTypeWidget = new QWidget;
	m_defTypeWidget->setMaximumHeight(30);
	m_defTypeWidget->setLayout(deftype_layout);


	// color
	m_colorButton = new QPushButton("", this);
	m_colorButton->setFocusPolicy(Qt::NoFocus);
	m_colorButton->setStyleSheet(tr("background-color: #%1%2%3").arg(m_object_color.red(), 2, 16, QChar('0')).arg(m_object_color.green(), 2, 16, QChar('0')).arg(m_object_color.blue(), 2, 16, QChar('0')));
	m_colorButton->setMaximumHeight(25);
	m_colorButton->setMaximumWidth(25);
	connect(m_colorButton, SIGNAL(clicked()), this, SLOT(setColor()));
	m_colorLabel = new QLabel("Color:");
	QBoxLayout* color_layout = new QBoxLayout(QBoxLayout::LeftToRight);
	color_layout->setSpacing(2);
	color_layout->setMargin(1);
	color_layout->addWidget(m_colorLabel);
	color_layout->addWidget(m_colorButton);

	m_colorWidget = new QWidget;
	m_colorWidget->setMaximumHeight(30);
	m_colorWidget->setLayout(color_layout);
}

void MainWindow::createMenus()
{
	// file menu
	m_fileMenu = menuBar()->addMenu(tr("&File"));
	m_fileMenu->addAction(m_newAction);
	m_fileMenu->addAction(m_openAction);
	m_fileMenu->addAction(m_saveAction);
	m_fileMenu->addAction(m_saveAsAction);
	m_fileMenu->addSeparator();
	m_fileMenu->addAction(m_exportAction);
	m_fileMenu->addSeparator();
	m_fileMenu->addAction(m_exitAction);

	// edit menu
	m_editMenu = menuBar()->addMenu(tr("&Edit"));
	m_editMenu->addAction(m_copyAction);
	m_editMenu->addAction(m_pasteAction);
	m_editMenu->addSeparator();
	m_editMenu->addAction(m_toggleGridAction);
	m_editMenu->addAction(m_snapGridAction);
	m_editMenu->addSeparator();
	m_editMenu->addAction(m_bgColorAction);
	m_editMenu->addAction(m_levelConfAction);
	m_editMenu->addSeparator();
	m_editMenu->addAction(m_edgify_action);
	m_editMenu->addAction(m_clear_edgify_action);

	// texture menu
	m_textureMenu = menuBar()->addMenu(tr("&Texture"));
	m_textureMenu->addAction(m_loadTexAction);

	// prefab menu
	m_prefabMenu = menuBar()->addMenu(tr("Prefab"));
	m_prefabMenu->addAction(m_loadPrefabsAction);
	m_prefabMenu->addAction(m_savePrefabsAction);
}

void MainWindow::createToolbars()
{
	m_file_toolbar = addToolBar("File");
	m_file_toolbar->addAction(m_newAction);
	m_file_toolbar->addAction(m_openAction);
	m_file_toolbar->addAction(m_saveAction);
	m_file_toolbar->addAction(m_saveAsAction);

	m_op_toolbar = addToolBar("Operation");
	m_op_toolbar->addAction(m_select_action);
	m_op_toolbar->addAction(m_move_action);
	m_op_toolbar->addAction(m_rotate_action);
	m_op_toolbar->addAction(m_scale_action);
	m_op_toolbar->addAction(m_draw_poly_action);
	m_op_toolbar->addAction(m_draw_rect_action);
	m_op_toolbar->addAction(m_tilemap_action);
	m_op_toolbar->addAction(m_tile_zedit_action);

	m_zbase_toolbar = addToolBar("Z Base");
	m_zbase_toolbar->addWidget(m_zbaseWidget);

	m_grid_toolbar = addToolBar("Grid");
	m_grid_toolbar->addAction(m_toggleGridAction);
	m_grid_toolbar->addAction(m_snapGridAction);
	m_grid_toolbar->addWidget(m_gridSizeWidget);

	m_editor_toolbar = addToolBar("Editors");
	m_editor_toolbar->addAction(m_toggle_texedit);
	m_editor_toolbar->addAction(m_toggle_objedit);
	m_editor_toolbar->addAction(m_toggle_tiledesigner);
	m_editor_toolbar->addAction(m_toggle_tileset_window);
	m_editor_toolbar->addAction(m_toggle_preview);

	m_visbox_toolbar = addToolBar("Visualization Box");
	m_visbox_toolbar->addAction(m_toggle_visbox);
	m_visbox_toolbar->addAction(m_visbox_conf_action);

	m_zoom_toolbar = addToolBar("Zoom");
	m_zoom_toolbar->addWidget(m_zoomLevelWidget);

	m_deftype_toolbar = addToolBar("Default Type");
	m_deftype_toolbar->addWidget(m_defTypeWidget);
	m_deftype_toolbar->addWidget(m_colorWidget);
}

void MainWindow::createStatusbar()
{

}

void MainWindow::setCurrentFile(QString& filename)
{
	QString filetitle;

	if (filename.length() <= 0)
	{
		filetitle = tr("Untitled");
	}
	else
	{
		filetitle = filename;
	}

	m_open_file = filename;

	setWindowFilePath(tr("%1 - WorldEditH").arg(filetitle));
}

void MainWindow::reset()
{
	m_level->reset();
	m_level->resetModify();

	emit m_objbrowser->reset();
	emit m_glwidget->deselect();
	emit m_objbrowser->deselect();

	emit m_glwidget->reset();

	emit m_tileset_window->reset();
}

void MainWindow::newDocument()
{
	reset();
	selectionMode();

	setCurrentFile(tr(""));

	// load default texture
	changeTexture("blue_checker.png");

	for (int i = 0; i < 16; i++)
	{
		QColorDialog::setCustomColor(i, QColor(255, 255, 255));
	}
}

void MainWindow::resetControls()
{
}

void MainWindow::setGridSize(int size)
{
	emit m_glwidget->setGridSize(size);
}

void MainWindow::setZoomLevel(int zoom)
{
	emit m_glwidget->setZoomLevel(zoom);
}

void MainWindow::zoomIn()
{
	int current = m_zoomLevelCombo->currentIndex();
	current++;
	if (current >= m_zoomLevelCombo->count())
		current = m_zoomLevelCombo->count() - 1;

	m_zoomLevelCombo->setCurrentIndex(current);
	setZoomLevel(current);
}

void MainWindow::zoomOut()
{
	int current = m_zoomLevelCombo->currentIndex();
	current--;
	if (current < 0)
		current = 0;

	m_zoomLevelCombo->setCurrentIndex(current);
	setZoomLevel(current);
}

void MainWindow::loadTexture()
{
	QString filename = QFileDialog::getOpenFileName(this,
													tr("Load Texture"),
													getTextureDir(),
													tr("Texture File (*.png);;All Files (*.*)"));

	if (!filename.isEmpty())
	{
		changeTexture(filename);

		// set current directory
		setTextureDir(filename);
	}
}


void MainWindow::readConfigFile(QString& filename)
{
	QFile* file = new QFile(filename);
	file->open(QIODevice::ReadOnly);

	QXmlStreamReader input(file);

	vector<std::string> trigger_list;
	std::string current_name;

	while (!input.atEnd())
	{
		input.readNext();

		if (input.isStartElement())
		{
			QString element = input.name().toString();
			QXmlStreamAttributes attrs = input.attributes();
			int num_attrs = attrs.size();

			if (element == "triggers")
			{
				trigger_list.clear();
			}
			else if (element == "trigger")
			{
				for (int i=0; i < num_attrs; i++)
				{
					QString attr_name = attrs[i].name().toString();

					if (attr_name == "name")
					{
						current_name = attrs[i].value().toString().toStdString();
					}
				}
			}
		}
		else if (input.isEndElement())
		{
			QString element = input.name().toString();

			if (element == "trigger")
			{
				trigger_list.push_back(current_name);
			}
		}
	};

	m_level->setTriggerList(trigger_list);
}

bool MainWindow::readBinaryProjectFile(QString& filename)
{
	BinaryFile input;

	const unsigned int hspf_id = 0x48535046;
	const unsigned int hspf_version = 0x10003;
	
	Level::Object::Param params[Level::Object::NUM_PARAMS];
	QString object_name = "";
	QString texture_name = "";
	QString tile_name = "";

	QColor custom_colors[16];

	glm::vec4 points[8];

	reset();

	try
	{
		int inb, inp;
		char buf[200];
		input.open(filename.toStdString(), BinaryFile::MODE_READONLY);

		// ID
		unsigned int id = input.read_dword();
		if (id != hspf_id)
			throw "HSPF ID not found";

		// version
		unsigned int version = input.read_dword();
		if (version != hspf_version)
			throw "Wrong HSPF version";

		// texture name
		inb = 0;
		inp = 0;
		do
		{
			inb = input.read_byte();
			buf[inp++] = inb;
		} while (inb != 0);

		texture_name = QString(buf);


		// custom colors
		for (int i = 0; i < 16; i++)
		{
			unsigned int cc = input.read_dword();
			custom_colors[i] = QColor((cc >> 16) & 0xff, (cc >> 8) & 0xff, cc & 0xff);
		}

		// ---------------------

		// num objects
		int num_objects = input.read_dword();

		// objects
		for (int i=0; i < num_objects; i++)
		{
			// object name
			inb = 0;
			inp = 0;
			do
			{
				inb = input.read_byte();
				buf[inp++] = inb;
			} while (inb != 0);

			object_name = QString(buf);

			int objtype = input.read_dword();
			int objz = input.read_dword();

			unsigned int object_color = input.read_dword();

			glm::vec2 pps[8];
			glm::vec2 uvs[8];

			int num_points = input.read_dword();
			for (int j=0; j < num_points; j++)
			{
				float x = input.read_float();
				float y = input.read_float();
				float u = input.read_float();
				float v = input.read_float();
				
				pps[j] = glm::vec2(x, y);
				uvs[j] = glm::vec2(u, v);
			}
			for (int j=0; j < Level::Object::NUM_PARAMS; j++)
			{
				params[j].i = input.read_dword();
			}

			int id = m_level->insertObject(pps, uvs, num_points, (Level::ObjectType)objtype, object_name.toStdString(), object_color);
			emit m_objbrowser->add(id);

			// set params
			Level::Object* obj = m_level->getObjectById(id);
			for (int j=0; j < Level::Object::NUM_PARAMS; j++)
			{
				obj->setParam(j, params[j]);
			}

			obj->setZ(objz);	
		}

		// ------------------

		// num of tiles
		int num_tiles = input.read_dword();

		PolygonDef* top = new PolygonDef(6);
		PolygonDef* side = new PolygonDef(4);

		// tiles
		for (int i=0; i < num_tiles; i++)
		{
			// tile name
			inb = 0;
			inp = 0;
			do
			{
				inb = input.read_byte();
				buf[inp++] = inb;
			} while (inb != 0);

			tile_name = QString(buf);

			//  tile type
			unsigned int type = input.read_dword();

			int num_top_points = 0;
			glm::vec2 top_pps[6];
			glm::vec2 side_pps[4];

			top->reset();
			side->reset();

			// top UVs
			switch (type)
			{
				case Tilemap::TILE_FULL:		num_top_points = 6; break;
				case Tilemap::TILE_LEFT:		num_top_points = 4; break;
				case Tilemap::TILE_RIGHT:		num_top_points = 4; break;
				case Tilemap::TILE_TOP:			num_top_points = 3; break;
				case Tilemap::TILE_BOTTOM:		num_top_points = 3; break;
				case Tilemap::TILE_MID:			num_top_points = 4; break;
				case Tilemap::TILE_CORNER_TL:	num_top_points = 3; break;
				case Tilemap::TILE_CORNER_TR:	num_top_points = 3; break;
				case Tilemap::TILE_CORNER_BL:	num_top_points = 3; break;
				case Tilemap::TILE_CORNER_BR:	num_top_points = 3; break;
			}

			for (int j = 0; j < num_top_points; j++)
			{
				float x = input.read_float();
				float y = input.read_float();

				top->insertPoint(glm::vec2(x, y));
			}

			// side UVs
			for (int j=0; j < 4; j++)
			{
				float x = input.read_float();
				float y = input.read_float();
				
				side->insertPoint(glm::vec2(x, y));
			}

			unsigned int tile_color = input.read_dword();

			int id = m_level->insertTile(tile_name.toStdString(), top, side, tile_color, (Tilemap::TileType)type);
			emit m_tileset_window->add(id);
		}

		delete top;
		delete side;

		// ----------------

		// buckets
		int num_buckets = input.read_dword();

		for (int i = 0; i < num_buckets; i++)
		{
			int bucket_x = input.read_dword() * Tilemap::BUCKET_WIDTH;
			int bucket_y = input.read_dword() * Tilemap::BUCKET_HEIGHT;

			for (int y = 0; y < Tilemap::BUCKET_HEIGHT; y++)
			{
				for (int x = 0; x < Tilemap::BUCKET_WIDTH; x++)
				{
					unsigned int data = input.read_dword();

					int tile = data & Tilemap::TILE_MASK;
					int z = (data & Tilemap::Z_MASK) >> Tilemap::Z_SHIFT;

					m_level->editTilemapTile(bucket_x + x, bucket_y + y, tile);
					m_level->editTilemapZ(bucket_x + x, bucket_y + y, z);
				}
			}
		}


		// ------------------------

		// load texture if needed
		if (!texture_name.isEmpty())
		{
			if (texture_name != m_texture_file)
			{
				QMessageBox box;
				box.setText("Different texture.");
				box.setInformativeText("The prefab file uses a different texture. Want to load this texture?");
				box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
				box.setDefaultButton(QMessageBox::Yes);
				int ret = box.exec();

				if (ret == QMessageBox::Yes)
				{
					changeTexture(texture_name);
				}
			}
		}	
	}
	catch (ios_base::failure&)
	{
		input.close();
		return false;
	}
	catch (int e)
	{
		input.close();
		return false;
	}

	// apply custom colors
	for (int i = 0; i < 16; i++)
	{
		QColorDialog::setCustomColor(i, custom_colors[i]);
	}

	return true;
}


bool MainWindow::writeBinaryProjectFile(QString& filename)
{
	BinaryFile output;

	const char hspf_id[4] = { 0x48, 0x53, 0x50, 0x46 };
	const unsigned int hspf_version = 0x10003;

	int num_objects = m_level->numObjects();

	try
	{
		output.open(filename.toStdString(), BinaryFile::MODE_WRITEONLY);

		// ID
		output.write((char*)hspf_id, 4);

		// version
		output.write_dword(hspf_version);

		// texture name
		QByteArray texname = m_texture_file.toLocal8Bit();
		for (int i=0; i < texname.length(); i++)
		{
			output.write_byte(texname.at(i));
		}
		output.write_byte(0);		// null terminator


		// custom colors
		for (int i = 0; i < 16; i++)
		{
			QColor color = QColorDialog::customColor(i);
			unsigned int cc = (color.red() << 16) | (color.green() << 8) | (color.blue());
			output.write_dword(cc);
		}

		// num objects
		output.write_dword(num_objects);

		// objects
		for (int i=0; i < num_objects; i++)
		{
			Level::Object* object = m_level->getObject(i);

			int num_points = object->getNumPoints();

			// object name
			std::string name = object->getName();
			for (int j=0; j < name.length(); j++)
			{
				output.write_byte(name.at(j));
			}
			output.write_byte(0);	// null terminator

			// type
			output.write_dword(object->getType());

			// z
			output.write_dword(object->getZ());

			// color
			output.write_dword(object->getColor());

			// num points
			output.write_dword(num_points);

			// points
			for (int p=0; p < num_points; p++)
			{
				glm::vec2 point = object->getPoint(p);
				glm::vec2 uv = object->getUV(p);
				
				output.write_float(point.x);
				output.write_float(point.y);
				output.write_float(uv.x);
				output.write_float(uv.y);
			}

			// params
			for (int p=0; p < Level::Object::NUM_PARAMS; p++)
			{
				Level::Object::Param param = object->getParam(p);
				output.write_dword(param.i);
			}
		}
		

		// tiles
		int num_tiles = m_level->getNumTiles();
		output.write_dword(num_tiles);

		for (int i=0; i < num_tiles; i++)
		{
			const Tilemap::Tile* tile  = m_level->getTile(i);

			// tile name
			std::string name = tile->name;
			for (int j=0; j < name.length(); j++)
			{
				output.write_byte(name.at(j));
			}
			output.write_byte(0);	// null terminator

			// tile type
			output.write_dword(tile->type);

			int num_top_points = 0;

			// top UVs
			switch (tile->type)
			{
				case Tilemap::TILE_FULL:		num_top_points = 6; break;				
				case Tilemap::TILE_LEFT:		num_top_points = 4; break;				
				case Tilemap::TILE_RIGHT:		num_top_points = 4; break;
				case Tilemap::TILE_TOP:			num_top_points = 3; break;				
				case Tilemap::TILE_BOTTOM:		num_top_points = 3; break;
				case Tilemap::TILE_MID:			num_top_points = 4; break;
				case Tilemap::TILE_CORNER_TL:	num_top_points = 3; break;
				case Tilemap::TILE_CORNER_TR:	num_top_points = 3; break;
				case Tilemap::TILE_CORNER_BL:	num_top_points = 3; break;
				case Tilemap::TILE_CORNER_BR:	num_top_points = 3; break;
			}

			for (int j = 0; j < num_top_points; j++)
			{
				output.write_float(tile->top_points[j].x);
				output.write_float(tile->top_points[j].y);
			}

			// side UVs
			for (int j = 0; j < 4; j++)
			{
				output.write_float(tile->side_points[j].x);
				output.write_float(tile->side_points[j].y);
			}

			// tile color
			output.write_dword(tile->color);
		}

		// buckets
		std::vector<Tilemap::Bucket*> buckets;

		int total_buckets = (Tilemap::AREA_WIDTH / Tilemap::BUCKET_WIDTH) * (Tilemap::AREA_HEIGHT / Tilemap::BUCKET_HEIGHT);
		for (int i = 0; i < total_buckets; i++)
		{
			Tilemap::Bucket* b = m_level->getTileBucket(i);
			if (b != nullptr)
			{
				buckets.push_back(b);
			}
		}

		// num buckets
		output.write_dword(buckets.size());

		for (int i = 0; i < buckets.size(); i++)
		{
			Tilemap::Bucket* b = buckets.at(i);
			output.write_dword(b->x);
			output.write_dword(b->y);

			for (int t = 0; t < (Tilemap::BUCKET_WIDTH * Tilemap::BUCKET_HEIGHT); t++)
			{
				unsigned int tiledata = b->map[t];
				output.write_dword(tiledata);
			}
		}

		/*
		std::vector<std::vector<int>> ptlist;

		if (!edgify(ptlist))
		{
			throw "Edgify failed!";
		}

		// edges
		int num_edges = ptlist.size();

		// num of edges
		output.write_dword(num_edges);

		for (int j = 0; j < num_edges; j++)
		{
			std::vector<int> points = ptlist.at(j);
			int num_points = points.size();

			output.write_dword(num_points);
			for (int k = 0; k < num_points; k++)
			{
				output.write_dword(points[k]);
			}
		}
		*/
	}
	catch (ios_base::failure&)
	{
		output.close();
		return false;
	}
	catch (int e)
	{
		output.close();
		return false;
	}

	return true;
}


void MainWindow::writeLevelFile(QString& filename)
{
	BinaryFile output;

	const char hslx_id[4] = { 0x48, 0x53, 0x4c, 0x58 };
	const unsigned int level_version = 0x10003;

	int num_objects = m_level->numObjects();
	int num_tiles = m_level->getNumTiles();

	try
	{
		output.open(filename.toStdString(), BinaryFile::MODE_WRITEONLY);

		// ID
		output.write((char*)hslx_id, 4);

		// BLB version
		output.write_dword(level_version);

		// Texture name
		QString texname_stripped = m_texture_file.section('/', -1);
		QByteArray texname = texname_stripped.toLocal8Bit();
		for (int i=0; i < texname.length(); i++)
		{
			output.write_byte(texname.at(i));
		}
		output.write_byte(0);	// null terminator

		// Number of objects
		output.write_dword(num_objects);

		// Objects
		for (int i=0; i < num_objects; i++)
		{
			Level::Object* obj = m_level->getObject(i);
			int num_points = obj->getNumPoints();
			Level::ObjectType type = obj->getType();
			int z = obj->getZ();

			// Object type
			output.write_dword(type);

			// Num points
			output.write_dword(num_points);

			// Object Z
			output.write_dword(z);

			// object color
			output.write_dword(obj->getColor());

			// points
			for (int p=0; p < num_points; p++)
			{
				glm::vec2 point = obj->getPoint(p);
				glm::vec2 uv = obj->getUV(p);
				
				output.write_float(point.x);
				output.write_float(point.y);
				output.write_float(uv.x);
				output.write_float(uv.y);
			}

			// parameters
			switch (type)
			{
				case Level::OBJECT_TYPE_DESTRUCTIBLE:
				{
					break;
				}
				case Level::OBJECT_TYPE_TRIGGER:
				{
					// trigger type
					output.write_dword(obj->getParam(0).i);
					output.write_dword(obj->getParam(1).i);
					output.write_dword(obj->getParam(2).i);
					output.write_dword(obj->getParam(3).i);
					break;
				}
				case Level::OBJECT_TYPE_MOVER:
				{
					output.write_dword(obj->getParam(0).i);
					output.write_dword(obj->getParam(1).i);
					output.write_dword(obj->getParam(2).i);
					output.write_dword(obj->getParam(3).i);
					output.write_dword(obj->getParam(4).i);
					break;
				}
			}
		}

		// Number of tiles
		output.write_dword(num_tiles);

		for (int i=0; i < num_tiles; i++)
		{
			const Tilemap::Tile* tile = m_level->getTile(i);	

			// tile color
			output.write_dword(tile->color);

			// tile type
			output.write_dword(tile->type);

			int num_top_points = 0;

			switch (tile->type)
			{
				case Tilemap::TILE_FULL:		num_top_points = 6; break;				
				case Tilemap::TILE_LEFT:		num_top_points = 4;	break;				
				case Tilemap::TILE_RIGHT:		num_top_points = 4; break;
				case Tilemap::TILE_TOP:			num_top_points = 3; break;				
				case Tilemap::TILE_BOTTOM:		num_top_points = 3; break;
				case Tilemap::TILE_MID:			num_top_points = 4; break;
				case Tilemap::TILE_CORNER_TL:	num_top_points = 3; break;
				case Tilemap::TILE_CORNER_TR:	num_top_points = 3; break;
				case Tilemap::TILE_CORNER_BL:	num_top_points = 3; break;
				case Tilemap::TILE_CORNER_BR:	num_top_points = 3; break;
			}

			for (int j = 0; j < num_top_points; j++)
			{
				output.write_float(tile->top_points[j].x);
				output.write_float(tile->top_points[j].y);
			}

			for (int j = 0; j < 4; j++)
			{
				output.write_float(tile->side_points[j].x);
				output.write_float(tile->side_points[j].y);
			}
		}

		// Tilemap

		// buckets
		std::vector<Tilemap::Bucket*> buckets;

		int total_buckets = (Tilemap::AREA_WIDTH / Tilemap::BUCKET_WIDTH) * (Tilemap::AREA_HEIGHT / Tilemap::BUCKET_HEIGHT);
		for (int i = 0; i < total_buckets; i++)
		{
			Tilemap::Bucket* b = m_level->getTileBucket(i);
			if (b != nullptr)
			{
				buckets.push_back(b);
			}
		}

		// num buckets
		output.write_dword(buckets.size());

		for (int i = 0; i < buckets.size(); i++)
		{
			Tilemap::Bucket* b = buckets.at(i);
			output.write_dword(b->x);
			output.write_dword(b->y);

			for (int t = 0; t < (Tilemap::BUCKET_WIDTH * Tilemap::BUCKET_HEIGHT); t++)
			{
				int tile = b->map[t] & Tilemap::TILE_MASK;
				int z = (b->map[t] & Tilemap::Z_MASK) >> Tilemap::Z_SHIFT;
				output.write_word(tile);
				output.write_byte(z);
			}
		}
		

		/*
		int tm_width = m_level->getTilemapWidth();
		int tm_height = m_level->getTilemapHeight();

		// Tilemap data
		for (int j=0; j < tm_height; j++)
		{
			for (int i=0; i < tm_width; i++)
			{
				int data = m_level->readTilemapRaw(i, j) + 1;

				assert(data >= 0 && data < 32768);

				// encode to var num
				if (data < 128)
				{
					// single byte, bit 0x80 is zero
					output.write_byte(data);
				}
				else
				{
					// first byte has bit 0x80 set, and the low 7 bits of word
					output.write_byte((data & 0x7f) | 0x80);

					// second byte has bits 8-14
					output.write_byte((data >> 7) & 0xff);
				}
			}
		}
		*/

		// Edges
		std::vector<std::vector<int>> ptlist;

		if (!edgify(ptlist))
		{
			throw "Edgify failed!";
		}

		// edges
		int num_edges = ptlist.size();

		// num of edges
		output.write_dword(num_edges);

		for (int j = 0; j < num_edges; j++)
		{
			std::vector<int> points = ptlist.at(j);
			int num_points = points.size();

			output.write_dword(num_points);
			for (int k = 0; k < num_points; k++)
			{
				output.write_dword(points[k]);
			}
		}

	}
	catch (ios_base::failure&)
	{
		output.close();
		return;
	}
	catch (int e)
	{
		output.close();
		return;
	}
}