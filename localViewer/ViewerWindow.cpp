#include <ViewerWindow.h>
#include <QPushButton>
#include <QToolButton>
#include <QDialogButtonBox>
#include <QSlider>
#include <QLabel>
#include <QBoxLayout>
#include <QLineEdit>
#include <QPixmap>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QScrollBar>
#include <QFileDialog>
#include <QWheelEvent>
#include <QApplication>
#include <QMenu>
#include <QComboBox>
#include <algorithm>

namespace
{
class ZoomingGraphicsView
	: public QGraphicsView
{
public:
	ZoomingGraphicsView(QGraphicsScene* scene) : QGraphicsView(scene) {}

	virtual void wheelEvent(QWheelEvent* event) override
	{
		if(QApplication::keyboardModifiers() & Qt::ControlModifier)
		{
			float curScale = transform().m11();
			if(event->delta() > 0 && curScale < 16.0f)
			{
				scale(2.0f, 2.0f);
			}
			else if(event->delta() < 0 && curScale > 0.5f)
			{
				scale(0.5f, 0.5f);
			}
		}
		else
		{
			QGraphicsView::wheelEvent(event);
		}
	}
};

class ResolutionSelectionDialog
	: public QDialog
{
public:
	ResolutionSelectionDialog(QWidget* parent = nullptr)
		: QDialog(parent)
	{
		QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight);
		setLayout(layout);

		m_comboBox = new QComboBox;
		m_comboBox->addItem("128", QVariant(128));
		m_comboBox->addItem("256", QVariant(256));
		m_comboBox->addItem("512", QVariant(512));
		m_comboBox->addItem("1024", QVariant(1024));

		QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
		connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

		layout->addWidget(m_comboBox);
		layout->addWidget(buttons);
	}

	int selectedResolution() const
	{
		int selectedIdx = m_comboBox->currentIndex();
		return m_comboBox->itemData(selectedIdx).toInt();
	}

	QComboBox* m_comboBox;
};
}

ViewerWindow::ViewerWindow()
{
	QWidget* combinedWidgets = new QWidget(this);
	QBoxLayout* combinedLayout = new QBoxLayout(QBoxLayout::TopToBottom);

	{
		QWidget* connectWidget = new QWidget();
		QBoxLayout* connectionLayout = new QBoxLayout(QBoxLayout::LeftToRight);

		QLineEdit* serverEntry = new QLineEdit("192.168.0.23:3017");
		connectionLayout->addWidget(serverEntry);

		QMenu* connectionMenu;
		{
			connectionMenu = new QMenu;
			connectionMenu->addAction("Lock/Unlock X",
				[this]()
				{
					this->toggleXLockRequested();
				});
			connectionMenu->addAction("Change resolution",
				[this]()
				{
					ResolutionSelectionDialog diag;
					if(diag.exec() == QDialog::Accepted)
					{
						this->newResolutionRequested(diag.selectedResolution());
					}
				});
		}

		QToolButton* connectButton = new QToolButton;
		connectButton->setText("Connect");
		connectButton->setMenu(connectionMenu);
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

		QPushButton* saveButton = new QPushButton("Save");
		connectionLayout->addWidget(saveButton);

		QObject::connect(saveButton, &QPushButton::clicked,
			[this](bool)
			{
				QString saveFile = QFileDialog::getSaveFileName(this,
						"Save file", QString(), "Image (*.png)");
				if(saveFile != nullptr)
				{
					this->saveImage(saveFile);
				}
			});
	}

	QWidget* liveDisplay = new QWidget(this);
	QBoxLayout* liveLayout = new QBoxLayout(QBoxLayout::LeftToRight);
	{
		QWidget* readoutWidget = new QWidget(this);

		QSlider* readoutSlider = new QSlider();
		QLabel* readoutLabel = new QLabel;
		QBoxLayout* readoutLayout = new QBoxLayout(QBoxLayout::TopToBottom);
		readoutLayout->addWidget(readoutSlider);
		//readoutLayout->addWidget(readoutLabel); //<TODO.eoin Calc minimum size and re-add
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
		m_imageScene = new QGraphicsScene();
		m_imageDisplayItem = m_imageScene->addPixmap(QPixmap::fromImage(*imageData));
		m_imageScene->setSceneRect(imageData->rect());

		// Scene makes a copy of the QPixmap, so we need to explicitly
		// set the data again after it is changed:
		imageData->fill(QColor::fromRgb(0xaaaaff));
		m_imageDisplayItem->setPixmap(QPixmap::fromImage(*imageData));

		ZoomingGraphicsView* imageView = new ZoomingGraphicsView(m_imageScene);
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

void ViewerWindow::setImageSize(QRect imageSize)
{
	m_imageScene->setSceneRect(imageSize);
}
