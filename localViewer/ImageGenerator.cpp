#include <ImageGenerator.h>
#include <QImage>

// Color this many of the most recently recieved pixels differently:
static const int newQueueSize = 100;

// Rescale an input reading; red for new pixels, grey for old
template<bool NEW_PIXEL>
inline QRgb u32ToRgb(uint32_t in)
{
	// Apparently a 23 bit input value; just linearly scale it for now
	//uint8_t val = (uint8_t)((in & 0x007f8000) >> 15);
	// In practice, seeing a max value of order magnitude 0x006400a6
	uint8_t val = (uint8_t)((255.0f) * ((float)in / (float)0x6fffff));

	if(NEW_PIXEL)
	{
		return 0xff000000 | (val << 16);
	}
	else
	{
		return 0xff000000 | val | (val << 8) | (val << 16);
	}
}

ImageGenerator::ImageGenerator()
{
	m_imageData = std::make_unique<QImage>(1024, 1024, QImage::Format_RGB32);
}

ImageGenerator::~ImageGenerator() = default;

void ImageGenerator::updatePixels(Packets::CurrentState* newStates, int numStates)
{
	for(int i = 0; i < numStates; i++)
	{
		m_recvQueue.push_back(newStates[i]);
	}

	while(m_recvQueue.size() > newQueueSize)
	{
		const auto& last = m_recvQueue.front();
		m_imageData->setPixel(last.m_x, last.m_y, u32ToRgb<false>(last.m_input0));
		m_recvQueue.pop_front();
	}

	for(const Packets::CurrentState& newPixel : m_recvQueue)
	{
		m_imageData->setPixel(newPixel.m_x, newPixel.m_y, u32ToRgb<true>(newPixel.m_input0));
	}

	// Colour the most recently recieved pixel bright blue, so we can see the read head
	{
		const Packets::CurrentState& newest = m_recvQueue.back();
		m_imageData->setPixel(newest.m_x, newest.m_y, 0xff0000ff);
	}

	emit updatedImage(m_imageData.get());
}

void ImageGenerator::saveImage(QString filenameOut)
{
	m_imageData->save(filenameOut);
}
