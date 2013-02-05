void CMainDlg::DoSaveRecord()
{	...
	
	DWORD TickCount = GetTickCount() - m_dwLastTickCount;
	DWORD EndTime = m_dwLastWavTime + TickCount;
	ASSERT(EndTime > m_dwStartTime);
	
	INT Valid = 1;

	//hyf(2010-03-30):�ڴ�������������Ƿ�ϸ���ж�
	//	������Ҫ�ж�¼��ʱ����һ����˵���������ʱ��С�ڣ�ͷ����+β����+�������ʱ�������磺200ms��������ô��Ϊ����������ϸ�
	if(EndTime - m_dwStartTime < m_iMinWavLen)
		Valid = 0;
	else if (m_bAutoWavCheck) {
		if (m_nSentSize > 0) {
			// ��������
			//	ע�⣺������ֻ���16bits���ݽ��д������m_waveform.wBitsPerSampleӦ����16��
			SpLib_Quality Chk_Qlt(m_waveform.nSamplesPerSec, m_waveform.wBitsPerSample, 0.01);
			// ����������feed��ȥ��ͬʱ������������
			Chk_Qlt.feed((short*)m_pSentBuf, m_nSentSize/sizeof(short));
			
			// �����жϺ������ɣ�
			if (Chk_Qlt.isCLPOK() == FALSE || Chk_Qlt.isSNROK() == FALSE 
				|| Chk_Qlt.isSilence() || Chk_Qlt.isNoise()) 
				Valid = 0;
		}
	}

	...}