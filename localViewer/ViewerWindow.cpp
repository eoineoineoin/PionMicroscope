#include <ViewerWindow.hpp>
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
#include <cmath>

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

const char* lockLabels[2][2] = {{"Lock X", "Unlock X"}, {"Lock Y", "Unlock Y"}};
}

ViewerWindow::ViewerWindow()
{
	QWidget* combinedWidgets = new QWidget(this);
	QBoxLayout* combinedLayout = new QBoxLayout(QBoxLayout::TopToBottom);

	{
		QWidget* connectWidget = new QWidget();
		QBoxLayout* connectionLayout = new QBoxLayout(QBoxLayout::LeftToRight);

		QLineEdit* serverEntry = new QLineEdit("localhost:3017");
		connectionLayout->addWidget(serverEntry);

		QPushButton* connectButton = new QPushButton("Connect");
		connectButton->setText("Connect");
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

		QPushButton* resolutionButton = new QPushButton("Resolution...");
		connectionLayout->addWidget(resolutionButton);
		QObject::connect(resolutionButton, &QPushButton::clicked,
			[this]()
			{
				ResolutionSelectionDialog diag;
				if(diag.exec() == QDialog::Accepted)
				{
					this->newResolutionRequested(diag.selectedResolution());
				}
			});

		QPushButton* clearButton = new QPushButton("Clear");
		connectionLayout->addWidget(clearButton);
		QObject::connect(clearButton, &QPushButton::clicked, this, &ViewerWindow::clearImageRequested);

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

	// Controls for manaual X/Y control
	{
		QWidget* beamControls = new QWidget();
		QGridLayout* beamControlsLayout = new QGridLayout();

		for(int a = 0; a < 2; a++)
		{
			m_manualControlToggles[a] = new QPushButton(lockLabels[a][0]);
			m_manualControlSliders[a] = new QSlider(Qt::Horizontal);
			m_manualControlSliders[a]->setRange(0, 1000);
			m_manualControlSliders[a]->setValue(500);
			m_manualControlSliders[a]->setEnabled(false);
			beamControlsLayout->addWidget(m_manualControlToggles[a], a, 0);
			beamControlsLayout->addWidget(m_manualControlSliders[a], a, 1);

			QObject::connect(m_manualControlToggles[a], &QPushButton::clicked,
					this, &ViewerWindow::lockUnlockClicked);
			QObject::connect(m_manualControlSliders[a], &QSlider::valueChanged,
					this, &ViewerWindow::emitManualControlOverride);
		}

		beamControls->setLayout(beamControlsLayout);
		combinedLayout->addWidget(beamControls);
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

void ViewerWindow::setDisplayedManualControls(bool lockedX, float fracX, bool lockedY, float fracY)
{
	bool locked[] = {lockedX, lockedY};
	float frac[] = {fracX, fracY};
	
	for(int a = 0; a < 2; a++)
	{
		// Disable signals so we don't re-trigger commands back to the server:
		m_manualControlSliders[a]->blockSignals(true);

		// Make sure the text matches on the button:
		m_manualControlToggles[a]->setText(lockLabels[a][locked[a]]);
		m_manualControlSliders[a]->setEnabled(locked[a]);
		m_manualControlSliders[a]->setValue(std::round(frac[a] * m_manualControlSliders[a]->maximum()));

		// And re-enable the signals again:
		m_manualControlSliders[a]->blockSignals(false);
	}
}

void ViewerWindow::updateImage(QImage* newImage)
{
	m_imageDisplayItem->setPixmap(QPixmap::fromImage(*newImage));
}

void ViewerWindow::setImageSize(QRect imageSize)
{
	m_imageScene->setSceneRect(imageSize);
}

void ViewerWindow::lockUnlockClicked(bool)
{
	int idx = QObject::sender() != m_manualControlToggles[0]; // 0 for x, 1 for y

	bool toggledState = !m_manualControlSliders[idx]->isEnabled();
	m_manualControlSliders[idx]->setEnabled(toggledState);
	m_manualControlToggles[idx]->setText(lockLabels[idx][toggledState]);

	emitManualControlOverride();
}

void ViewerWindow::emitManualControlOverride()
{
	bool xLocked = m_manualControlSliders[0]->isEnabled();
	float fracX = m_manualControlSliders[0]->value() / (float)m_manualControlSliders[0]->maximum();
	bool yLocked = m_manualControlSliders[1]->isEnabled();
	float fracY = m_manualControlSliders[1]->value() / (float)m_manualControlSliders[1]->maximum();

	emit newBeamStateRequested(xLocked, fracX, yLocked, fracY);
}
