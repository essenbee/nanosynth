#include "LFO.h"

CLFO::CLFO(void)
{
	m_uLFOMode = sync;
}

CLFO::~CLFO(void)
{
}

void CLFO::reset()
{
	// --- call base class
	COscillator::reset();
}

void CLFO::startOscillator()
{
	// --- if one shot or sync'd LFO, reset 
	if(m_uLFOMode == sync || m_uLFOMode == shot)
		reset();

	// --- set flag
	m_bNoteOn = true;
}

void CLFO::stopOscillator()
{
	// --- clear flag
	m_bNoteOn = false;
}

