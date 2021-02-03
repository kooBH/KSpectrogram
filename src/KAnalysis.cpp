#include "KAnalysis.h"
#include "KSpecWidget.h"
#include "KSpectrogram.h"


#define FLOAT_CAST reinterpret_cast<float*>
#define SHORT_CAST reinterpret_cast<short*>

 KAnalysis::KAnalysis():
  btn_play("Play"),
  btn_stop("stop"),
  btn_load("Load"),
  btn_close("Close All"),
#ifndef NDEBUG
  btn_debug("debug"),
#endif
  label_scale("scale"),
  label_window("window"),
  label_sample_rate("sample rate"),
  width(0),
  height(0),
  num_spec(0),
  scale(1),
  playing(false),
  id_max(0),
  buffer_play(nullptr)
  {
	 sp = nullptr;
 t_indicate=nullptr;
 setUpdatesEnabled(true);
 setAcceptDrops(true);

 resize(600, 400);


 /* Default Values */
 frame_size = 512;
 shift_size = 128;
 sample_rate = 16000;

 
 /*** Processor ***/
  logspec = new LogSpec(max_channels,frame_size);
  stft = new STFT(max_channels, frame_size, shift_size);

  //setStyleSheet(_BG_COLOR_2_);

/*** Button Section ***/
  layout_button.addWidget(&btn_play);

 
  QObject::connect(
      &btn_play, 
      &QPushButton::clicked, 
      [&]() {if(!playing)Play(); }
  );

  QObject::connect(&btn_stop, &QPushButton::clicked, [&]() 
      { Stop(); }
  );
  layout_button.addWidget(&btn_stop);

  layout_button.addWidget(&btn_load);
  QObject::connect(&btn_load, &QPushButton::clicked, [&]() {
                  QString fileName;
                  QFileDialog dialog;

                  /* signal - slot for import button */
                  fileName = dialog.getOpenFileName(this,
                      tr("Open Wav File"), ".", tr("something (*.wav)"));
                     LoadFile(fileName.toStdString().c_str());
                    }
  );

  layout_button.addWidget(&btn_close);
  QObject::connect(&btn_close, &QPushButton::clicked, [&]() { CloseAll(); } );

  layout_button.addWidget(&label_scale);

  /* Scale ComboBox */

  combo_scale.addItem("1");
  combo_scale.addItem("2");
  combo_scale.addItem("4");
  combo_scale.addItem("8");

  combo_scale.setCurrentIndex(0);
  QObject::connect(&combo_scale, &QComboBox::currentTextChanged,
                     [&](const QString &text) {
                       scale = text.toInt();
                     }
  );

  layout_button.addWidget(&combo_scale);
  layout_button.addWidget(&label_window);

  /* window ComboBox */
  combo_window.addItem("128/512");
  combo_window.addItem("256/1024");
  combo_window.addItem("512/2048");

  // https://doc.qt.io/qt-5/qcombobox.html#currentIndexChanged
  QObject::connect(&combo_window, QOverload<int>::of(&QComboBox::currentIndexChanged),
               [&](int index) {
                   switch(index){
                    case 0:
                        shift_size = 128;
                        frame_size = 512;
                      break;
                    case 1:
                        shift_size = 256;
                        frame_size = 1024;
                      break;
                    case 2:
                        shift_size = 512;
                        frame_size = 2048;
                      break;
                   }
                  delete stft,logspec;
                  stft = new STFT(max_channels, frame_size, shift_size);
                  logspec = new LogSpec(max_channels,frame_size);
               }
  );
  
  layout_button.addWidget(&combo_window);

  /* sample_rate combobox */

  layout_button.addWidget(&label_sample_rate);
  combo_sample_rate.addItem("16000");
  combo_sample_rate.addItem("22500");
  combo_sample_rate.addItem("48000");
  combo_sample_rate.addItem("96000");

  QObject::connect(&combo_sample_rate, QOverload<int>::of(&QComboBox::currentIndexChanged),
               [&](int index) {
                   switch(index){
                    case 0:sample_rate = 16000;
                      break;
                    case 1:sample_rate = 22500;
                      break;
                    case 2:sample_rate = 48000;
                      break;
                    case 3:sample_rate = 96000;
                      break;
                   }
               }
  );

  layout_button.addWidget(&combo_sample_rate);


#ifndef NDEBUG
  QObject::connect(&btn_debug, &QPushButton::clicked, [&]() 
      {func_debug(); }
      );
  layout_button.addWidget(&btn_debug);
#endif

  widget_button.setLayout(&layout_button);
  layout.addWidget(&widget_button,BorderLayout::North);

/*** Spectrogram Section ***/

  // void QLayout::setContentsMargins(int left, int top, int right, int bottom);
  layout_spec.setContentsMargins(10,5,10,0);
  
  layout_spec.setAlignment(Qt::AlignTop);
  widget_spec.setLayout(&layout_spec);
//  widget_spec.setStyleSheet(_BG_COLOR_2_);
  
  area_spec.setWidget(&widget_spec);
  //area_spec.setStyleSheet(_BG_COLOR_3_);
  
  layout.addWidget(&area_spec,BorderLayout::Center);
  setLayout(&layout);
}

KAnalysis::~KAnalysis(){
  if(buffer_play) {
    switch(fmt_type){
      case 3:
        delete[] FLOAT_CAST(buffer_play);
        break;
      default :
        delete[] SHORT_CAST(buffer_play);
        break;
    }
  }
  if(t_indicate)t_indicate->join();
  delete stft,logspec,sp;
}

/* dragEnter?êÏÑú   */
 void KAnalysis::dragEnterEvent(QDragEnterEvent *event){
  /* Recognize  dropEvent, only After  acceptProposedAction. */
  event->acceptProposedAction();

}

 void KAnalysis::dropEvent(QDropEvent * event){
  std::string temp_str;

  /* Dose Dropped file have Url? */
  if (event->mimeData()->hasUrls()){
    QList<QUrl> url = event->mimeData()->urls();

    //for (int i = 0; i < url.size(); ++i) {
    // Get Urls as QList but gonna use only one url.
    for (int i = 0; i < 1; ++i) {
      // std::cout<<( temp.toString() ).toStdString()<<"\n";
      /* ex) file:///home/kbh/git/IIP_demo/build/input.wav */
      /* file://<path> format  */

      QUrl temp_qurl =  url.at(i);
      temp_str = (temp_qurl.toString() ).toStdString();
	 // std::cout << "temp_str "<<temp_str << "\n";
#ifdef _WIN32
	  /*
		 in Windows , getOpenFileName has
		 '/' before path. 
		 ex)
		 WIN32 : file:///C:/folder/file.wav
		 else  : file://C:/folder/file.wav
	  */
	  
	  temp_str = temp_str.substr(8);
#else
	  temp_str = temp_str.substr(7);
#endif
	  LoadFile(temp_str.c_str());
    }
  }
}

 void KAnalysis::LoadFile(const char* file_name){
#ifndef NDEBUG
      std::cout<<"INFO::load : "<<file_name<<"\n";
#endif

    /* Is it wav file ?*/
    if(IsWavFile((char*)file_name)) {
      //if(wav_buf->GetIsOpen())wav_buf->Finish();
      wav_buf = new WAV();
      wav_buf->SetSizes(frame_size, shift_size);
      std::cout << "KAnalysis::LoadFile("<<file_name << ")"<<std::endl;

      wav_buf->OpenFile(file_name);
      fmt_type = wav_buf->GetFmtType();

      //wav_buf->Print();
      int channels = wav_buf->GetChannels();
      int sample_rate = wav_buf->GetSampleRate();
      int length = wav_buf->GetSize();

      /* Adjusting in case sample size doesn't be devided by sihft_size */
      if (length % (channels * 2 * shift_size) != 0) {
        length /= channels * 2 * shift_size;
        length += 1;
      }
      else
        length /= channels * 2 * shift_size;
#ifndef NDEBUG
      printf("NOTE::the number of samples : %d * shift_size\n", length);
#endif

      /* Initialize Spectrogram Widgets */
      for (int i = 0; i < channels; i++) {

        KSpecWidget* temp_spec
          = new KSpecWidget(this,
            i + 1, length, frame_size + 2, shift_size, sample_rate, scale, fmt_type, id_max++);
        vector_spec.push_back(temp_spec);
        num_spec++;
      }

      /* Initialize buffers */
      double** buf_raw;
      buf_raw = new double* [channels];
      for (int i = 0; i < channels; i++) {
        buf_raw[i] = new double[frame_size];
        memset(buf_raw[i], 0, (frame_size) * sizeof(double));
      }
      double*** buf_data;
      buf_data = new double** [length];
      for (int i = 0; i < length; i++) {
        buf_data[i] = new double* [channels];
        for (int j = 0; j < channels; j++) {
          buf_data[i][j] = new double[frame_size + 2];
          memset(buf_data[i][j], 0, (frame_size + 2) * sizeof(double));
        }
      }
      double arr_max[max_channels];
      double arr_min[max_channels];

      /* Initialze buffer in WAV */
      for (int i = 0; i < channels; i++) {
        arr_max[i] = 0;
        arr_min[i] = 0;
      }

      /* 2020.05.13 kbh
       * keeping wav buffer of every spectrogram.
     * takes too much memory. maybe create temp file for buffer?
     */
      for (int j = 0; j < length; j++) {
        /* Create Spectrogram */
        wav_buf->Convert2Array(buf_raw);


        /* Create Buffer to Play */
        {
          switch (fmt_type) {
          case 3:
#pragma omp parallel for
            for (int i = 0; i < channels; i++) {
              memcpy(buf_data[j][i], buf_raw[i], sizeof(double) * frame_size);
              /* give wav buffer for each specwidget */
              float* temp_buf =
                FLOAT_CAST(vector_spec.at(num_spec - channels + i)->Get_buffer_wav());
              for (int k = 0; k < shift_size; k++)
                temp_buf[j * shift_size + k] =
                static_cast<float>(buf_raw[i][k]);
            }
            break;
          default:
#pragma omp parallel for
            for (int i = 0; i < channels; i++) {
              memcpy(buf_data[j][i], buf_raw[i], sizeof(double) * frame_size);
              /* give wav buffer for each specwidget */
              short* temp_buf =
                SHORT_CAST(vector_spec.at(num_spec - channels + i)->Get_buffer_wav());
              for (int k = 0; k < shift_size; k++)
                temp_buf[j * shift_size + k] =
                static_cast<short>(buf_raw[i][k]);
            }
            break;
          }
        }

        stft->stft(buf_raw, buf_data[j], channels);
        logspec->Process(buf_data[j], channels);

        for (int i = 0; i < channels; i++) {
          for (int k = 0; k < frame_size / 2 + 1; k++) {
            /* Reference : Audacity Implementaion */
            switch (fmt_type) {
            case 3:
              if (buf_data[j][i][k] < -250.0)buf_data[j][i][k] = -250.0;
              break;
            default:
              //if (buf_data[j][i][k] < -0.0)buf_data[j][i][k] = -0.0;
              //if (buf_data[j][i][k] < -200.0)buf_data[j][i][k] = -200.0;
              break;
            }
            /* filter inf and -inf */
            if  ((buf_data[j][i][k] == std::numeric_limits<double>::infinity())
              | (-buf_data[j][i][k] == std::numeric_limits<double>::infinity()))
              continue;
            /* Get min max */
            if (buf_data[j][i][k] > arr_max[i])
              arr_max[i] = buf_data[j][i][k];
            if (buf_data[j][i][k] < arr_min[i])
              arr_min[i] = buf_data[j][i][k];
          }
        }
      }

      /* Exception for all values are 0.0  */
    /*  for (int i = 0; i < channels; i++) {
        if (arr_max[i] - arr_min[i] < gap_min)
          arr_max[i] = arr_min[i] + gap_min;
      }*/

      for (int i = 0; i < channels; i++) {
        vector_spec.at(num_spec - channels + i)->SetRange(arr_min[i], arr_max[i]);
      //  printf("NOTE::Spectrogram[%d] min %lf | max %lf\n",i,arr_min[i],arr_max[i]);
      }

      /* Draw Spectrograms */
      for (int i = 0; i < channels; i++) {
        for (int j = 0; j < length; j++) {
          vector_spec.at(num_spec - channels + i)->Update(buf_data[j][i]);
        }
      }

      /* Display Spectrograms */
      for (int i = 0; i < channels; i++) {
        KSpecWidget* temp_spec = vector_spec.at(num_spec - channels + i);
        temp_spec->Confirm();
        layout_spec.addWidget((QWidget*)temp_spec);

        /* Set width as max of specs */
        if (width < temp_spec->GetSpecWidth())
          width = temp_spec->GetSpecWidth();
        height += temp_spec->GetSpecHeight() + 10;
        widget_spec.resize(width, height);
      }
      wav_buf->Finish();
      delete wav_buf;
      for (int i = 0; i < channels; i++) {
        delete[] buf_raw[i];
        delete[] buf_data[i];
      }
      delete[] buf_data;
      delete[] buf_raw;
    }
    else
      printf("ERROR::Not a wav file.\n");
 }

 bool KAnalysis::IsWavFile(char* file_name) {
   char* temp;
   char str_buf[128];
   strcpy(str_buf, file_name);
   bool ret;
   temp = strtok(str_buf, ".");
   temp = strtok(nullptr, ".");
   if (temp == nullptr) {
     printf("ERROR::File without file extension.\n");
     return false;
   }
   ret = !strcmp(temp, "wav");
   return ret;
 }


 void KAnalysis::CloseSpec(KSpecWidget* target) {
   /* Find */
   auto it =
     std::find(vector_spec.begin(), vector_spec.end(), target);
   auto dist = distance(vector_spec.begin(), it);
   /* delete & adjust */
   vector_spec.erase(vector_spec.begin() + dist);
   layout_spec.removeWidget((QWidget*)target);
   height -= target->GetSpecHeight() + 10;
   delete target;
   widget_spec.resize(width, height);
   num_spec--;
 }

 /* ?¨ÏÉù ?úÏãú */
 void KAnalysis::Indicate() {
   int delay = (1000000 / (double)sample_rate) * shift_size;

   for (int i = 0; i < vector_spec.size(); i++) {
     vector_spec.at(i)->SetDrawable(false);
     QWidget::connect(this, &KAnalysis::IndicateSignal,
       vector_spec.at(i), &KSpecWidget::Indicator);
     //    vector_spec.at(i)->Refresh(); 
   }
   for (int j = from; j < to; j++) {

     auto m_start = std::chrono::high_resolution_clock::now();

     emit(IndicateSignal(j));
     /*
       for(int i=0;i<vector_selected.size();i++) {
         vector_selected.at(i)->Indicator(j);
       }
       */


     auto m_elapsed
       = std::chrono::duration_cast<std::chrono::microseconds>(
         std::chrono::high_resolution_clock::now() - m_start);
     auto microseconds
       = std::chrono::duration_cast<std::chrono::microseconds>(m_elapsed).count();
     std::chrono::microseconds timespan_micro(delay - microseconds);
     /* to synchronize play. wait for (expected time - elapsed time).
        since computation, will be microsecond error.
     */
     std::this_thread::sleep_for(timespan_micro);

     if (!playing)break;
   }
   for (auto i = 0; i < vector_spec.size(); i++) {
     vector_spec.at(i)->SetDrawable(true);
     QWidget::disconnect(this, &KAnalysis::IndicateSignal, 0, 0);
   }
   playing = false;
   if (sp->IsRunning())sp->Stop();
   delete sp;
   emit(SignalStopPlay());
   printf("Indicator Destructed\n");
 }


 void KAnalysis::SetArea(QPoint global, QPoint origin) {
   for (int i = 0; i < vector_spec.size(); i++)
     vector_spec.at(i)->SetArea(global, origin);
 }

 void KAnalysis::RefreshSpecs() {
   for (int i = 0; i < vector_spec.size(); i++)
     vector_spec.at(i)->Refresh();
 }

 void KAnalysis::SetRange(int _from, int _to) {
   /* emptying vector */
   vector_selected.clear();
   from = _from;
   to = _to;
#ifndef NDEBUG
   /*
    printf("KAnalysis::SetRange [%-4d  %-4d]\n",from,to);
     for(int i=0;i<vector_spec.size();i++){
       if(vector_spec.at(i)->IsInArea())
         printf("ID : %d is in area.\n",vector_spec.at(i)->Get_id());
     }
     */
#endif
 }

 inline void KAnalysis::Play() {
   // playing = true;
   channel_play = 0;
   size_play = to - from;
   for (int i = 0; i < vector_spec.size(); i++) {
     if (vector_spec.at(i)->IsInArea()) {
       vector_selected.push_back(vector_spec.at(i));
       channel_play++;
     }
   }
   /*
   printf("2. size of vector : %d\n",vector_selected.size());
   printf("size_buf %d * %d * %d = %d\n",
       channel_play,
       size_play,
       shift_size,
       channel_play * size_play * shift_size
       );
       */

       /* Nothnig is sellected*/
   if (channel_play == 0) {
     // empty
     if (vector_spec.size() == 0)return;
#ifndef NDEBUG
     printf("KAnalysis::Play::play first spectrogram\n");
#endif

     // play first one.
     channel_play = 1;
     size_play = vector_spec.at(0)->GetSpecWidth();
     from = 0;
     to = size_play;

     fmt_type = vector_spec.at(0)->Get_fmt_type();

     switch (fmt_type) {
     case 3:
       if (buffer_play) delete[] FLOAT_CAST(buffer_play);
       buffer_play = new float[1 * size_play * shift_size];
       for (int i = 0; i < size_play * shift_size; i++) {
         if (i > (vector_spec.at(0)->Get_length()) * shift_size)
           FLOAT_CAST(buffer_play)[i] = 0;
         else
           FLOAT_CAST(buffer_play)[i]
           = FLOAT_CAST(vector_spec.at(0)->Get_buffer_wav())[i];
       }
       break;

     default:
       if (buffer_play) delete[] SHORT_CAST(buffer_play);
       buffer_play = new short[1 * size_play * shift_size];
       for (int i = 0; i < size_play * shift_size; i++) {
         if (i > (vector_spec.at(0)->Get_length()) * shift_size)
           SHORT_CAST(buffer_play)[i] = 0;
         else
           SHORT_CAST(buffer_play)[i]
           = SHORT_CAST(vector_spec.at(0)->Get_buffer_wav())[i];
       }
       break;

     }

     /* Normal Behavior */
   }
   else {
     /* Pick first specWidget as representing type */
     fmt_type = vector_selected.at(0)->Get_fmt_type();

     //  printf("play fmt : %d\n",fmt_type);
       /* Create buffer for play */
     switch (fmt_type) {
     case 3:
#ifndef NDEBUG
       printf("INFO:: build float buffer\n");
#endif
       if (buffer_play) delete[] FLOAT_CAST(buffer_play);
       buffer_play = new float[channel_play * size_play * shift_size];
       for (int i = 0; i < size_play * shift_size; i++) {
         for (int j = 0; j < vector_selected.size(); j++) {
           if (i > (vector_selected.at(j)->Get_length()) * shift_size)
             FLOAT_CAST(buffer_play)[i * channel_play + j] = 0;
           else
             FLOAT_CAST(buffer_play)[i * channel_play + j]
             = FLOAT_CAST(vector_selected.at(j)->Get_buffer_wav())[i + from * shift_size];
         }
       }
       break;
     default:
#ifndef NDEBUG
       printf("INFO:: build short buffer\n");
#endif
       if (buffer_play) delete[] SHORT_CAST(buffer_play);
       buffer_play = new short[channel_play * size_play * shift_size];
       for (int i = 0; i < size_play * shift_size; i++) {
         for (int j = 0; j < vector_selected.size(); j++) {
           if (i > (vector_selected.at(j)->Get_length()) * shift_size)
             SHORT_CAST(buffer_play)[i * channel_play + j] = 0;
           else
             SHORT_CAST(buffer_play)[i * channel_play + j]
             = SHORT_CAST(vector_selected.at(j)->Get_buffer_wav())[i + from * shift_size];
         }
       }
       break;
     }

   }


   /*** Scaling ***/
   /* Get Peak */
   short peak_short_buf = 100;
   float peak_float_buf = 0.5;

   const int peak_start_idx = 8000;

   int ii;
   /* Search max peak value */
#pragma omp parallel for private(ii)
   for (ii = peak_start_idx; ii < channel_play * size_play * shift_size; ii++) {
     switch (fmt_type) {
     case 3:
       if (peak_float_buf < fabs(FLOAT_CAST(buffer_play)[ii]))
         peak_float_buf = fabs(FLOAT_CAST(buffer_play)[ii]);
       break;
     default:
       if (peak_short_buf < abs(SHORT_CAST(buffer_play)[ii]))
         peak_short_buf = abs(SHORT_CAST(buffer_play)[ii]);
       break;
     }
   }


   switch (fmt_type) {
   case 3:
     printf("peak %f\n", peak_float_buf);
     break;
   default:
     printf("peak %d %d\n", peak_short_buf, 32768 / peak_short_buf);
     break;
   }
   /* norm & scale */
#pragma omp parallel for private(ii)
   for (ii = 0; ii < channel_play * size_play * shift_size; ii++) {
     switch (fmt_type) {
     case 3:
       FLOAT_CAST(buffer_play)[ii] / peak_float_buf;
       break;

     default:
       SHORT_CAST(buffer_play)[ii] *= (32768 / peak_short_buf);
       break;
       break;
     }
   }



   /*
    * audio play
    * */

   if (!buffer_play) {
     std::cout << "buffer_play is NULL!!" << std::endl;
     return;
   }

   if (channel_play != 0) {
     sp = new RtOutput(1, channel_play, sample_rate, 48000, shift_size, frame_size);
     switch (fmt_type) {
     case 3:
       sp->FullBufLoad(FLOAT_CAST(buffer_play), (long)(channel_play * size_play * shift_size));
       break;
     default:
       sp->FullBufLoad(SHORT_CAST(buffer_play), (long)(channel_play * size_play * shift_size));
       break;
     }
     sp->Start();
     emit(SignalStartPlay());
   }

   /* Indicator == Destructor for sp */

   playing = true;
   if (t_indicate)delete t_indicate;
   t_indicate = new std::thread(&KAnalysis::Indicate, this);
   t_indicate->detach();
   //delete[] buffer_play;

 }

 inline void KAnalysis::Stop() {
   if (playing) {
     playing = false;
   }
   RefreshSpecs();
 }

 void KAnalysis::CloseAll() {
   while (vector_spec.size() != 0) {
     KSpecWidget* temp = vector_spec.at(0);
     CloseSpec(temp);
   }
 }


#ifndef NDEBUG
 inline void KAnalysis::func_debug() {
   for (int i = 0; i < vector_spec.size(); i++)
     printf("%d : drawable %s\n",
       vector_spec.at(i)->Get_id(),
       vector_spec.at(i)->IsDrawable() ? "O" : "X"
     );
}
#endif
