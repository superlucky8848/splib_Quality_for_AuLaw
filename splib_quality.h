#ifndef __SPLIB_QUALITY_H__
#define __SPLIB_QUALITY_H__

#define MAX 32256        /* max sample value, for clipping rate */
#define MIN -32256       /* min sample value, for clipping rate */
#define NOISEPERC 5      /* percentage of histogram, considered as noise */
#define square(x) (x)*(x)

class SpLib_Quality {
public: 
	SpLib_Quality (int Freq, int nBits, float WinSize, int nClipMethod = 1) {
		m_SamFreq = Freq;
		m_BitsPerSample = nBits;
		m_WindowSize = WinSize;
		m_maxAmp = 0;
		m_minAmp = 0;
		m_meanAmp = 0;
		m_samples = 0;
		m_clp = 0;
		m_snr = 0;
		m_nClipMethod = nClipMethod;
	};
	~SpLib_Quality() {};
	
public:
	void feed(short* buf, int sample_count);

	bool isCLPOK (float fThresold = 0.0) { return (m_clp <= fThresold); }
	bool isSNROK (float fThresold = 5.0) { return (m_snr >= fThresold); }
	bool isNoise ();
	bool isSilence ();
	
private:
	int	time2nosamples(float fLen) { return (int)(fLen * m_SamFreq); }
	
private:
	int m_SamFreq;
	int m_BitsPerSample;
	float m_WindowSize;
	
	int m_maxAmp;
	int m_minAmp;
	int m_samples;
	float m_meanAmp;
	float m_clp;
	float m_snr;

	int m_nClipMethod;
};

#endif
