#include <ViewerWindow.h>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QBoxLayout>
#include <QLineEdit>
#include <QTcpSocket>

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
				this->connect(serverEntry->text());
			});
	}

	{
		QWidget* readoutWidget = new QWidget(this);

		QSlider* readoutSlider = new QSlider();
		QLabel* readoutLabel = new QLabel("1234");
		QBoxLayout* readoutLayout = new QBoxLayout(QBoxLayout::LeftToRight);
		readoutLayout->addWidget(readoutSlider);
		readoutLayout->addWidget(readoutLabel);
		readoutWidget->setLayout(readoutLayout);

		QObject::connect(this, &ViewerWindow::newReadout,
			[readoutSlider, readoutLabel](float readout)
			{
				QString formatted;
				formatted.setNum(readout);
				readoutLabel->setText(formatted);
				readoutSlider->setValue(readout);
			});
		combinedLayout->addWidget(readoutWidget);
	}

	
	combinedWidgets->setLayout(combinedLayout);
	setCentralWidget(combinedWidgets);
}

void ViewerWindow::connect(QString address)
{
	QStringList split = address.split(':');
	printf("Connecting to %s port %s\n",
			split.at(0).toLocal8Bit().constData(),
			split.at(1).toLocal8Bit().constData());

	m_dataConnection = std::make_unique<QTcpSocket>();
	m_dataConnection->connectToHost(split.at(0), split.at(1).toUShort());

	QObject::connect(m_dataConnection.get(), &QTcpSocket::readyRead,
		[this]()
		{
			float packet[100];
			int bytesRead = this->m_dataConnection->read((char*)&packet, sizeof(packet));

			float lastValue = packet[bytesRead / sizeof(packet[0]) - 1];
			lastValue = 100.0f * (lastValue / 5374017);
			this->newReadout(lastValue);
		});
}

ViewerWindow::~ViewerWindow() = default;
