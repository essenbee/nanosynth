# pragma once

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
	COscillator::reset();
}

void CLFO::startOscillator()
{
	if (m_uLFOMode == sync || m_uLFOMode == shot)
	{
		reset();
	}

	m_bNoteOn = true;
}

void CLFO::stopOscillator()
{
	m_bNoteOn = false;
}

