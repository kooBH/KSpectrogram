#ifndef _H_K_SPECWIDGET_
#define _H_K_SPECWIDGET_

class KAnalysis;
class KSpectrogram;

class KSpecWidget : public QWidget {
	//  Q_OBJECT

private:
	//    BorderLayout layout;
	QHBoxLayout layout;
	QWidget widget_west;
	QVBoxLayout layout_west;
	QPushButton btn_close;
	QLabel label_ch;
	QLabel label_sample_rate;
	QPushButton btn_export;
	KSpectrogram* spec;
	KAnalysis*boss;

	int shift_size;
	int sample_rate;
	int length;
	/* wav buffer for each channel */
	void *buffer_wav;
  int fmt_type;
  
	int id;

	const int width_widget_west = 80;

public:
	KSpecWidget(KAnalysis*_boss, int _ch, int _w, int _h, int _shift_size, int _sample_rate, int _scale,int _fmt_type, int _id);
	~KSpecWidget();

	/* Functions for spectrogram drawing */
	// draw a column of spectogram
	void Update(double *buffer);
	// Set min and max to normalize spectrogram
	void SetRange(int, int);
	// Confirm that cpectrum is drawn 
	void Confirm();

	void Close();
	inline void Export();

	void Refresh();
	void SetDrawable(bool);
	bool IsDrawable();
	bool IsInArea();

	void UpMoveEvent(QPoint, QPoint);
	void UpRefresh();
	void UpRange(int from, int to);
	void SetArea(QPoint, QPoint);
	void Indicator(int pos);

	void* Get_buffer_wav();
	int Get_fmt_type();
	int GetSpecWidth();
	int GetSpecHeight();
	int Get_id();
	int Get_length();

	KAnalysis* GetBoss();
};


#endif
