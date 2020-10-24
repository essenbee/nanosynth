#pragma once
#include "filter.h"
#include "VAOnePoleFilter.h"

class CDiodeLadderFilter : public CFilter
{
public:
	CDiodeLadderFilter(void);
	~CDiodeLadderFilter(void);

	CVAOnePoleFilter m_LPF1;
	CVAOnePoleFilter m_LPF2;
	CVAOnePoleFilter m_LPF3;
	CVAOnePoleFilter m_LPF4;

	// variables
	double m_dK;			// K, set with Q
	double m_dGamma;		// needed for final calc and update
	double m_dSG1; 
	double m_dSG2; 
	double m_dSG3; 
	double m_dSG4; 

	// -- CFilter Overrides --
	virtual void reset();
	virtual void setQControl(double dQControl);
	virtual void update();

	inline virtual double doFilter(double xn)
	{
		// --- return xn if filter not supported
		if(m_uFilterType != LPF4)
			return xn;

		m_LPF4.setFeedback(0.0);
		m_LPF3.setFeedback(m_LPF4.getFeedbackOutput());
		m_LPF2.setFeedback(m_LPF3.getFeedbackOutput());
		m_LPF1.setFeedback(m_LPF2.getFeedbackOutput());

		// --- form input
		double dSigma = m_dSG1*m_LPF1.getFeedbackOutput() + 
						m_dSG2* m_LPF2.getFeedbackOutput() +
						m_dSG3*m_LPF3.getFeedbackOutput() +
						m_dSG4* m_LPF4.getFeedbackOutput();

		// --- for passband gain compensation!
		xn *= 1.0 + m_dAuxControl*m_dK;

		// --- form input
		double dU = (xn - m_dK*dSigma)/(1 + m_dK*m_dGamma);

		// ---NLP
		if(m_uNLP == ON)
			dU = fasttanh(m_dSaturation*dU);

		// --- cascade of four filters
		return m_LPF4.doFilter(m_LPF3.doFilter(m_LPF2.doFilter(m_LPF1.doFilter(dU))));
	}
};
