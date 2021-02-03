#include "KAnalysis.h"
#include "KSpecWidget.h"
#include "KSpectrogram.h"

KSpecWidget::KSpecWidget(
  KAnalysis*_boss,int _ch,int _w, int _h, 
  int _shift_size,int _sample_rate,int _scale,
  int _fmt_type,int _id):
  QWidget(),
  btn_close("close"),
  label_ch( QString::number(_ch)),
  label_sample_rate( QString::number(_sample_rate)),
  btn_export("export"),
  shift_size(_shift_size),
  sample_rate(_sample_rate),
  length(_w),
  fmt_type(_fmt_type),
  id(_id)
{
  boss = _boss;
  spec = new KSpectrogram(this, _w, _h, _scale, _id);
  /* btn_close */
  layout_west.addWidget(&btn_close);
  QObject::connect(&btn_close, &QPushButton::clicked, [&]() {
                   boss->CloseSpec(this);
                  }
  );
  layout_west.addWidget(&label_ch);
  layout_west.addWidget(&label_sample_rate);
  /* btn_export */
  QObject::connect(&btn_export, &QPushButton::clicked, [&](){
      Export();
      }
  );
  
  layout_west.addWidget(&btn_export);
  layout_west.setAlignment ( Qt::AlignTop);

  //QString tq = _BG_COLOR_3_;
  //widget_west.setStyleSheet(tq);

  widget_west.setLayout(&layout_west);
  //widget_west.setFixedWidth(width_widget_west);
  widget_west.setFixedSize(width_widget_west,_h/_scale);



  layout.addWidget(&widget_west);
  layout.addWidget(spec);

  // Eliminate Margins
  layout.setContentsMargins(0,0,0,0);
  layout.setAlignment ( Qt::AlignTop|Qt::AlignLeft);

  setLayout(&layout);


  /* Buffer for each channel, it will be filled at KAnalysis */
  switch(fmt_type){
    case 3:
      buffer_wav = new float[(length * shift_size)];
      break;
    default:
      buffer_wav = new short[(length * shift_size)];
      break;
  }
}

KSpecWidget::~KSpecWidget(){
	delete spec;
  switch(fmt_type){
    case 3:
  delete[] reinterpret_cast<float*>(buffer_wav);
      break;
    default:
  delete[] reinterpret_cast<short*>(buffer_wav);
      break;
  }
}

void KSpecWidget::Update(double *buffer){
  spec->Update(buffer);
}

void KSpecWidget::SetRange(int a,int b){
  spec->SetRange(a,b);
}

void KSpecWidget::Close(){
  boss->CloseSpec(this);
}

inline void KSpecWidget::Export(){
  QString fileName;
  QString format;
  QFileDialog dialog;
  /* QString QFileDialog::getSaveFileName(
   * QWidget *parent = nullptr, const QString &caption = QString(),
   * const QString &dir = QString(), const QString &filter = QString(),
   * QString *selectedFilter = nullptr, QFileDialog::Options options = ...) */
  fileName = dialog.getSaveFileName(this,
      tr("Save Spectrogram"),
      "./spectrogram", 
       /* TODO
       * If you set like *.jpg, *.png then these can be searched by REGEX.
       * You can get conventional dialog for jpg,png s.
       * But you will get string for format as '*.png',
       * Therefore you need to map it to 'PNG'
       * I keep current state for now.
       * */
      tr("JPEG;;PNG;;BMP"),
      &format,
      QFileDialog::DontUseNativeDialog
      );
  std::cout<<"NOTE::"<<fileName.toStdString()<<"."
    <<format.toLower().toStdString()<<"\n";
  /*  bool QImage::save(const QString &fileName, 
   * const char *format = nullptr, int quality = -1) const */
  /* quality [0,100] : 0 - compressed, 100 - uncompressed */
  (spec->GetPixmapPointer())->save(fileName+"."+format.toLower() ,
      format.toStdString().c_str(),100);
}

void KSpecWidget::Confirm(){
  spec->Confirm();
}

int KSpecWidget::GetSpecWidth(){
  return spec->width;
}
int KSpecWidget::GetSpecHeight(){
  return spec->height;

}
void* KSpecWidget::Get_buffer_wav(){
  return buffer_wav;
}

void KSpecWidget::Indicator(int pos){
  spec->Indicator(pos);
}

void KSpecWidget::Refresh(){
  spec->Refresh();
}

void KSpecWidget::SetDrawable(bool flag){
  spec->SetDrawable(flag);
}

void KSpecWidget::UpMoveEvent(QPoint global,QPoint origin){
  boss->SetArea(global,origin);
}

void KSpecWidget::SetArea(QPoint global,QPoint origin){
  spec->SetArea(global,origin);
}

bool KSpecWidget::IsInArea(){
  return spec->IsInArea();
}

KAnalysis* KSpecWidget::GetBoss(){
  return boss;
}

void KSpecWidget::UpRefresh(){
  boss->RefreshSpecs();
}

void KSpecWidget::UpRange(int from, int to){
  boss->SetRange(from,to);
}


int KSpecWidget::Get_id(){
  return id;
}
int KSpecWidget::Get_length(){
  return length;
}

bool KSpecWidget::IsDrawable(){
  return spec->IsDrawable();
}

int KSpecWidget::Get_fmt_type(){
  return fmt_type;
}
