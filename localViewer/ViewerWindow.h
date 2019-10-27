#pragma once
#include <QMainWindow>
#include <memory>

class ViewerWindow
	: public QMainWindow
{
	Q_OBJECT
	public:
		ViewerWindow();
		~ViewerWindow();

		void connect(QString address);

	signals:
		void newReadout(float readout);

	protected:
		std::unique_ptr<class QTcpSocket> m_dataConnection;
};

