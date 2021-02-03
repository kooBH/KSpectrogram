#include "KAnalysis.h"
#include "KSpecWidget.h"
#include "KSpectrogram.h"


KSpectrogram::KSpectrogram(KSpecWidget* _boss,int w,int h,int _scale,int _id):
  QOpenGLWidget(),
  drawable(true),
  idx(0),
  scale(_scale),
  width(w),
  height((h/2)/_scale),
  x_origin(0),
  y_origin(0),
  id(_id),
  in_area(false),
  is_fresh(true),
  //paint(&buf_alt),
  brush_semi_black(QColor(0,0,0,64),Qt::SolidPattern),
  brush_semi_white(QColor(255,255,255,128),Qt::Dense4Pattern),
  brush_white(QColor(255,255,255,255),Qt::SolidPattern),
  flag_alt(false)
{
  img = new QImage(width,(h/2)/_scale,QImage::Format_RGB16);
  boss =_boss;
#ifndef NDEBUG
 // printf("INFO::spectrogram Img : %d %d\n",img->width(),img->height());
#endif
  setFixedSize(img->width(),img->height());
  setAutoFillBackground(true);
}

KSpectrogram::~KSpectrogram(){}

void KSpectrogram::SetRange(double v_min,double v_max){
  r_min = v_min;
  r_max = v_max;
}

void KSpectrogram::Update(double*buffer){


#pragma omp parallel for 
    for(int j=0;j<height;j++){
      int r,g,b;
      double t = buffer[scale*j];

  //    printf("buffer[%d*%d] : %lf -> ", scale, j, t);

/*
https://stackoverflow.com/questions/929103/convert-a-number-range-to-another-range-maintaining-ratio
NewValue = (((OldValue - OldMin) * (NewMax - NewMin)) / (OldMax - OldMin)) + NewMin
*/
     /* proper domain for tanh is range (-3,3) */
    //  printf("%d %lf\n",j,t);
     // t = (t +80 ) / 100 -2;
      t = (((t - r_min) *  4) / (r_max- r_min)) -2;
     // t = (((t - r_min) * 6 ) / (r_max- r_min)) -3;
           /* tanh = (-1,1)*/
      t = tanh(t);
     /* Jet Colormap 이 주로 쓰는 칼라맵 */
      // color.GetHotToCold(t,&r,&g,&b);
      ColorMap::GetJet(t,&r,&g,&b);

 //     printf("%d %d %d\n", r,g,b);
        
      img->setPixelColor(idx,height-1-j,QColor(r,g,b,255));
    }
    idx++;
}

void KSpectrogram::Confirm(){
  buf.convertFromImage(*img);
  delete img;
  buf_alt = buf.copy();
  pos_x = this->mapToGlobal(this->pos()).x();
  pos_y = this->mapToGlobal(this->pos()).y();
}


void KSpectrogram::Indicator(int pos){
  if(!in_area)return;
  /* 선택된 영역위에 그림 */
  flag_alt = true;
  QPainter paint;  
  paint.begin(&buf_alt);
  paint.fillRect(pos,0,4,height,brush_semi_black);
  paint.end();

  update();
}


void KSpectrogram::paintEvent(QPaintEvent *event){
//  printf("%d %d\n",id,pos);
  QPainter paint;
  paint.begin(this);
  if(flag_alt){
    paint.drawPixmap(0,0,buf.width(),buf.height(),buf_alt);  
  }
  else{
    paint.drawPixmap(0,0,buf.width(),buf.height(),buf);  
  }
  paint.end();

}

/* select area [clicked point , end] */
void KSpectrogram::mousePressEvent(QMouseEvent *event){
  QPainter paint;

#ifndef NDEBUG
//  std::cout<<"mousePressEvent::Global pos 1::"<<event->globalX()<<" "<<event->globalY()<<"\n";
  //std::cout<<"mosePressEvent::Local  pos 2::"<<event->pos().x()<<" "<<event->pos().y()
  //  <<" "<<this->pos().x()<<"\n";
#endif

  if(!drawable)return;
  UpRefresh();

  paint.begin(&buf_alt);
  in_area=true;
//  int x = event->pos().x()-this->pos().x();
  int x = event->pos().x();
  if(x > width)
    x = width;

  /* QPixmap::fromImage uses single global Pixmap cache.
   * the size global cache is limited. */
  x_origin = x;
//  y_origin = event->pos().y()-this->pos().y();
  y_origin = event->pos().y();
  paint.fillRect(x,0,3,height,brush_white) ;
  paint.end();

  flag_alt = true;

  UpRange(x_origin,width);
  update();
}


void KSpectrogram::mouseReleaseEvent(QMouseEvent *event){
  if(!drawable)return;

}

void KSpectrogram::mouseMoveEvent(QMouseEvent *event){
  if(!drawable)return;
  //  QMouseEvent->pos() is local position.
   //int x = event->pos().x()-this->pos().x();
   int x = event->pos().x();
   if(x<0)x=0;
   if(x>width)x=width;
   QPoint origin(x_origin,y_origin);
#ifndef NDEBUG
   /*
   printf("mouseMoveEvent(%d)::[%d=%d-(%d)][%d,%d][%d,%d]\n",
       id,x_origin,x,x-x_origin,
       this->mapToGlobal(event->pos()).x(),
       this->mapToGlobal(event->pos()).y(),
       this->mapToGlobal(origin).x(),
       this->mapToGlobal(origin).y()
   );
   */
#endif
   UpMoveEvent(this->mapToGlobal(event->pos()),this->mapToGlobal(origin));
   UpRange(x_origin,x);
}

/* Select entrire area */
void KSpectrogram::mouseDoubleClickEvent(QMouseEvent * event){
  if(!drawable)return;
   //reset pix
   buf_alt = buf.copy();
   flag_alt = true;
   // fill 
   QPainter paint(&buf_alt);
   paint.fillRect(0,0,width,height,brush_semi_white);
#ifndef NDEBUG
  // printf("mouseDoubleClickEvent\n");
#endif
   update();
   UpRange(0,width);
}


QPixmap* KSpectrogram::GetPixmapPointer(){
  return &buf;
}


bool KSpectrogram::IsDrawable(){
  return drawable;
}

void KSpectrogram::Refresh(){
  buf_alt = buf.copy();
  in_area=false;
  flag_alt = false;
  update();
}
void KSpectrogram::SetDrawable(bool flag){
  drawable = flag;
}

void KSpectrogram::SetArea(QPoint global, QPoint origin){

  int high = global.y() > origin.y()?global.y():origin.y();
  int low = global.y() > origin.y()?origin.y():global.y();
  
  // 윈도우가 움직였을 수 도 있음.
  QPoint me = this->mapToGlobal( this->pos());

  // this 가 영역에 포함되어 있는가?
  if(me.y() + height > low && me.y() < high)
    in_area = true;
  else
    in_area = false;

#ifndef NDEBUG
  /*
  printf("%d %s at [%d %d]::[%d %d] from [%d %d]\n",
      id,in_area?"O":"X",
      me.x(),me.y(),global.x(),global.y(),origin.x(),origin.y());
      */
#endif
 
  // 선택 영역에 속하지 않음.
  if(!in_area){
    // 의미 없는 refresh 방지.
    if(is_fresh)return;
    Refresh();
    is_fresh = true;
    return;
  }
  is_fresh = false;
  flag_alt = true;

  x_origin = this->mapFromGlobal(origin).x();
  x_local= (this->mapFromGlobal(global)).x();
  
  buf_alt = buf.copy(); 
  QPainter paint(&buf_alt);
  paint.fillRect(x_origin,0,x_local-x_origin,height,brush_semi_white);
  paint.end();
  update();
}

inline void KSpectrogram::UpMoveEvent(QPoint global,QPoint origin){
  boss->UpMoveEvent(global,origin);
}

bool KSpectrogram::IsInArea(){
  return in_area;
}

void KSpectrogram::UpRefresh(){
  boss->UpRefresh();
}

void KSpectrogram::UpRange(int from, int to){
  if(to > from)
    boss->UpRange(from,to);
  else
    boss->UpRange(to,from);
}
