void CMainDlg::DoSaveRecord()
{	...
	
	DWORD TickCount = GetTickCount() - m_dwLastTickCount;
	DWORD EndTime = m_dwLastWavTime + TickCount;
	ASSERT(EndTime > m_dwStartTime);
	
	INT Valid = 1;

	//hyf(2010-03-30):在此添加语音质量是否合格的判断
	//	首先需要判断录音时长，一般来说，如果语音时长小于（头静音+尾静音+最短语音时长（例如：200ms）），那么认为这段语音不合格；
	if(EndTime - m_dwStartTime < m_iMinWavLen)
		Valid = 0;
	else if (m_bAutoWavCheck) {
		if (m_nSentSize > 0) {
			// 创建对象
			//	注意：本程序只针对16bits数据进行处理，因此m_waveform.wBitsPerSample应该是16；
			SpLib_Quality Chk_Qlt(m_waveform.nSamplesPerSec, m_waveform.wBitsPerSample, 0.01);
			// 将语音数据feed进去，同时给出采样点数
			Chk_Qlt.feed((short*)m_pSentBuf, m_nSentSize/sizeof(short));
			
			// 调用判断函数即可；
			if (Chk_Qlt.isCLPOK() == FALSE || Chk_Qlt.isSNROK() == FALSE 
				|| Chk_Qlt.isSilence() || Chk_Qlt.isNoise()) 
				Valid = 0;
		}
	}

	...}