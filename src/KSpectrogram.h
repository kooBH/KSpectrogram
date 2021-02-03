#ifndef _H_SPECTROGRAM_
#define _H_SPECTROGRAM_

#include <QWidget>

/*** Spectrogram ***/
#include <QOpenGLWidget>

#include <QPainter>
#include <QBrush>

#include <QVBoxLayout>
#include <QImage>              
#include <QPixmap>            
#include <QColor>

#include <atomic>
#include <math.h>
#include <cmath>

#include "ColorMap.h"

class KAnalysis;
class KSpecWidget;

//class KSpectrogram : public QWidget{
class KSpectrogram : public QOpenGLWidget {
	Q_OBJECT

 public slots:
	 void Indicator(int pos);
protected:
	void paintEvent(QPaintEvent *event) override;
private:
	int pos_draw;
	bool flag_alt;

	// spec 생성 시에만  사용
	QImage *img;
	// 출력용 버퍼
	QPixmap buf; // 원본 버퍼
	QPixmap buf_alt; // 출력용 버퍼
	QVBoxLayout layout;

	KSpecWidget*boss;

	/* idx for draw pixmap col by col */
	int idx;
	int pos_x, pos_y;
	double r_min, r_max;

	int scale;

	// show image with area display.
	int x_origin, y_origin;
	int x_local;
	int id;
	bool in_area, is_fresh;

	/* Audio Play */
	int offset_start, offset_end;
	int offset_play;

	std::atomic<bool> drawable;

	void UpRange(int from, int to);

	QBrush brush_semi_black;
	QBrush brush_semi_white;
	QBrush brush_white;

public:
	int width;
	int height;

	KSpectrogram(KSpecWidget*boss, int w, int h, int scale, int id);
	~KSpectrogram();

	// set range of spectrogarm values.
	void SetRange(double minimum, double maximum);
	// update each column of spectrogram.
	void Update(double*);
	// confrim finsih of updating and display spectrogram .
	void Confirm();
	void LockDrawing();

	QPixmap* GetPixmapPointer();

//	void Indicator(int pos);
	void UpRefresh();
	void Refresh();

	bool IsDrawable();
	bool IsInArea();

	void SetArea(QPoint, QPoint);
	void SetDrawable(bool flag);

	inline void UpMoveEvent(QPoint, QPoint);

	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseDoubleClickEvent(QMouseEvent * event);

};

#endif
