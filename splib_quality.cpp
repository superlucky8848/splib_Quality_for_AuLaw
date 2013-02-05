#include <stdlib.h>
#include <math.h>
#include "splib_quality.h"

float * ms_windows = NULL;

int cmp(const void *f1, const void *f2) {
  if (*((float *)f1) > *((float *)f2)) return 1;
  else if (*((float *)f1) < *((float *)f2)) return -1;
  else return 0;
}

#define CLP_METHOD	1
void SpLib_Quality::feed(short* buf, int sample_count)
{
  int w_counter = 0;
  int total_no_samples = 0;
  int sample_no = 0;
  int max = 0;
  int min = 0;
  int no_min = 0;
  int no_max = 0;
  int window_size = 0;
  int last_window_size = 0;
  int window_no = 0;
  int w_sample_no = 0;
  int no_windows = 0;
  int no_noise_windows = 0;
  int samples_per_period = 0;   /*һ�������ڵ���������*/
  bool bclp_per_period = false; /*һ���������Ƿ���ڽط�*/
  int total_over = 0;
  long total = 0;
  float mstot = 0;
  float msnoise = 0;
  float window_squared = 0;
  float total_squared = 0;
  float cliprate = 0.0f;
  float mean = 0.0f;
  float snr = 0.0f;

/* find maximum, minimum, number of times these appear, mean, snr */
  
  window_size = time2nosamples(m_WindowSize);
  no_windows = (int)floor(sample_count/(float)window_size);
  last_window_size = sample_count % window_size;
  
  ms_windows = new float[no_windows+10];
/* do for number-of-windows+1 times */
  
  for (window_no=0;window_no*window_size<sample_count+1;window_no++) {
    
/* do for number-of-samples-in-one-window times */
    
    for (w_sample_no=0;w_sample_no<window_size&&sample_no<sample_count;w_sample_no++) {
      
      sample_no = window_no*window_size + w_sample_no; /* absolute sample number */

      total_no_samples++;
      total_squared += square(buf[sample_no]);  /* buf[sample_no] is sample value */
      window_squared += square(buf[sample_no]);
      total += buf[sample_no];
      
      if (buf[sample_no] > max) {
		  max = buf[sample_no];
	  }
      if (buf[sample_no] < min) {
		  min = buf[sample_no];
	  }

	  if (m_nClipMethod == 1) {
		  // criterion of MS for Clip rate determine
		  if (buf[sample_no] >= MAX) {
			  total_over++;
			  if (total_over >= 2) no_max++;
		  } else if (buf[sample_no] <= MIN) {
			  total_over++;
			  if (total_over >= 2) no_min++;
		  } else {
			  total_over = 0;
		  }
	  } else if (m_nClipMethod == 2) {
		  // strictly clip rate method of Kingline
		  if (buf[sample_no] >= MAX || buf[sample_no] <= MIN)
			  bclp_per_period = true;

		  if (sample_no < sample_count-1) {
			  if (buf[sample_no] * buf[sample_no+1] < 0) {
				  if (bclp_per_period) {
					  bclp_per_period = false;
					  no_max += samples_per_period;
				  }
				  samples_per_period = 0;
			  }
		  }
		  samples_per_period ++ ;
	  } else {
		  // general clip rate method
		  if (buf[sample_no] >= MAX) {
			  no_max++;
		  }
		  if (buf[sample_no] <= MIN) {
			  no_min++;
		  }

	  }
    }
    
    if (sample_no < sample_count - last_window_size) { /* only complete windows for snr */
		ms_windows[window_no] = window_squared/(float)window_size;
	}
    window_squared = 0;
  }

  no_noise_windows = no_windows * NOISEPERC /100;

  if (total_no_samples != 0) {  
    mean = (float)total / (float)total_no_samples;
  }

/* compute the clipping rate */
  cliprate = 100 * ((float)no_max + (float)no_min) / (float)total_no_samples;

/* compute snr, distract mean value from sample value */

  sample_no = 0;
  window_squared = 0;
  mstot = 0;
  msnoise = 0;
  
/* do for number-of-windows+1 times */

  for (window_no=0;window_no*window_size<=sample_count;window_no++) {

/* do for number-of-samples-in-one-window times */

    for (w_sample_no=0;w_sample_no<window_size&&sample_no<sample_count;w_sample_no++) {
		
		sample_no = window_no*window_size + w_sample_no; /* absolute sample number */
		
		window_squared += square((float)buf[sample_no]-mean);
      
    }
    
    if (sample_no < sample_count - last_window_size) { /* only complete windows */
		ms_windows[window_no] = window_squared/(float)window_size;
	}
    window_squared = 0;
  }

  qsort((void *)ms_windows,no_windows,sizeof(window_squared),cmp);
  
  for (w_counter=0;w_counter<no_windows;w_counter++) {
    mstot += ms_windows[w_counter];
    if (w_counter < no_noise_windows) {
      msnoise += ms_windows[w_counter];
    }
  }
  
  if (no_noise_windows != 0) {
    snr = (float)(10.0 * log10((mstot/(float)no_windows)/(msnoise/(float)no_noise_windows)));
  }

  if (no_noise_windows == 0) {
    if (no_windows != 0) {
      
	  m_maxAmp = max;
	  m_minAmp = min;
	  m_meanAmp = mean;
	  m_samples = sample_count;
	  m_clp = cliprate;
	}
  }
  else {
	m_maxAmp = max;
	m_minAmp = min;
	m_meanAmp = mean;
	m_samples = sample_count;
	m_clp = cliprate;
	m_snr = snr;
  }

  if(ms_windows) delete [] ms_windows;
}

bool SpLib_Quality::isNoise ()
{
	return (m_maxAmp - m_minAmp < 3000);
}

bool SpLib_Quality::isSilence ()
{
	int nMaxAmp = m_maxAmp;
	nMaxAmp = nMaxAmp > abs(m_minAmp) ? nMaxAmp : abs(m_minAmp);
	return (nMaxAmp < 2000);
}
