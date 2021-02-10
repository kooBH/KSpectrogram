#ifndef _H_K_ANALYSIS_
#define _H_K_ANALYSIS_

#include "QLabel.h"
#include "QPushButton.h"
#include "QWidget.h"
#include "BorderLayout.h"
#include "QComboBox.h"

#include <QFileDialog>
#include <QScrollArea>

/*** KSpecWidget ***/
#include <thread>

/** Drag & Drop **/
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QList>
#include <QUrl>

/* STD */
#include <string.h>
#include <string>
#include <vector>

/* Audio Play*/
#include "RtOutput.h"

/** Process **/
#include "WAV.h"
#include "STFT.h"

#include <LogSpec.h>


class KSpectrogram;
class KSpecWidget;

struct device {
  int number;
  std::string name;
//  int channels;
  std::vector<int> samplerate;
};

class KAnalysis : public QWidget{
    Q_OBJECT

  private:

    int width,height;

    BorderLayout layout;
    QWidget widget_button;
    QHBoxLayout layout_button;
      QPushButton btn_play;
      QPushButton btn_stop;
      QPushButton btn_load;
      QPushButton btn_close;
      QLabel label_scale;
      QComboBox combo_scale;
      QLabel label_window;
      QComboBox combo_window;
      QLabel label_sample_rate;
      QComboBox combo_sample_rate;

    QScrollArea area_spec;
    QWidget widget_spec;
    QVBoxLayout layout_spec;

    /* Output device */
    QWidget widget_output;
    QHBoxLayout layout_output;
    QPushButton btn_AudioProbe;
    QLabel label_device;
    QComboBox combobox_device;
    QLabel label_samplerate;
    QComboBox combobox_samplerate;
    //QLabel label_channels;
    //QComboBox combobox_channels;

    std::vector<device*> vector_device;
    int output_device;
    //int output_channels;
    int output_samplerate;

    /* For Management of KSpectrogram Widgets */
    std::vector<KSpecWidget*> vector_spec;
    int num_spec;
    int id_max;

    /* For Proccessing */
    // const int 멤버 변?�는 read-only ??�?
    static const int max_channels = 16;
    /* type window 
     * 0 : shift 128 / frame 512
     * 1 : shift 256 / frame 1024
     * */
    int frame_size;
    int shift_size;
    double** buffer;

    WAV* wav_buf;
    STFT* stft;
    LogSpec* logspec;

    int scale;
    bool IsWavFile(char*);

    // minimum gap for spectrogram
    const double gap_min = 10;

    //TODO test code
    std::thread *t_indicate;
    
    /* audio play*/
    bool playing;
    int sample_rate;
    inline void Stop();
    void Indicate();
    void* buffer_play;
    std::vector<KSpecWidget*>vector_selected;

    RtOutput *sp;

    int x_mouse;
    int from,to;
    int channel_play;
    int size_play;

    int fmt_type;

  public :
      KAnalysis();
      ~KAnalysis();

    /* Load File and Create Spectrogram */
    void LoadFile(const char*);

    inline void Play();

    /* draw area of dragged rect */ 
    void SetArea(QPoint,QPoint);
    /* refresh all spectrograms */
    void RefreshSpecs();
    void SetRange(int,int);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent * event);
    void CloseSpec(KSpecWidget*);
    void CloseAll();

    public slots:
      void SlotAudioProbe();
      void SlotChangeDevice(int);
      void SlotChangeSamplerate(int);

   signals :
      void IndicateSignal(int pos);
      void SignalStartPlay();
      void SignalStopPlay();
};



#endif
