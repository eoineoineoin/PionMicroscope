#include <ViewerWindow.h>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QBoxLayout>
#include <QLineEdit>
#include <QPixmap>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QScrollBar>

ViewerWindow::ViewerWindow()
{
	QWidget* combinedWidgets = new QWidget(this);
	QBoxLayout* combinedLayout = new QBoxLayout(QBoxLayout::TopToBottom);

	{
		QWidget* connectWidget = new QWidget();
		QBoxLayout* connectionLayout = new QBoxLayout(QBoxLayout::LeftToRight);

		QLineEdit* serverEntry = new QLineEdit("192.168.0.23:3017");
		connectionLayout->addWidget(serverEntry);

		QPushButton* connectButton = new QPushButton("Connect");
		connectionLayout->addWidget(connectButton);

		connectWidget->setLayout(connectionLayout);
		combinedLayout->addWidget(connectWidget);

		QObject::connect(connectButton, &QPushButton::clicked,
			[serverEntry, this](bool)
			{
				QStringList split = serverEntry->text().split(':');
				printf("Connecting to %s port %s\n",
						split.at(0).toLocal8Bit().constData(),
						split.at(1).toLocal8Bit().constData());

				this->connectRequested(split.at(0), split.at(1).toUShort());
			});
	}

	QWidget* liveDisplay = new QWidget(this);
	QBoxLayout* liveLayout = new QBoxLayout(QBoxLayout::LeftToRight);
	{
		QWidget* readoutWidget = new QWidget(this);

		QSlider* readoutSlider = new QSlider();
		QLabel* readoutLabel = new QLabel("0");
		QBoxLayout* readoutLayout = new QBoxLayout(QBoxLayout::TopToBottom);
		readoutLayout->addWidget(readoutSlider);
		//readoutLayout->addWidget(readoutLabel);
		readoutWidget->setLayout(readoutLayout);

		QObject::connect(this, &ViewerWindow::newReadout,
			[readoutSlider, readoutLabel](float readout)
			{
				QString formatted;
				formatted.setNum(readout, 'g', 3);
				readoutLabel->setText(formatted);
				readoutSlider->setValue((int)(readout * readoutSlider->maximum()));
			});
		liveLayout->addWidget(readoutWidget);
	}
	liveDisplay->setLayout(liveLayout);
	combinedLayout->addWidget(liveDisplay);


	// Image display TODO.eoin clean up; this shouldn't be creating images
	{
		QImage* imageData = new QImage(1024, 1024, QImage::Format_RGB32);
		QGraphicsScene* imageScene = new QGraphicsScene();
		m_imageDisplayItem = imageScene->addPixmap(QPixmap::fromImage(*imageData));
		imageScene->setSceneRect(imageData->rect());

		// Scene makes a copy of the QPixmap, so we need to explicitly
		// set the data again after it is changed:
		imageData->fill(QColor::fromRgb(0xaaaaff));
		m_imageDisplayItem->setPixmap(QPixmap::fromImage(*imageData));

		QGraphicsView* imageView = new QGraphicsView(imageScene);
		imageView->scale(4.0, 4.0);
		liveLayout->addWidget(imageView);

		imageView->horizontalScrollBar()->setValue(0);
		imageView->verticalScrollBar()->setValue(0);
		delete imageData;
	}

	
	combinedWidgets->setLayout(combinedLayout);
	setCentralWidget(combinedWidgets);
}

ViewerWindow::~ViewerWindow() = default;

void ViewerWindow::updateImage(QImage* newImage)
{
	m_imageDisplayItem->setPixmap(QPixmap::fromImage(*newImage));
}
