#pragma once
#include <QMainWindow>
#include <memory>

class QString;
class QGraphicsScene;
class QGraphicsPixmapItem;
class QPushButton;
class QSlider;

class ViewerWindow
	: public QMainWindow
{
	Q_OBJECT
	public:
		ViewerWindow();
		~ViewerWindow();

		void setDisplayedManualControls(bool lockedX, float fracX, bool lockedY, float fracY);

	signals:
		// Update display with most recently read value, in range 0-1
		void newReadout(float readout);
		void connectRequested(QString hostname, uint16_t port);
		void saveImage(QString filenameOut);
		void newResolutionRequested(int res);
		void clearImageRequested();

		// Emitted when the user has requested a change to the beam manual controls
		// If lockedX/lockedY are true, that axis is under user-control, and will be set
		// to the fraction fracX/fracY along the image resolution; otherwise, if an axis
		// is not locked, the beam will be automatically incremented by the beam
		// controller, and the fractions will be ignored.
		void newBeamStateRequested(bool lockedX, float fracX, bool lockedY, float fracY);
	
	public slots:
		void updateImage(QImage* imageData);
		void setImageSize(QRect imageSize);

		void lockUnlockClicked(bool);

	protected:
		void emitManualControlOverride();

		QGraphicsScene* m_imageScene;
		QGraphicsPixmapItem* m_imageDisplayItem;

		QPushButton* m_manualControlToggles[2];
		QSlider* m_manualControlSliders[2];
};

