#pragma once
#include "filter.h"
#include "VAOnePoleFilter.h"

class CKThreeFiveFilter : public CFilter
{
public:
	CKThreeFiveFilter(void);
	~CKThreeFiveFilter(void);

	// our member filters
	// LPF: LPF1+LPF2+HPF1
	// HPF: HPF1+LPF1+HPF2
	CVAOnePoleFilter m_LPF1;
	CVAOnePoleFilter m_LPF2;
	CVAOnePoleFilter m_HPF1;
	CVAOnePoleFilter m_HPF2;

	// -- CFilter Overrides --
	virtual void reset();
	virtual void setQControl(double dQControl);
	virtual void update();
	virtual double doFilter(double xn);

	// variables
	double m_dAlpha0;   // our u scalar value
	double m_dK;		// K, set with Q
};
